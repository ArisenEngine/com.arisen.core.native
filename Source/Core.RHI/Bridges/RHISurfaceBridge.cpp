#include "RHI/Diagnostics/RHIError.h"
// RHISurfaceBridge.cpp - extern "C" bridge for RHISurface
#include "RHI/Presentation/RHISurface.h"
#include "RHI/Presentation/RHISwapChain.h"
#include "Base/BindingMacros.h"

using namespace ArisenEngine;
using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHISurface", "Core.RHI.dll", "Arisen.Native.RHI")

extern "C" {
RHI_DLL void RHISurface_InitSwapChain(RHISurface* surface)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(surface, "RHISurface");
    if (!surface) return;
    surface->InitSwapChain();
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void* RHISurface_GetSwapChain(RHISurface* surface)
{
    RHI_ABI_GUARD()
    {
        const void* surfaceToken = surface;
        RHI_ABI_REQUIRE_POINTER(surface, "RHISurface");
    auto* swapChain = surface->GetSwapChain();
    if (!swapChain)
        ThrowInvalidState(__func__, "RHISwapChain", 0, "Surface swapchain is unavailable");
    return RegisterAbiOwner(swapChain, ERHIAbiOwnerType::SwapChain, surfaceToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHISurface_SetResolution(RHISurface* surface, UInt32 width, UInt32 height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(surface, "RHISurface");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Surface resolution must be non-zero");

    if (!surface) return;
    surface->SetResolution(width, height);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL bool RHISurface_TrySetResolution(RHISurface* surface, UInt32 width, UInt32 height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(surface, "RHISurface");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Surface resolution must be non-zero");

    return surface && surface->TrySetResolution(width, height);

    }
    RHI_ABI_CATCH_RETURN()
}
} // extern "C"

ARISEN_BIND_END_BRIDGE()
