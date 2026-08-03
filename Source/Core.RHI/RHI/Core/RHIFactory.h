#pragma once
#include "Base/FoundationMinimal.h"
#include "Base/BindingMacros.h"
#include "../Core/RHICommon.h"
#include "../Handles/RHIHandle.h"
#include <string>
#include "../Descriptors/RHIResourceDescriptors.h"
#include "../Queues/RHIQueueType.h"
#include "../Resources/RHIDeferredDeletionQueue.h" // RHIGpuTicket

namespace ArisenEngine::RHI
{
    class RHISampler;
    struct RHISamplerDesc;
    class RHIShaderProgram;
    struct RHIShaderProgramDesc;
    class RHICommandBufferPool;
    struct RenderPassDescriptor; // Assuming this exists or will be added
    struct RHIFrameBufferDescriptor; // Assuming this exists or will be added

    // TODO(CppSharp-P1): RHIFactory is missing the RHI_DLL export macro. Although accessed indirectly via 
    // RHIDevice::GetFactory(), CppSharp need to see this class to generate correct bindings. 
    // Consider adding RHI_DLL or exposing via an opaque handle.
    // TODO(Interface-P1): Consider extending RHIFactory to RHIResourceManager, integrating Buffer/Image 
    // operational methods (BufferMemoryCopy, MapBuffer, GetBufferSize, etc.) that are currently scattered in RHIDevice.
    // This would allow Resource Create/Release/Query/Map to be completed through the same interface.
    class RHIFactory
    {
    public:
        virtual ~RHIFactory() noexcept = default;

        virtual RHIShaderProgramHandle CreateGPUProgram() = 0;
        virtual bool ReleaseGPUProgram(RHIShaderProgramHandle handle) = 0;
        virtual bool AttachProgramByteCode(RHIShaderProgramHandle handle, RHIShaderProgramDesc&& desc) = 0;

        virtual RHICommandBufferPoolHandle CreateCommandBufferPool(RHIQueueType queueType = RHIQueueType::Graphics) = 0;
        virtual bool ReleaseCommandBufferPool(RHICommandBufferPoolHandle handle) = 0;

        virtual RHIRenderPassHandle CreateRenderPass() = 0;
        virtual bool ReleaseRenderPass(RHIRenderPassHandle renderPass) = 0;

        virtual RHIFrameBufferHandle CreateFrameBuffer() = 0;
        virtual bool ReleaseFrameBuffer(RHIFrameBufferHandle frameBuffer) = 0;

        virtual RHIBufferHandle CreateBuffer(RHIBufferDescriptor&& desc, const String& name = "Anonymous") = 0;
        virtual bool ReleaseBuffer(RHIBufferHandle bufferHandle) = 0;

        virtual RHIImageHandle CreateImage(RHIImageDescriptor&& desc, const String& name = "Anonymous") = 0;
        virtual bool ReleaseImage(RHIImageHandle imageHandle) = 0;

        virtual RHIMemoryPoolHandle CreateMemoryPool(UInt64 size, UInt32 usageBits) = 0;
        virtual bool ReleaseMemoryPool(RHIMemoryPoolHandle handle) = 0;

        virtual RHIBufferHandle CreateBufferAliased(RHIBufferDescriptor&& desc, RHIMemoryPoolHandle pool, UInt64 offset,
                                                    const String& name = "Anonymous") = 0;
        virtual RHIImageHandle CreateImageAliased(RHIImageDescriptor&& desc, RHIMemoryPoolHandle pool, UInt64 offset,
                                                  const String& name = "Anonymous") = 0;

        virtual RHIImageViewHandle CreateImageView(RHIImageHandle image, RHIImageViewDesc&& desc) = 0;
        virtual bool ReleaseImageView(RHIImageViewHandle imageView) = 0;

        virtual RHISamplerHandle CreateSampler(RHISamplerDesc&& desc) = 0;
        virtual bool ReleaseSampler(RHISamplerHandle sampler) = 0;

