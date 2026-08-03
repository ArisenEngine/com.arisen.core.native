#pragma once

#include "Base/FoundationMinimal.h"

#include "../Handles/RHIHandle.h"
#include "RHIDeferredDeletionQueue.h"


#include <atomic>
#include <mutex>

namespace ArisenEngine::RHI
{
    // Backend-agnostic registry:
    // - generation guards against use-after-free
    // - refCount is protected by a mutex (simple and portable)
    // - destruction is routed through IRHIDeferredDeletionQueue (GPU-safe)
    class RHIResourceRegistry final
    {
    public:
        NO_COPY_NO_MOVE_NO_DEFAULT(RHIResourceRegistry)

        explicit RHIResourceRegistry(IRHIDeferredDeletionQueue* deletionQueue)
            : m_DeletionQueue(deletionQueue)
        {
        }

        ~RHIResourceRegistry() noexcept
        {
            // Owning backends must call Shutdown while their deferred queue is
            // alive. Destruction is the final allocation-free fallback.
            DestroyAllImmediately();
        }

        /**
         * @brief Shuts down the registry by enqueuing all remaining resources for destruction.
         * This is safe to call multiple times and ensures the registry remains valid for
         * lookup/release calls that might happen during the subsequent flush of the enqueued items.
         */
        void Shutdown()
        {
            if (m_ShutdownComplete.load(std::memory_order_acquire))
                return;

            if (!m_DeletionQueue)
            {
                DestroyAllImmediately();
                return;
            }

            std::lock_guard<std::mutex> lock(m_Mutex);
            for (size_t i = 0; i < m_Entries.size(); ++i)
            {
                auto& e = m_Entries[i];
                if (e.refCount > 0 && e.item.ptr && e.item.deleter)
                {
                    RHIDeletionDependencies deps;
                    // Enqueue owns the item only after it returns successfully. If
                    // it throws, this entry remains intact for retry or terminal drain.
                    m_DeletionQueue->Enqueue(deps, e.item);
                    e.refCount = 0;
                    e.item = {};
                    e.generation++;
                    for (auto& ticket : e.maxTickets) ticket = 0;
                }
            }
            m_ShutdownComplete.store(true, std::memory_order_release);
        }

        // The caller must establish that no GPU work can still reference these
        // resources. This terminal path is allocation-free and is used only when
        // an already-idle owner cannot publish into the deferred queue.
        void DestroyAllImmediately() noexcept
        {
            for (size_t i = 0; i < m_Entries.size(); ++i)
            {
                RHIDeferredDeleteItem item{};
                {
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    auto& e = m_Entries[i];
                    if (e.refCount == 0)
                        continue;

                    item = e.item;
                    e.refCount = 0;
                    e.item = {};
                    e.generation++;
                    for (auto& ticket : e.maxTickets) ticket = 0;
                }

                if (item.ptr && item.deleter)
                    item.deleter(item.ptr);
            }
            m_ShutdownComplete.store(true, std::memory_order_release);
        }

        // Create a new entry with refCount=1.
        RHIResourceHandle Create(RHIDeferredDeleteItem item)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);

            UInt32 idx = 0;
            if (!m_FreeList.empty())
            {
                idx = m_FreeList.back();
                m_FreeList.pop_back();
            }
            else
            {
                idx = static_cast<UInt32>(m_Entries.size());
                m_Entries.emplace_back();
            }

            auto& e = m_Entries[idx];
            e.refCount = 1;
            e.item = item;
            for (int i = 0; i < 4; ++i) e.maxTickets[i] = 0;
            return RHIResourceHandle{idx, e.generation};
        }

        bool Retain(RHIResourceHandle h)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (!ValidateUnlocked(h))
                return false;
            m_Entries[h.index].refCount += 1;
            return true;
        }

        void RejectNextReleaseForTesting() noexcept
        {
            m_RejectNextReleaseForTesting.store(true, std::memory_order_release);
        }

        // Record resource usage on a specific queue.
        void UpdateTicket(RHIResourceHandle h, RHIQueueType queue, RHIGpuTicket ticket)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (!ValidateUnlocked(h))
                return;

            auto& e = m_Entries[h.index];
            auto idx = static_cast<UInt32>(queue);
            if (idx < 4 && ticket > e.maxTickets[idx])
            {
                e.maxTickets[idx] = ticket;
            }
        }

        bool Release(RHIResourceHandle h)
        {
            RHIDeferredDeleteItem immediateItem{};
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                if (!ValidateUnlocked(h))
                {
                    // A completed terminal shutdown has already transferred every
                    // remaining entry. Deferred parent destructors may still release
                    // child handles while that queue is being flushed.
                    return m_ShutdownComplete.load(std::memory_order_acquire) &&
                        h.IsValid() && h.index < m_Entries.size();
                }

                if (m_RejectNextReleaseForTesting.exchange(false, std::memory_order_acq_rel))
                    return false;

                auto& e = m_Entries[h.index];

                if (e.refCount > 1)
                {
                    e.refCount -= 1;
                    return true;
                }

                RHIDeletionDependencies finalDeps;
                for (int i = 0; i < 4; ++i)
                    finalDeps.tickets[i] = e.maxTickets[i];

                // Reserve the reusable slot before ownership transfer. If this
                // allocation fails, the entry remains fully live.
                m_FreeList.emplace_back(h.index);
                if (e.item.ptr && e.item.deleter && m_DeletionQueue)
                {
                    try
                    {
                        m_DeletionQueue->Enqueue(finalDeps, e.item);
                    }
                    catch (...)
                    {
                        m_FreeList.pop_back();
                        throw;
                    }
                }
                else
                {
                    immediateItem = e.item;
                }

                e.item = {};
                e.refCount = 0;
                e.generation += 1;
                for (auto& ticket : e.maxTickets) ticket = 0;
            }

            if (immediateItem.ptr && immediateItem.deleter)
                immediateItem.deleter(immediateItem.ptr);
            return true;
        }

        bool IsAlive(RHIResourceHandle h) const
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return ValidateUnlocked(h);
        }

        RHIDeletionDependencies GetTickets(RHIResourceHandle h) const
        {
            RHIDeletionDependencies deps;
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (ValidateUnlocked(h))
            {
                const auto& e = m_Entries[h.index];
                for (int i = 0; i < 4; ++i) deps.tickets[i] = e.maxTickets[i];
            }
            return deps;
        }

    private:
        struct Entry
        {
            UInt32 refCount{0};
            UInt32 generation{1};
            RHIDeferredDeleteItem item;
            RHIGpuTicket maxTickets[4]{0, 0, 0, 0};
        };

        bool ValidateUnlocked(RHIResourceHandle h) const
        {
            if (!h.IsValid())
                return false;
            if (h.index >= m_Entries.size())
                return false;
            const auto& e = m_Entries[h.index];
            if (e.generation != h.generation)
                return false;
            if (e.refCount == 0)
                return false;
            return true;
        }

        IRHIDeferredDeletionQueue* m_DeletionQueue{nullptr};
        std::atomic<bool> m_ShutdownComplete{false};
        std::atomic<bool> m_RejectNextReleaseForTesting{false};
        mutable std::mutex m_Mutex;
        Containers::Vector<Entry> m_Entries;
        Containers::Vector<UInt32> m_FreeList;
    };
} // namespace ArisenEngine::RHI
