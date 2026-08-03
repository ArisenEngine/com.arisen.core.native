#include "RHIAbiOwnerRegistry.h"

#include "RHIError.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace
{
    using namespace ArisenEngine::RHI;

    constexpr uint32_t MaxOwnerSlots = 4096;
    constexpr uintptr_t TokenIndexMask = 0xFFFFU;
    constexpr uint32_t TokenIndexBits = 16;
    constexpr uint32_t TokenGenerationBits = 31U;
    constexpr uintptr_t TokenGenerationMask = (uintptr_t{1} << TokenGenerationBits) - 1U;
    constexpr uintptr_t TokenMask = (TokenGenerationMask << TokenIndexBits) | TokenIndexMask;

    static_assert(sizeof(uintptr_t) == 8, "RHI ABI owner tokens require a 64-bit process");
    static_assert(MaxOwnerSlots < TokenIndexMask);

    struct OwnerSlot
    {
        std::atomic<void*> object{nullptr};
        std::atomic<uint32_t> generation{1};
        std::atomic<ERHIAbiOwnerType> type{ERHIAbiOwnerType::Invalid};
        std::atomic<uintptr_t> parentToken{0};
    };

    struct OwnerRegistry
    {
        OwnerRegistry()
        {
            for (uint32_t i = 0; i < MaxOwnerSlots; ++i)
                freeSlots[i] = MaxOwnerSlots - i - 1U;
        }

        std::array<OwnerSlot, MaxOwnerSlots> slots{};
        std::array<uint32_t, MaxOwnerSlots> freeSlots{};
        uint32_t freeSlotCount = MaxOwnerSlots;
        std::unordered_map<const void*, uint32_t> objectSlots;
        std::mutex mutationMutex;
    };

    OwnerRegistry& GetRegistry()
    {
        static OwnerRegistry registry;
        return registry;
    }

    uintptr_t EncodeToken(uint32_t index, uint32_t generation) noexcept
    {
        return (static_cast<uintptr_t>(generation) << TokenIndexBits) |
            static_cast<uintptr_t>(index + 1U);
    }

    bool DecodeToken(const void* token, uint32_t& index, uint32_t& generation) noexcept
    {
        const uintptr_t value = reinterpret_cast<uintptr_t>(token);
        if (value == 0 || (value & ~TokenMask) != 0)
            return false;

        const uintptr_t encodedIndex = value & TokenIndexMask;
        const uintptr_t encodedGeneration = value >> TokenIndexBits;
        if (encodedIndex == 0 || encodedIndex > MaxOwnerSlots || encodedGeneration == 0)
            return false;

        index = static_cast<uint32_t>(encodedIndex - 1U);
        generation = static_cast<uint32_t>(encodedGeneration);
        return true;
    }

    uint32_t NextGeneration(uint32_t generation) noexcept
    {
        generation = static_cast<uint32_t>((generation + 1U) & TokenGenerationMask);
        return generation == 0 ? 1U : generation;
    }

    bool IsLiveSlot(const OwnerSlot& slot,
                    uint32_t generation,
                    ERHIAbiOwnerType expectedType) noexcept
    {
        return slot.generation.load(std::memory_order_acquire) == generation &&
            slot.type.load(std::memory_order_acquire) == expectedType &&
            slot.object.load(std::memory_order_acquire) != nullptr;
    }

    [[noreturn]] void ThrowInvalidOwnerToken(const char* operation,
                                             ERHIAbiOwnerType type,
                                             const void* token,
                                             uint32_t index,
                                             uint32_t generation,
                                             const char* message)
    {
        char detail[256]{};
        std::snprintf(detail,
                      sizeof(detail),
                      "%s. Token=%u:%u",
                      message ? message : "Invalid RHI ABI owner token",
                      index,
                      generation);
        SetLastErrorDetailed(EErrorCode::InvalidHandle,
                             operation,
                             0,
                             GetAbiOwnerTypeName(type),
                             reinterpret_cast<uint64_t>(token),
                             index,
                             generation,
                             detail);
        throw std::invalid_argument(detail);
    }

    void InvalidateSlotLocked(OwnerRegistry& registry, uint32_t index, uint32_t generation) noexcept
    {
        OwnerSlot& slot = registry.slots[index];
        if (slot.generation.load(std::memory_order_relaxed) != generation ||
            slot.object.load(std::memory_order_relaxed) == nullptr)
        {
            return;
        }

        const uintptr_t token = EncodeToken(index, generation);
        for (uint32_t childIndex = 0; childIndex < MaxOwnerSlots; ++childIndex)
        {
            OwnerSlot& child = registry.slots[childIndex];
            if (child.object.load(std::memory_order_relaxed) != nullptr &&
                child.parentToken.load(std::memory_order_relaxed) == token)
            {
                InvalidateSlotLocked(
                    registry,
                    childIndex,
                    child.generation.load(std::memory_order_relaxed));
            }
        }

        const void* object = slot.object.exchange(nullptr, std::memory_order_acq_rel);
        const auto mapIt = registry.objectSlots.find(object);
        if (mapIt != registry.objectSlots.end() && mapIt->second == index)
            registry.objectSlots.erase(mapIt);

        slot.parentToken.store(0, std::memory_order_relaxed);
        slot.type.store(ERHIAbiOwnerType::Invalid, std::memory_order_release);
        slot.generation.store(NextGeneration(generation), std::memory_order_release);
        registry.freeSlots[registry.freeSlotCount++] = index;
    }
}

