#include "RHI/Diagnostics/RHIError.h"
#include "RHIAbiValidation.h"
// RHIQueueBridge.cpp - extern "C" bridge for RHIQueue virtual methods
#include "RHI/Core/RHIDevice.h"
#include "RHI/Queues/RHIQueue.h"
#include "RHI/Commands/RHICommandBuffer.h"
#include "Base/BindingMacros.h"

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHIQueue", "Core.RHI.dll", "Arisen.Native.RHI")

extern "C" {

RHI_DLL uint64_t RHIQueue_Submit(RHIQueue* queue, uint32_t cbIndex, uint32_t cbGeneration, RHISubmitDescriptor* descriptor)
{
    RHI_ABI_GUARD()
    {
        const void* queueToken = queue;
        RHI_ABI_REQUIRE_POINTER(queue, "RHIQueue");
    RHICommandBufferHandle handle;
    handle.index = cbIndex;
    handle.generation = cbGeneration;
    auto* device = ResolveAbiOwnerParent<RHIDevice>(
        queueToken, ERHIAbiOwnerType::Device, __func__);
    if (!device->GetCommandBuffer(handle))
        ThrowInvalidHandle(__func__, "RHICommandBuffer", cbIndex, cbGeneration);
    if (!descriptor)
        return queue->Submit(handle);

    RHISubmitDescriptor resolvedDescriptor = *descriptor;
    if (resolvedDescriptor.WaitSwapChain)
        resolvedDescriptor.WaitSwapChain = ResolveAbiOwner(
            resolvedDescriptor.WaitSwapChain, ERHIAbiOwnerType::SwapChain, __func__);
    if (resolvedDescriptor.SignalSwapChain)
        resolvedDescriptor.SignalSwapChain = ResolveAbiOwner(
            resolvedDescriptor.SignalSwapChain, ERHIAbiOwnerType::SwapChain, __func__);
    RHI_ABI_REQUIRE_ARRAY(resolvedDescriptor.pWaitSemaphores,
                          resolvedDescriptor.waitSemaphoreCount,
                          "RHISemaphoreHandle");
    RHI_ABI_REQUIRE_ARRAY(resolvedDescriptor.pWaitDstStageMask,
                          resolvedDescriptor.waitSemaphoreCount,
                          "EPipelineStageFlag");
    RHI_ABI_REQUIRE_ARRAY(resolvedDescriptor.pSignalSemaphores,
                          resolvedDescriptor.signalSemaphoreCount,
                          "RHISemaphoreHandle");
    for (uint32_t i = 0; i < resolvedDescriptor.waitSemaphoreCount; ++i)
    {
        ABI::RequireMask(resolvedDescriptor.pWaitDstStageMask[i],
                         ABI::PipelineStageMask,
                         false,
                         __func__,
                         "pWaitDstStageMask[i]");
    }
    auto* factory = device->GetFactory();
    if (!factory)
        ThrowInvalidState(__func__, "RHIFactory", 0, "RHI factory is unavailable");
    for (uint32_t i = 0; i < resolvedDescriptor.waitSemaphoreCount; ++i)
    {
        const auto semaphore = resolvedDescriptor.pWaitSemaphores[i];
        if (!factory->IsAlive(semaphore))
            ThrowInvalidHandle(__func__, "RHISemaphore", semaphore.index, semaphore.generation);
    }
    for (uint32_t i = 0; i < resolvedDescriptor.signalSemaphoreCount; ++i)
    {
        const auto semaphore = resolvedDescriptor.pSignalSemaphores[i];
        if (!factory->IsAlive(semaphore))
            ThrowInvalidHandle(__func__, "RHISemaphore", semaphore.index, semaphore.generation);
    }
    return queue->Submit(handle, &resolvedDescriptor);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIQueue_Update(RHIQueue* queue)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(queue, "RHIQueue");
    queue->Update();
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL uint64_t RHIQueue_GetCompletedTicket(RHIQueue* queue)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(queue, "RHIQueue");
    return queue->GetCompletedTicket();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint64_t RHIQueue_GetLatestTicket(RHIQueue* queue)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(queue, "RHIQueue");
    return queue->GetLatestTicket();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIQueue_WaitForTicket(RHIQueue* queue, uint64_t ticket)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(queue, "RHIQueue");
    ABI::RequireSubmittedTicket(ticket, queue->GetLatestTicket(), __func__);
    queue->WaitForTicket(ticket);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL int RHIQueue_GetType(RHIQueue* queue)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(queue, "RHIQueue");
    return static_cast<int>(queue->GetType());
    }
    RHI_ABI_CATCH_RETURN()
}

} // extern "C"

ARISEN_BIND_END_BRIDGE()