        virtual RHISemaphoreHandle CreateSemaphore() = 0;
        virtual RHISemaphoreHandle CreateTimelineSemaphore(uint64_t initialValue = 0) = 0;
        virtual bool ReleaseSemaphore(RHISemaphoreHandle semaphore) = 0;



        virtual RHIAccelerationStructureHandle CreateAccelerationStructure(const String& name = "Anonymous") = 0;
        virtual bool ReleaseAccelerationStructure(RHIAccelerationStructureHandle handle) = 0;

        // Resource Management (Moved from RHIDevice)
        virtual void BufferMemoryCopy(RHIBufferHandle handle, const void* src, UInt64 size, UInt64 offset = 0) = 0;

        // Async transfer API - enqueues a copy and returns a ticket for later synchronization.
        // Caller must call FlushTransfers() or WaitForTicket() before using the buffer.
        virtual RHIGpuTicket BufferMemoryCopyAsync(RHIBufferHandle handle, const void* src, UInt64 size, UInt64 offset = 0) = 0;

        // Submit all pending async transfers. Returns the flush ticket (0 if nothing pending).
        virtual RHIGpuTicket FlushTransfers() = 0;

        // Poll GPU completion and reclaim staging memory. Call once per frame.
        virtual void UpdateTransfers() = 0;

        virtual void* MapBuffer(RHIBufferHandle handle) = 0;
        virtual void UnmapBuffer(RHIBufferHandle handle) = 0;
        virtual UInt64 GetBufferSize(RHIBufferHandle handle) = 0;
        virtual UInt64 GetBufferOffset(RHIBufferHandle handle) = 0;
        virtual UInt64 GetBufferRange(RHIBufferHandle handle) = 0;
        virtual UInt64 GetBufferDeviceAddress(RHIBufferHandle handle) = 0;

        // Resource Queries
        virtual RHIImageViewHandle FindImageViewForImage(RHIImageHandle image) = 0;
        virtual EFormat GetImageViewFormat(RHIImageViewHandle handle) = 0;
        virtual UInt32 GetImageViewWidth(RHIImageViewHandle handle) = 0;
        virtual UInt32 GetImageViewHeight(RHIImageViewHandle handle) = 0;

        virtual void SetGPUProgramSpecializationConstant(RHIShaderProgramHandle handle, UInt32 constantID, UInt32 size,
                                                         const void* data) = 0;

        virtual UInt32 RegisterBindlessResource(RHIImageViewHandle image) = 0;
        virtual UInt32 RegisterBindlessResource(RHIBufferHandle buffer) = 0;
        virtual UInt32 RegisterBindlessResource(RHISamplerHandle sampler) = 0;
        virtual bool UnregisterBindlessResourceImage(UInt32 bindlessIndex) = 0;
        virtual bool UnregisterBindlessResourceBuffer(UInt32 bindlessIndex) = 0;
        virtual bool UnregisterBindlessResourceSampler(UInt32 bindlessIndex) = 0;

        // Export boundaries use these owner-backed checks to reject stale generations
        // before dispatching an operation into a concrete backend.
        virtual bool IsAlive(RHIShaderProgramHandle handle) const = 0;
        virtual bool IsAlive(RHICommandBufferPoolHandle handle) const = 0;
        virtual bool IsAlive(RHIRenderPassHandle handle) const = 0;
        virtual bool IsAlive(RHIFrameBufferHandle handle) const = 0;
        virtual bool IsAlive(RHIBufferHandle handle) const = 0;
        virtual bool IsAlive(RHIImageHandle handle) const = 0;
        virtual bool IsAlive(RHIImageViewHandle handle) const = 0;
        virtual bool IsAlive(RHISamplerHandle handle) const = 0;
        virtual bool IsAlive(RHISemaphoreHandle handle) const = 0;
    };
}