namespace ArisenEngine::RHI
{
    const char* GetAbiOwnerTypeName(ERHIAbiOwnerType type) noexcept
    {
        switch (type)
        {
        case ERHIAbiOwnerType::Instance: return "RHIInstance";
        case ERHIAbiOwnerType::Device: return "RHIDevice";
        case ERHIAbiOwnerType::Factory: return "RHIFactory";
        case ERHIAbiOwnerType::Surface: return "RHISurface";
        case ERHIAbiOwnerType::SwapChain: return "RHISwapChain";
        case ERHIAbiOwnerType::Queue: return "RHIQueue";
        case ERHIAbiOwnerType::CommandBufferPool: return "RHICommandBufferPool";
        case ERHIAbiOwnerType::CommandBuffer: return "RHICommandBuffer";
        case ERHIAbiOwnerType::PipelineCache: return "RHIPipelineCache";
        case ERHIAbiOwnerType::PipelineState: return "RHIPipelineState";
        case ERHIAbiOwnerType::DescriptorPool: return "RHIDescriptorPool";
        default: return "RHIAbiOwner";
        }
    }

    void* RegisterAbiOwnerPointer(void* object,
                                  ERHIAbiOwnerType type,
                                  const void* parentToken)
    {
        if (!object)
            ThrowInvalidParameter(__func__, "object", "Cannot register a null RHI ABI owner");
        if (type == ERHIAbiOwnerType::Invalid)
            ThrowInvalidParameter(__func__, "type", "Cannot register an invalid RHI ABI owner type");

        uint32_t parentIndex = 0;
        uint32_t parentGeneration = 0;
        const uintptr_t parentValue = reinterpret_cast<uintptr_t>(parentToken);
        if (parentToken && !DecodeToken(parentToken, parentIndex, parentGeneration))
            ThrowInvalidOwnerToken(__func__, ERHIAbiOwnerType::Invalid, parentToken,
                                   UINT32_MAX, 0, "Malformed parent owner token");

        OwnerRegistry& registry = GetRegistry();
        std::lock_guard lock(registry.mutationMutex);

        if (parentToken)
        {
            const OwnerSlot& parent = registry.slots[parentIndex];
            if (parent.generation.load(std::memory_order_relaxed) != parentGeneration ||
                parent.object.load(std::memory_order_relaxed) == nullptr)
            {
                ThrowInvalidOwnerToken(__func__, ERHIAbiOwnerType::Invalid, parentToken,
                                       parentIndex, parentGeneration,
                                       "Parent owner token is stale or destroyed");
            }
        }

        const auto existing = registry.objectSlots.find(object);
        if (existing != registry.objectSlots.end())
        {
            OwnerSlot& slot = registry.slots[existing->second];
            if (slot.object.load(std::memory_order_relaxed) == object)
            {
                if (slot.type.load(std::memory_order_relaxed) != type)
                    ThrowInvalidState(__func__, GetAbiOwnerTypeName(type),
                                      reinterpret_cast<uint64_t>(object),
                                      "Native owner is already registered with a different ABI type");

                const uintptr_t existingParent = slot.parentToken.load(std::memory_order_relaxed);
                if (parentValue != 0 && existingParent != 0 && existingParent != parentValue)
                    ThrowInvalidState(__func__, GetAbiOwnerTypeName(type),
                                      reinterpret_cast<uint64_t>(object),
                                      "Native owner is already registered under a different parent");
                if (existingParent == 0 && parentValue != 0)
                    slot.parentToken.store(parentValue, std::memory_order_relaxed);

                const uint32_t generation = slot.generation.load(std::memory_order_relaxed);
                return reinterpret_cast<void*>(EncodeToken(existing->second, generation));
            }

            registry.objectSlots.erase(existing);
        }

        if (registry.freeSlotCount == 0)
            ThrowInvalidState(__func__, GetAbiOwnerTypeName(type),
                              reinterpret_cast<uint64_t>(object),
                              "RHI ABI owner-token registry is exhausted");

        const uint32_t index = registry.freeSlots[--registry.freeSlotCount];
        OwnerSlot& slot = registry.slots[index];
        const uint32_t generation = slot.generation.load(std::memory_order_relaxed);
        try
        {
            registry.objectSlots.emplace(object, index);
        }
        catch (...)
        {
            registry.freeSlots[registry.freeSlotCount++] = index;
            throw;
        }
        slot.parentToken.store(parentValue, std::memory_order_relaxed);
        slot.type.store(type, std::memory_order_relaxed);
        slot.object.store(object, std::memory_order_release);
        return reinterpret_cast<void*>(EncodeToken(index, generation));
    }

