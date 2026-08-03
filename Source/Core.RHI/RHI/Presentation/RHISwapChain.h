#pragma once
#include "Base/FoundationMinimal.h"
#include "RHI/Handles/RHIHandle.h"
#include "RHI/Definitions/CoreRHICommon.h"
#include "RHI/Resources/RHIDeferredDeletionQueue.h"
#include "RHI/Enums/Image/EFormat.h"
#include "RHI/Enums/Image/EColorSpace.h"
#include "RHI/Enums/Image/EImageUsageFlagBits.h"
#include "RHI/Enums/Image/ECompositeAlphaFlagBits.h"
#include "RHI/Enums/Memory/ESharingMode.h"
#include "RHI/Enums/Swapchain/EPresentMode.h"

namespace ArisenEngine::RHI
{
    enum class RHISwapChainFrameState : UInt8
    {
        Idle,
        Acquired,
        Submitted,
        Presented,
        Retired
    };

    struct RHISwapChainDescriptor
    {
        UInt32 width;
        UInt32 height;
        UInt32 imageCount;
        UInt32 imageArrayLayers;
        UInt32 imageUsageFlagBits;
        UInt32 queueFamilyIndexCount;
        EFormat colorFormat;
        EColorSpace colorSpace;
        ESharingMode sharingMode;
        EPresentMode presentMode;
        UInt32 swapChainCreateFlags;
        UInt32 surfaceTransformFlagBits;
        UInt32 compositeAlphaFlagBits;
        bool clipped;
        bool bExportSharedWin32Handle;
        void* customData;
    };

    class RHI_DLL RHISwapChain
    {
    public:
        NO_COPY_NO_MOVE_NO_DEFAULT(RHISwapChain)
        explicit RHISwapChain(UInt32 maxFramesInFlight) : m_MaxFramesInFlight(maxFramesInFlight) {}
        virtual ~RHISwapChain() noexcept = default;

        virtual void* GetHandle() const = 0;
        virtual void CreateSwapChainWithDesc(RHISwapChainDescriptor desc) = 0;
        virtual RHIImageHandle BeginFrame(UInt32 frameIndex) = 0;
        virtual void EndFrame(UInt32 frameIndex) = 0;
        virtual RHIGpuTicket RetireFrame(UInt32 frameIndex) = 0;

        virtual RHISemaphoreHandle GetImageAvailableSemaphore(UInt32 frameIndex) const = 0;
        virtual RHISemaphoreHandle GetRenderFinishSemaphore(UInt32 frameIndex) const = 0;
        virtual RHIImageHandle AcquireCurrentImage(UInt32 frameIndex) = 0;
        virtual RHIImageViewHandle GetImageView(UInt32 frameIndex) const = 0;
        
        virtual void Cleanup() = 0;
        virtual void Present(UInt32 frameIndex) = 0;
        virtual void SetResolution(UInt32 width, UInt32 height) = 0;
        virtual bool TrySetResolution(UInt32 width, UInt32 height)
        {
            SetResolution(width, height);
            return true;
        }

        virtual void* GetSharedWin32Handle(UInt32 index) { return nullptr; }
        virtual UInt64 GetSharedMemorySize(UInt32 index) { return 0; }
        virtual void* GetRenderFinishedSemaphoreWin32Handle(UInt32 frameIndex) { return nullptr; }
        virtual void* CreateConsumedSemaphoreWin32Handle(UInt32 frameIndex) { return nullptr; }
        virtual void CompleteConsumedSemaphoreWin32Handle(void* handle) {}
        virtual void ReleaseConsumedSemaphoreWin32Handle(void* handle) {}
        virtual bool AcknowledgeExternalConsumerRelease() { return true; }
        UInt32 GetMaxFramesInFlight() const { return m_MaxFramesInFlight; }

    protected:
        virtual void RecreateSwapChainIfNeeded() = 0;
        
        UInt32 m_MaxFramesInFlight;
    };
}
