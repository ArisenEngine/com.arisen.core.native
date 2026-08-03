#include "RHI/Diagnostics/RHIError.h"
#include "RHI/Core/RHIFactory.h"
#include "RHI/Definitions/CoreRHICommon.h"
#include "RHI/Handles/RHIHandle.h"
#include "Base/BindingMacros.h"

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")

template <typename T>
static inline RHIHandle<T> MakeHandle(uint32_t index, uint32_t generation)
{
    RHIHandle<T> h;
    h.index = index;
    h.generation = generation;
    return h;
}

ARISEN_BIND_BEGIN_BRIDGE("RHIFactoryBindless", "Core.RHI.dll", "Arisen.Native.RHI")

extern "C" {
RHI_DLL uint32_t RHIFactory_RegisterBindlessResourceSampler(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHISamplerTag>(index, generation);
    if (!f->IsAlive(handle))
        ThrowInvalidHandle(__func__, "RHISampler", handle.index, handle.generation);
    const uint32_t bindlessIndex = f->RegisterBindlessResource(handle);
    if (bindlessIndex == UINT32_MAX)
        ThrowInvalidState(__func__, "RHIBindlessSamplerTable", 0, "Bindless sampler table is full");
    return bindlessIndex;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIFactory_UnregisterBindlessResourceImage(RHIFactory* f, uint32_t bindlessIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    if (!f->UnregisterBindlessResourceImage(bindlessIndex))
        ThrowInvalidParameter(__func__, "bindlessIndex", "Bindless image index is not registered");
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_UnregisterBindlessResourceBuffer(RHIFactory* f, uint32_t bindlessIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    if (!f->UnregisterBindlessResourceBuffer(bindlessIndex))
        ThrowInvalidParameter(__func__, "bindlessIndex", "Bindless buffer index is not registered");
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_UnregisterBindlessResourceSampler(RHIFactory* f, uint32_t bindlessIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    if (!f->UnregisterBindlessResourceSampler(bindlessIndex))
        ThrowInvalidParameter(__func__, "bindlessIndex", "Bindless sampler index is not registered");
    }
    RHI_ABI_CATCH_VOID()
}
} // extern "C"

ARISEN_BIND_END_BRIDGE()