    void* ResolveAbiOwnerPointer(const void* token,
                                 ERHIAbiOwnerType expectedType,
                                 const char* operation)
    {
        if (!token)
            ThrowInvalidParameter(operation, "owner", GetAbiOwnerTypeName(expectedType));

        uint32_t index = UINT32_MAX;
        uint32_t generation = 0;
        if (!DecodeToken(token, index, generation))
            ThrowInvalidOwnerToken(operation, expectedType, token, index, generation,
                                   "Malformed RHI ABI owner token");

        const OwnerSlot& slot = GetRegistry().slots[index];
        if (!IsLiveSlot(slot, generation, expectedType))
            ThrowInvalidOwnerToken(
                operation, expectedType, token, index, generation,
                "Invalid, stale, destroyed, or type-mismatched RHI ABI owner token");

        void* object = slot.object.load(std::memory_order_acquire);
        if (slot.generation.load(std::memory_order_acquire) != generation ||
            slot.type.load(std::memory_order_acquire) != expectedType ||
            slot.object.load(std::memory_order_acquire) != object)
        {
            ThrowInvalidOwnerToken(operation, expectedType, token, index, generation,
                                   "RHI ABI owner token changed while being resolved");
        }

        return object;
    }

    void* ResolveAbiOwnerParentPointer(const void* token,
                                       ERHIAbiOwnerType expectedParentType,
                                       const char* operation)
    {
        uint32_t index = UINT32_MAX;
        uint32_t generation = 0;
        if (!DecodeToken(token, index, generation))
            ThrowInvalidOwnerToken(operation, expectedParentType, token, index, generation,
                                   "Malformed child owner token");

        const OwnerSlot& slot = GetRegistry().slots[index];
        if (slot.generation.load(std::memory_order_acquire) != generation ||
            slot.object.load(std::memory_order_acquire) == nullptr)
        {
            ThrowInvalidOwnerToken(operation, expectedParentType, token, index, generation,
                                   "Child owner token is stale or destroyed");
        }

        const void* parentToken = reinterpret_cast<const void*>(
            slot.parentToken.load(std::memory_order_acquire));
        if (!parentToken)
            ThrowInvalidState(operation, GetAbiOwnerTypeName(expectedParentType),
                              reinterpret_cast<uint64_t>(token),
                              "RHI ABI owner does not have the expected parent");

        return ResolveAbiOwnerPointer(parentToken, expectedParentType, operation);
    }

    bool InvalidateAbiOwner(const void* token, ERHIAbiOwnerType expectedType) noexcept
    {
        uint32_t index = UINT32_MAX;
        uint32_t generation = 0;
        if (!DecodeToken(token, index, generation))
            return false;

        OwnerRegistry& registry = GetRegistry();
        std::lock_guard lock(registry.mutationMutex);
        OwnerSlot& slot = registry.slots[index];
        if (slot.generation.load(std::memory_order_relaxed) != generation ||
            slot.type.load(std::memory_order_relaxed) != expectedType ||
            slot.object.load(std::memory_order_relaxed) == nullptr)
        {
            return false;
        }

        InvalidateSlotLocked(registry, index, generation);
        return true;
    }

    bool InvalidateAbiOwnerByObject(const void* object, ERHIAbiOwnerType expectedType) noexcept
    {
        if (!object)
            return false;

        OwnerRegistry& registry = GetRegistry();
        std::lock_guard lock(registry.mutationMutex);
        const auto found = registry.objectSlots.find(object);
        if (found == registry.objectSlots.end())
            return false;

        OwnerSlot& slot = registry.slots[found->second];
        if (slot.type.load(std::memory_order_relaxed) != expectedType ||
            slot.object.load(std::memory_order_relaxed) != object)
        {
            return false;
        }

        InvalidateSlotLocked(
            registry,
            found->second,
            slot.generation.load(std::memory_order_relaxed));
        return true;
    }

    bool TryGetAbiOwnerTokenIdentity(const void* token,
                                     RHIAbiOwnerTokenIdentity& identity) noexcept
    {
        return DecodeToken(token, identity.index, identity.generation);
    }
}
