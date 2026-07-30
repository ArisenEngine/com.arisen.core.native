#include "RHI/Presentation/RHISwapChain.h"
#include "RHI/Handles/RHIHandle.h"
#include "Base/BindingMacros.h"

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHISwapChain", "Core.RHI.dll", "Arisen.Native.RHI")

extern "C" {
typedef ArisenEngine::RHI::RHISwapChain SwapChain;
typedef ArisenEngine::RHI::RHIImageHandle ImageHandle;
typedef ArisenEngine::RHI::RHIImageViewHandle ImageViewHandle;

RHI_DLL uint64_t RHISwapChain_BeginFrame(SwapChain* swapChain, uint32_t frameIndex)
{
    if (!swapChain)
    {
        printf("[DEBUG] RHISwapChain_BeginFrame: swapChain is NULL!\n");
        return 0;
    }
    printf("[DEBUG] RHISwapChain_BeginFrame: swapChain=%p, frameIndex=%u\n", swapChain, frameIndex);
    auto handle = swapChain->BeginFrame(frameIndex);
    return *reinterpret_cast<uint64_t*>(&handle);
}

RHI_DLL void RHISwapChain_EndFrame(SwapChain* swapChain, uint32_t frameIndex)
{
    if (!swapChain) return;
    swapChain->EndFrame(frameIndex);
}

RHI_DLL uint64_t RHISwapChain_GetImageView(SwapChain* swapChain, uint32_t frameIndex)
{
    if (!swapChain) return 0;
    auto handle = swapChain->GetImageView(frameIndex);
    return *reinterpret_cast<uint64_t*>(&handle);
}

RHI_DLL void* RHISwapChain_GetSharedWin32Handle(SwapChain* swapChain, uint32_t frameIndex)
{
    if (!swapChain) return nullptr;
    return swapChain->GetSharedWin32Handle(frameIndex);
}

RHI_DLL uint64_t RHISwapChain_GetSharedMemorySize(SwapChain* swapChain, uint32_t frameIndex)
{
    if (!swapChain) return 0;
    return swapChain->GetSharedMemorySize(frameIndex);
}

RHI_DLL void* RHISwapChain_GetRenderFinishedSemaphoreWin32Handle(SwapChain* swapChain, uint32_t frameIndex)
{
    if (!swapChain) return nullptr;
    return swapChain->GetRenderFinishedSemaphoreWin32Handle(frameIndex);
}

RHI_DLL void* RHISwapChain_CreateConsumedSemaphoreWin32Handle(SwapChain* swapChain, uint32_t frameIndex)
{
    if (!swapChain) return nullptr;
    return swapChain->CreateConsumedSemaphoreWin32Handle(frameIndex);
}

RHI_DLL void RHISwapChain_CompleteConsumedSemaphoreWin32Handle(SwapChain* swapChain, void* handle)
{
    if (!swapChain) return;
    swapChain->CompleteConsumedSemaphoreWin32Handle(handle);
}

RHI_DLL void RHISwapChain_ReleaseConsumedSemaphoreWin32Handle(SwapChain* swapChain, void* handle)
{
    if (!swapChain) return;
    swapChain->ReleaseConsumedSemaphoreWin32Handle(handle);
}

RHI_DLL bool RHISwapChain_AcknowledgeExternalConsumerRelease(SwapChain* swapChain)
{
    return swapChain && swapChain->AcknowledgeExternalConsumerRelease();
}

} // extern "C"


RHI_DLL void RHISwapChain_SetResolution(RHISwapChain* sc, uint32_t width, uint32_t height)
{
    sc->SetResolution(width, height);
}

ARISEN_BIND_END_BRIDGE()

