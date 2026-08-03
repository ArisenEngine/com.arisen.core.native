#pragma once

#include "RHI/Definitions/CoreRHICommon.h"

#include <cstdint>

namespace ArisenEngine::RHI
{
    // ABI calls resolve tokens without locking. Owner registration and teardown must be
    // serialized against calls that use that owner; teardown invalidates the token tree
    // before native destruction begins.
    class RHIInstance;
    class RHIDevice;
    class RHIFactory;
    class RHISurface;
    class RHISwapChain;
    class RHIQueue;
    class RHICommandBufferPool;
    class RHICommandBuffer;
    class RHIPipelineCache;
    class RHIPipelineState;
    class RHIDescriptorPool;

    enum class ERHIAbiOwnerType : uint16_t
    {
        Invalid = 0,
        Instance,
        Device,
        Factory,
        Surface,
        SwapChain,
        Queue,
        CommandBufferPool,
        CommandBuffer,
        PipelineCache,
        PipelineState,
        DescriptorPool
    };

    struct RHIAbiOwnerTokenIdentity
    {
        uint32_t index = UINT32_MAX;
        uint32_t generation = 0;
    };

    RHI_DLL void* RegisterAbiOwnerPointer(void* object,
                                          ERHIAbiOwnerType type,
                                          const void* parentToken = nullptr);
    RHI_DLL void* ResolveAbiOwnerPointer(const void* token,
                                         ERHIAbiOwnerType expectedType,
                                         const char* operation);
    RHI_DLL void* ResolveAbiOwnerParentPointer(const void* token,
                                               ERHIAbiOwnerType expectedParentType,
                                               const char* operation);
    RHI_DLL bool InvalidateAbiOwner(const void* token, ERHIAbiOwnerType expectedType) noexcept;
    RHI_DLL bool InvalidateAbiOwnerByObject(const void* object, ERHIAbiOwnerType expectedType) noexcept;
    RHI_DLL bool TryGetAbiOwnerTokenIdentity(const void* token,
                                             RHIAbiOwnerTokenIdentity& identity) noexcept;
    RHI_DLL const char* GetAbiOwnerTypeName(ERHIAbiOwnerType type) noexcept;

    template <typename TObject>
    TObject* RegisterAbiOwner(TObject* object,
                              ERHIAbiOwnerType type,
                              const void* parentToken = nullptr)
    {
        return reinterpret_cast<TObject*>(RegisterAbiOwnerPointer(object, type, parentToken));
    }

    template <typename TObject>
    TObject* ResolveAbiOwner(TObject* token,
                             ERHIAbiOwnerType expectedType,
                             const char* operation)
    {
        return static_cast<TObject*>(ResolveAbiOwnerPointer(token, expectedType, operation));
    }

    template <typename TObject>
    TObject* ResolveAbiOwnerParent(const void* token,
                                   ERHIAbiOwnerType expectedParentType,
                                   const char* operation)
    {
        return static_cast<TObject*>(ResolveAbiOwnerParentPointer(token, expectedParentType, operation));
    }

    template <typename TObject>
    struct RHIAbiOwnerTraits;

#define ARISEN_RHI_ABI_OWNER_TRAIT(objectType, ownerType) \
    template <> struct RHIAbiOwnerTraits<objectType> \
    { \
        static constexpr ERHIAbiOwnerType Type = ERHIAbiOwnerType::ownerType; \
    }

    ARISEN_RHI_ABI_OWNER_TRAIT(RHIInstance, Instance);
    ARISEN_RHI_ABI_OWNER_TRAIT(RHIDevice, Device);
    ARISEN_RHI_ABI_OWNER_TRAIT(RHIFactory, Factory);
    ARISEN_RHI_ABI_OWNER_TRAIT(RHISurface, Surface);
    ARISEN_RHI_ABI_OWNER_TRAIT(RHISwapChain, SwapChain);
    ARISEN_RHI_ABI_OWNER_TRAIT(RHIQueue, Queue);
    ARISEN_RHI_ABI_OWNER_TRAIT(RHICommandBufferPool, CommandBufferPool);
    ARISEN_RHI_ABI_OWNER_TRAIT(RHICommandBuffer, CommandBuffer);
    ARISEN_RHI_ABI_OWNER_TRAIT(RHIPipelineCache, PipelineCache);
    ARISEN_RHI_ABI_OWNER_TRAIT(RHIPipelineState, PipelineState);
    ARISEN_RHI_ABI_OWNER_TRAIT(RHIDescriptorPool, DescriptorPool);

#undef ARISEN_RHI_ABI_OWNER_TRAIT

    template <typename TObject>
    concept RHIAbiOwner = requires
    {
        RHIAbiOwnerTraits<TObject>::Type;
    };

    template <RHIAbiOwner TObject>
    void RequireAbiPointer(TObject*& value,
                           const char* operation,
                           const char*,
                           const char*)
    {
        value = ResolveAbiOwner(value, RHIAbiOwnerTraits<TObject>::Type, operation);
    }
}
