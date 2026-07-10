#include "RHI/Core/RHIFactory.h"
#include "RHI/Definitions/CoreRHICommon.h"
#include "RHI/Handles/RHIHandle.h"
#include "Base/BindingMacros.h"

using namespace ArisenEngine::RHI;

template <typename T>
static inline RHIHandle<T> MakeHandle(uint32_t index, uint32_t generation)
{
    RHIHandle<T> h;
    h.index = index;
    h.generation = generation;
    return h;
}

extern "C" {
RHI_DLL uint32_t RHIFactory_RegisterBindlessResourceSampler(RHIFactory* f, uint32_t index, uint32_t generation)
{
    return f->RegisterBindlessResource(MakeHandle<RHISamplerTag>(index, generation));
}

RHI_DLL void RHIFactory_UnregisterBindlessResourceImage(RHIFactory* f, uint32_t bindlessIndex)
{
    f->UnregisterBindlessResourceImage(bindlessIndex);
}

RHI_DLL void RHIFactory_UnregisterBindlessResourceBuffer(RHIFactory* f, uint32_t bindlessIndex)
{
    f->UnregisterBindlessResourceBuffer(bindlessIndex);
}

RHI_DLL void RHIFactory_UnregisterBindlessResourceSampler(RHIFactory* f, uint32_t bindlessIndex)
{
    f->UnregisterBindlessResourceSampler(bindlessIndex);
}
} // extern "C"
