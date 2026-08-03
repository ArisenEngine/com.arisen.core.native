#include "RHI/Diagnostics/RHIError.h"
#include "RHI/Presentation/RHISwapChain.h"
#include "RHI/Handles/RHIHandle.h"
#include "Base/BindingMacros.h"
#include <cstring>

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHISwapChain", "Core.RHI.dll", "Arisen.Native.RHI")

extern "C" {
typedef ArisenEngine::RHI::RHISwapChain SwapChain;
typedef ArisenEngine::RHI::RHIImageHandle ImageHandle;
typedef ArisenEngine::RHI::RHIImageViewHandle ImageViewHandle;

RHI_DLL uint64_t RHISwapChain_BeginFrame(SwapChain* swapChain, uint32_t frameIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");
    auto handle = swapChain->BeginFrame(frameIndex);
    uint64_t packed = 0;
    std::memcpy(&packed, &handle, sizeof(handle));
    return packed;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHISwapChain_EndFrame(SwapChain* swapChain, uint32_t frameIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");
    swapChain->EndFrame(frameIndex);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL uint64_t RHISwapChain_RetireFrame(SwapChain* swapChain, uint32_t frameIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");
        return swapChain->RetireFrame(frameIndex);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint64_t RHISwapChain_GetImageView(SwapChain* swapChain, uint32_t frameIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");
    auto handle = swapChain->GetImageView(frameIndex);
    uint64_t packed = 0;
    std::memcpy(&packed, &handle, sizeof(handle));
    return packed;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHISwapChain_GetSharedWin32Handle(SwapChain* swapChain, uint32_t frameIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");
    return swapChain->GetSharedWin32Handle(frameIndex);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint64_t RHISwapChain_GetSharedMemorySize(SwapChain* swapChain, uint32_t frameIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");
    return swapChain->GetSharedMemorySize(frameIndex);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHISwapChain_GetRenderFinishedSemaphoreWin32Handle(SwapChain* swapChain, uint32_t frameIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");
    return swapChain->GetRenderFinishedSemaphoreWin32Handle(frameIndex);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHISwapChain_CreateConsumedSemaphoreWin32Handle(SwapChain* swapChain, uint32_t frameIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");
    return swapChain->CreateConsumedSemaphoreWin32Handle(frameIndex);

    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHISwapChain_CompleteConsumedSemaphoreWin32Handle(SwapChain* swapChain, void* handle)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");
        RHI_ABI_REQUIRE_POINTER(handle, "Win32 semaphore handle");
    swapChain->CompleteConsumedSemaphoreWin32Handle(handle);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHISwapChain_ReleaseConsumedSemaphoreWin32Handle(SwapChain* swapChain, void* handle)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");
        RHI_ABI_REQUIRE_POINTER(handle, "Win32 semaphore handle");
    swapChain->ReleaseConsumedSemaphoreWin32Handle(handle);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL bool RHISwapChain_AcknowledgeExternalConsumerRelease(SwapChain* swapChain)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(swapChain, "SwapChain");

    return swapChain->AcknowledgeExternalConsumerRelease();

    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHISwapChain_SetResolution(RHISwapChain* sc, uint32_t width, uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(sc, "RHISwapChain");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Swapchain resolution must be non-zero");
    sc->SetResolution(width, height);
    }
    RHI_ABI_CATCH_VOID()
}

} // extern "C"

ARISEN_BIND_END_BRIDGE()

