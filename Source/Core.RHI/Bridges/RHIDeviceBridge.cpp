#include "RHI/Diagnostics/RHIError.h"
#include "RHIAbiValidation.h"
// RHIDeviceBridge.cpp - extern "C" bridge for RHIDevice virtual methods
#include "RHI/Core/RHIDevice.h"
#include "RHI/Core/RHIFactory.h"
#include "RHI/Core/RHIInstance.h"
#include "RHI/Definitions/CoreRHICommon.h"
#include "RHI/Handles/RHIHandle.h"
#include "RHI/Commands/RHICommandBuffer.h"
#include "Base/BindingMacros.h"
#include <cstring>

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHIDevice", "Core.RHI.dll", "Arisen.Native.RHI")

extern "C" {
ARISEN_BIND_STRUCT(RHISubmitDescriptor_Bridge)

struct RHISubmitDescriptor_Bridge
{
    RHISwapChain* waitSwapChain = nullptr;
    RHISwapChain* signalSwapChain = nullptr;
    uint32_t swapChainFrameIndex = 0;
    RHISemaphoreHandle* pWaitSemaphores = nullptr;
    uint32_t waitSemaphoreCount = 0;
    RHISemaphoreHandle* pSignalSemaphores = nullptr;
    uint32_t signalSemaphoreCount = 0;
};

RHI_DLL void RHIDevice_DeviceWaitIdle(RHIDevice* dev)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    dev->DeviceWaitIdle();
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIDevice_GraphicQueueWaitIdle(RHIDevice* dev)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    dev->GraphicQueueWaitIdle();
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL uint32_t RHIDevice_GetMaxFramesInFlight(RHIDevice* dev)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    return dev->GetMaxFramesInFlight();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHIDevice_GetFactory(RHIDevice* dev)
{
    RHI_ABI_GUARD()
    {
        const void* deviceToken = dev;
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    auto* factory = dev->GetFactory();
    if (!factory)
        ThrowInvalidState(__func__, "RHIFactory", 0, "RHI factory is unavailable");
    return RegisterAbiOwner(factory, ERHIAbiOwnerType::Factory, deviceToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHIDevice_GetInstance(RHIDevice* dev)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    auto* instance = dev->GetInstance();
    if (!instance)
        ThrowInvalidState(__func__, "RHIInstance", 0, "Owning RHI instance is unavailable");
    return RegisterAbiOwner(instance, ERHIAbiOwnerType::Instance);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIDevice_SetResolution(RHIDevice* dev, uint32_t width, uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Device resolution must be non-zero");
    dev->SetResolution(width, height);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIDevice_SetObjectName(RHIDevice* dev, int objectType, uint64_t handle, const char* name)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
        RHI_ABI_REQUIRE_POINTER(name, "UTF-8 object name");
        if (handle == 0)
            ThrowInvalidParameter(__func__, "handle", "Native object handle must be non-zero");
    dev->SetObjectName(ABI::RequireEnum<ERHIObjectType>(objectType, __func__, "objectType"),
                       handle, name);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIDevice_GetCapabilities(RHIDevice* dev, RHICapabilities* outCapabilities)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
        RHI_ABI_REQUIRE_POINTER(outCapabilities, "RHICapabilities");
    *outCapabilities = dev->GetCapabilities();
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void* RHIDevice_GetCommandBuffer(RHIDevice* dev, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        const void* deviceToken = dev;
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    RHICommandBufferHandle handle;
    handle.index = index;
    handle.generation = generation;
    auto* commandBuffer = dev->GetCommandBuffer(handle);
    if (!commandBuffer)
        ThrowInvalidHandle(__func__, "RHICommandBuffer", index, generation);
    auto* ownerPool = commandBuffer->GetOwner();
    if (!ownerPool)
        ThrowInvalidState(__func__, "RHICommandBufferPool", 0,
                          "Command buffer does not have a live owner pool");
    const void* poolToken = RegisterAbiOwner(
        ownerPool, ERHIAbiOwnerType::CommandBufferPool, deviceToken);
    return RegisterAbiOwner(commandBuffer, ERHIAbiOwnerType::CommandBuffer, poolToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHIDevice_GetCommandBufferPool(RHIDevice* dev, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        const void* deviceToken = dev;
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    RHICommandBufferPoolHandle handle;
    handle.index = index;
    handle.generation = generation;
    auto* pool = dev->GetCommandBufferPool(handle);
    if (!pool)
        ThrowInvalidHandle(__func__, "RHICommandBufferPool", index, generation);
    return RegisterAbiOwner(pool, ERHIAbiOwnerType::CommandBufferPool, deviceToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint64_t RHIDevice_GetCompletedSubmitTicket(RHIDevice* dev)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");

    auto* queue = dev->GetQueue(RHIQueueType::Graphics);
    if (!queue)
        ThrowInvalidState(__func__, "RHIQueue", 0, "Graphics queue is unavailable");

    queue->Update();
    return queue->GetCompletedTicket();

    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIDevice_WaitQueueTicket(RHIDevice* dev, uint64_t ticket)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    auto* queue = dev->GetQueue(RHIQueueType::Graphics);
    if (!queue)
        ThrowInvalidState(__func__, "RHIQueue", 0, "Graphics queue is unavailable");
    ABI::RequireSubmittedTicket(ticket, queue->GetLatestTicket(), __func__);
    queue->WaitForTicket(ticket);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL uint64_t RHIDevice_Submit(RHIDevice* dev, uint32_t index, uint32_t generation,
                                  RHISubmitDescriptor_Bridge* bridgeDesc)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    RHICommandBufferHandle handle;
    handle.index = index;
    handle.generation = generation;
    if (!dev->GetCommandBuffer(handle))
        ThrowInvalidHandle(__func__, "RHICommandBuffer", index, generation);

    auto* queue = dev->GetQueue(RHIQueueType::Graphics);
    if (!queue)
        ThrowInvalidState(__func__, "RHIQueue", 0, "Graphics queue is unavailable");

    if (bridgeDesc)
    {
        RHI_ABI_REQUIRE_ARRAY(bridgeDesc->pWaitSemaphores, bridgeDesc->waitSemaphoreCount,
                              "RHISemaphoreHandle");
        RHI_ABI_REQUIRE_ARRAY(bridgeDesc->pSignalSemaphores, bridgeDesc->signalSemaphoreCount,
                              "RHISemaphoreHandle");
        auto* factory = dev->GetFactory();
        if (!factory)
            ThrowInvalidState(__func__, "RHIFactory", 0, "RHI factory is unavailable");
        auto* waitSemaphores = bridgeDesc->pWaitSemaphores;
        auto* signalSemaphores = bridgeDesc->pSignalSemaphores;
        for (uint32_t i = 0; i < bridgeDesc->waitSemaphoreCount; ++i)
        {
            if (!factory->IsAlive(waitSemaphores[i]))
            {
                const auto invalid = waitSemaphores[i];
                ThrowInvalidHandle(__func__, "RHISemaphore", invalid.index, invalid.generation);
            }
        }
        for (uint32_t i = 0; i < bridgeDesc->signalSemaphoreCount; ++i)
        {
            if (!factory->IsAlive(signalSemaphores[i]))
            {
                const auto invalid = signalSemaphores[i];
                ThrowInvalidHandle(__func__, "RHISemaphore", invalid.index, invalid.generation);
            }
        }
        RHISubmitDescriptor desc{};
        desc.WaitSwapChain = bridgeDesc->waitSwapChain;
        desc.SignalSwapChain = bridgeDesc->signalSwapChain;
        if (desc.WaitSwapChain)
            desc.WaitSwapChain = ResolveAbiOwner(
                desc.WaitSwapChain, ERHIAbiOwnerType::SwapChain, __func__);
        if (desc.SignalSwapChain)
            desc.SignalSwapChain = ResolveAbiOwner(
                desc.SignalSwapChain, ERHIAbiOwnerType::SwapChain, __func__);
        desc.SwapChainFrameIndex = bridgeDesc->swapChainFrameIndex;
        desc.pWaitSemaphores = waitSemaphores;
        desc.waitSemaphoreCount = bridgeDesc->waitSemaphoreCount;
        desc.pSignalSemaphores = signalSemaphores;
        desc.signalSemaphoreCount = bridgeDesc->signalSemaphoreCount;
        return queue->Submit(handle, &desc);
    }

    return queue->Submit(handle);
    }
    RHI_ABI_CATCH_RETURN()
}
 
RHI_DLL void* RHIDevice_GetQueue(RHIDevice* dev, int queueType)
{
    RHI_ABI_GUARD()
    {
        const void* deviceToken = dev;
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    if (queueType < static_cast<int>(RHIQueueType::Graphics) ||
        queueType > static_cast<int>(RHIQueueType::Present))
        ThrowInvalidParameter(__func__, "queueType", "Queue type is outside the supported range");
    auto* queue = dev->GetQueue(static_cast<RHIQueueType>(queueType));
    if (!queue)
        ThrowInvalidState(__func__, "RHIQueue", 0, "Requested queue is unavailable");
    return RegisterAbiOwner(queue, ERHIAbiOwnerType::Queue, deviceToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHIDevice_GetPipelineCache(RHIDevice* dev)
{
    RHI_ABI_GUARD()
    {
        const void* deviceToken = dev;
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    auto* cache = dev->GetPipelineCache();
    if (!cache)
        ThrowInvalidState(__func__, "RHIPipelineCache", 0, "Pipeline cache is unavailable");
    return RegisterAbiOwner(cache, ERHIAbiOwnerType::PipelineCache, deviceToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHIDevice_GetSurface(RHIDevice* dev)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    auto* surface = dev->GetSurface();
    auto* instance = dev->GetInstance();
    if (!surface || !instance)
        ThrowInvalidState(__func__, "RHISurface", 0, "Device surface is unavailable");
    const void* instanceToken = RegisterAbiOwner(instance, ERHIAbiOwnerType::Instance);
    return RegisterAbiOwner(surface, ERHIAbiOwnerType::Surface, instanceToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHIDevice_GetDescriptorPool(RHIDevice* dev)
{
    RHI_ABI_GUARD()
    {
        const void* deviceToken = dev;
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    auto* pool = dev->GetDescriptorPool();
    if (!pool)
        ThrowInvalidState(__func__, "RHIDescriptorPool", 0, "Descriptor pool is unavailable");
    return RegisterAbiOwner(pool, ERHIAbiOwnerType::DescriptorPool, deviceToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint64_t RHIDevice_GetDescriptorPoolHandle(RHIDevice* dev)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");
    RHIDescriptorPoolHandle handle = dev->GetDescriptorPoolHandle();
    static_assert(sizeof(handle) == sizeof(uint64_t));
    uint64_t packedHandle = 0;
    std::memcpy(&packedHandle, &handle, sizeof(handle));
    return packedHandle;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHIDevice_GetSharedWin32Handle(RHIDevice* dev, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(dev, "RHIDevice");

    RHIImageHandle handle;
    handle.index = index;
    handle.generation = generation;
    auto* factory = dev->GetFactory();
    if (!factory || !factory->IsAlive(handle))
        ThrowInvalidHandle(__func__, "RHIImage", index, generation);
    return dev->GetSharedWin32Handle(handle);

    }
    RHI_ABI_CATCH_RETURN()
}
} // extern "C"


ARISEN_BIND_END_BRIDGE()


