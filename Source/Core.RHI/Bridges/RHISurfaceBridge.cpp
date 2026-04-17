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
    if (!surface) return;
    surface->InitSwapChain();
}

RHI_DLL void* RHISurface_GetSwapChain(RHISurface* surface)
{
    if (!surface)
    {
        return nullptr;
    }
    return static_cast<void*>(surface->GetSwapChain());
}

RHI_DLL void RHISurface_SetResolution(RHISurface* surface, UInt32 width, UInt32 height)
{
    if (!surface) return;
    surface->SetResolution(width, height);
}
} // extern "C"

ARISEN_BIND_END_BRIDGE()
