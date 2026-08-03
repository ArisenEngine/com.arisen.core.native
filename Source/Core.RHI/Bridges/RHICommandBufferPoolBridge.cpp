#include "RHI/Diagnostics/RHIError.h"
#include "RHIAbiValidation.h"
// RHICommandBufferPoolBridge.cpp
#include "RHI/Commands/RHICommandBufferPool.h"
#include "RHI/Commands/RHICommandBuffer.h"
#include "RHI/Core/RHIDevice.h"
#include "Base/BindingMacros.h"

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHICommandBufferPool", "Core.RHI.dll", "Arisen.Native.RHI")

extern "C" {
RHI_DLL void RHICommandBufferPool_GetCommandBuffer(RHICommandBufferPool* pool, uint32_t currentFrameIndex, int level,
                                                   uint32_t* outIndex, uint32_t* outGeneration)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pool, "RHICommandBufferPool");
        RHI_ABI_REQUIRE_POINTER(outIndex, "uint32_t");
        RHI_ABI_REQUIRE_POINTER(outGeneration, "uint32_t");
    RHICommandBufferHandle handle = pool->GetCommandBuffer(
        currentFrameIndex, ABI::RequireEnum<ECommandBufferLevel>(level, __func__, "level"));
    if (!handle.IsValid())
        ThrowInvalidState(__func__, "RHICommandBuffer", 0, "Pool returned an invalid command-buffer handle");
    *outIndex = handle.index;
    *outGeneration = handle.generation;
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBufferPool_ReleaseCommandBuffer(RHICommandBufferPool* pool, uint32_t currentFrameIndex,
                                                       uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        const void* poolToken = pool;
        RHI_ABI_REQUIRE_POINTER(pool, "RHICommandBufferPool");
    RHICommandBufferHandle handle;
    handle.index = index;
    handle.generation = generation;
    if (!pool->IsAlive(handle))
        ThrowInvalidHandle(__func__, "RHICommandBuffer", index, generation);
    auto* device = ResolveAbiOwnerParent<RHIDevice>(
        poolToken, ERHIAbiOwnerType::Device, __func__);
    auto* commandBuffer = device->GetCommandBuffer(handle);
    if (!commandBuffer)
        ThrowInvalidHandle(__func__, "RHICommandBuffer", index, generation);
    pool->ReleaseCommandBuffer(currentFrameIndex, handle);
    InvalidateAbiOwnerByObject(commandBuffer, ERHIAbiOwnerType::CommandBuffer);
    }
    RHI_ABI_CATCH_VOID()
}
} // extern "C"

ARISEN_BIND_END_BRIDGE()

