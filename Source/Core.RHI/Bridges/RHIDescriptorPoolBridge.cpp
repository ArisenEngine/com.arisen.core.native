#include "RHI/Diagnostics/RHIError.h"
#include "RHIAbiValidation.h"
// RHIDescriptorPoolBridge.cpp - extern "C" bridge for RHIDescriptorPool
#include "RHI/Descriptors/RHIDescriptorPool.h"
#include "RHI/Pipeline/RHIPipelineState.h"
#include "Base/BindingMacros.h"

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHIDescriptorPool", "Core.RHI.dll", "Arisen.Native.RHI")

namespace
{
    void RequirePool(RHIDescriptorPool* pool, uint32_t poolId, const char* operation)
    {
        if (!pool->IsPoolAlive(poolId))
            ThrowInvalidParameter(operation, "poolId", "Descriptor pool ID is not live");
    }

    RHIPipelineState* ResolveCompatiblePipelineState(RHIDescriptorPool* pool,
                                                     RHIPipelineState* pipelineState,
                                                     const char* operation)
    {
        if (pipelineState->GetOwnerDevice() != pool->GetOwnerDevice())
            ThrowInvalidParameter(operation, "pso", "Pipeline state belongs to a different RHI device");
        return pipelineState;
    }
}

extern "C" {

RHI_DLL uint32_t RHIDescriptorPool_AddPool(RHIDescriptorPool* pool, int* types, uint32_t* counts, uint32_t typeCount, uint32_t maxSets)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pool, "RHIDescriptorPool");
        RHI_ABI_REQUIRE_ARRAY(types, typeCount, "EDescriptorType");
        RHI_ABI_REQUIRE_ARRAY(counts, typeCount, "uint32_t");
        if (typeCount == 0 || maxSets == 0)
            ThrowInvalidParameter(__func__, "typeCount/maxSets", "Descriptor-pool counts must be non-zero");
    ArisenEngine::Containers::Vector<EDescriptorType> vTypes;
    ArisenEngine::Containers::Vector<ArisenEngine::UInt32> vCounts;
    for (uint32_t i = 0; i < typeCount; ++i)
    {
        if (counts[i] == 0)
            ThrowInvalidParameter(__func__, "counts[i]", "Descriptor count must be non-zero");
        vTypes.push_back(ABI::RequireEnum<EDescriptorType>(types[i], __func__, "types[i]"));
        vCounts.push_back(static_cast<ArisenEngine::UInt32>(counts[i]));
    }
    return pool->AddPool(std::move(vTypes), std::move(vCounts), static_cast<ArisenEngine::UInt32>(maxSets));
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL bool RHIDescriptorPool_ResetPool(RHIDescriptorPool* pool, uint32_t poolId)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pool, "RHIDescriptorPool");
        RequirePool(pool, poolId, __func__);
    return pool->ResetPool(poolId);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint32_t RHIDescriptorPool_AllocDescriptorSet(RHIDescriptorPool* pool, uint32_t poolId, uint32_t layoutIndex, RHIPipelineState* pso)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pool, "RHIDescriptorPool");
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
        RequirePool(pool, poolId, __func__);
        pso = ResolveCompatiblePipelineState(pool, pso, __func__);
        if (!pso->IsDescriptorSetLayoutAlive(layoutIndex))
            ThrowInvalidParameter(__func__, "layoutIndex", "Descriptor set layout is not built or out of range");
    return pool->AllocDescriptorSet(poolId, layoutIndex, pso);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIDescriptorPool_UpdateDescriptorSet(RHIDescriptorPool* pool, uint32_t poolId, uint32_t setIndex, RHIPipelineState* pso)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pool, "RHIDescriptorPool");
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
        RequirePool(pool, poolId, __func__);
        pso = ResolveCompatiblePipelineState(pool, pso, __func__);
        if (!pool->IsDescriptorSetAlive(poolId, setIndex))
            ThrowInvalidParameter(__func__, "setIndex", "Descriptor set index is not live");
    pool->UpdateDescriptorSet(poolId, setIndex, pso);
    }
    RHI_ABI_CATCH_VOID()
}

} // extern "C"

ARISEN_BIND_END_BRIDGE()

