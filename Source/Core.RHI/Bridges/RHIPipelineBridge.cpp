#include "RHI/Diagnostics/RHIError.h"
#include "RHIAbiValidation.h"
#include "RHI/Pipeline/RHIPipeline.h"
#include "RHI/Pipeline/RHIPipelineCache.h"
#include "RHI/Pipeline/RHIPipelineState.h"
#include "Base/BindingMacros.h"

#include <cmath>

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHIPipeline", "Core.RHI.dll", "Arisen.Native.RHI")

namespace
{
    RHIPipelineState* ResolveCompatiblePipelineState(RHIPipelineCache* cache,
                                                     RHIPipelineState* pipelineState,
                                                     const char* operation)
    {
        if (pipelineState->GetOwnerDevice() != cache->GetOwnerDevice())
            ThrowInvalidParameter(operation, "pso", "Pipeline state belongs to a different RHI device");
        return pipelineState;
    }

    bool IsBufferDescriptorType(EDescriptorType type)
    {
        return type == DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
            type == DESCRIPTOR_TYPE_STORAGE_BUFFER ||
            type == DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
            type == DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    }

    void RequireFinite(float value, const char* operation, const char* parameter)
    {
        if (!std::isfinite(value))
            ThrowInvalidParameter(operation, parameter, "Value must be finite");
    }
}

extern "C" {
RHI_DLL void* RHIPipelineCache_GetGraphicsPipeline(RHIPipelineCache* cache, RHIPipelineState* pso)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cache, "RHIPipelineCache");
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
        pso = ResolveCompatiblePipelineState(cache, pso, __func__);
    RHIPipelineHandle handle = cache->GetGraphicsPipeline(pso);
    if (!handle.IsValid())
        ThrowInvalidState(__func__, "RHIPipeline", 0, "Pipeline cache returned an invalid handle");
    uint64_t result = 0;
    std::memcpy(&result, &handle, sizeof(handle));
    return (void*)result;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHIPipelineCache_GetComputePipeline(RHIPipelineCache* cache, RHIPipelineState* pso)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cache, "RHIPipelineCache");
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
        pso = ResolveCompatiblePipelineState(cache, pso, __func__);

    RHIPipelineHandle handle = cache->GetComputePipeline(pso);
    if (!handle.IsValid())
        ThrowInvalidState(__func__, "RHIPipeline", 0, "Pipeline cache returned an invalid handle");
    uint64_t result = 0;
    std::memcpy(&result, &handle, sizeof(handle));
    return (void*)result;

    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIPipelineCache_ReleasePipeline(
    RHIPipelineCache* cache,
    uint32_t index,
    uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cache, "RHIPipelineCache");

    RHIPipelineHandle handle{index, generation};
    if (!cache->IsAlive(handle))
        ThrowInvalidHandle(__func__, "RHIPipeline", index, generation);
    if (!cache->ReleasePipeline(handle))
    {
        ThrowInvalidState(__func__, "RHIPipeline", 0,
                          "Backend rejected pipeline ownership release",
                          index, generation);
    }

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void* RHIPipelineCache_GetPipelineState(RHIPipelineCache* cache)
{
    RHI_ABI_GUARD()
    {
        const void* cacheToken = cache;
        RHI_ABI_REQUIRE_POINTER(cache, "RHIPipelineCache");
    auto state = cache->GetPipelineState();
    if (!state)
        ThrowInvalidState(__func__, "RHIPipelineState", 0, "Pipeline cache returned a null state");
    return RegisterAbiOwner(state.release(), ERHIAbiOwnerType::PipelineState, cacheToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIPipelineState_AddProgram(RHIPipelineState* pso, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
    RHIShaderProgramHandle handle;
    handle.index = index;
    handle.generation = generation;
    if (!pso->IsAlive(handle))
        ThrowInvalidHandle(__func__, "RHIShaderProgram", index, generation);
    pso->AddProgram(handle);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_SetBindPoint(RHIPipelineState* pso, int bindPoint)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
    pso->SetBindPoint(ABI::RequireEnum<EPipelineBindPoint>(bindPoint, __func__, "bindPoint"));
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_SetInputAssemblyState(RHIPipelineState* pso, int topology, int primitiveRestart)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");

    RHIInputAssemblyState state{};
    state.topology = ABI::RequireEnum<EPrimitiveTopology>(topology, __func__, "topology");
    state.primitiveRestartEnable = primitiveRestart != 0;
    pso->SetInputAssemblyState(state);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_AddVertexBindingDescription(RHIPipelineState* pso, uint32_t binding, uint32_t stride,
                                                          int inputRate)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");

    pso->AddVertexBindingDescription(
        binding, stride, ABI::RequireEnum<EVertexInputRate>(inputRate, __func__, "inputRate"));

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_AddVertexInputAttributeDescription(RHIPipelineState* pso, uint32_t location,
                                                                 uint32_t binding, int format, uint32_t offset)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");

    pso->AddVertexInputAttributeDescription(
        location, binding, ABI::RequireEnum<EFormat>(format, __func__, "format"), offset);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_ClearVertexInputDescriptions(RHIPipelineState* pso)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");

    pso->ClearVertexInputDescriptions();

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_SetRasterizationState(RHIPipelineState* pso, int polygonMode, int cullMode, int frontFace)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
    RHIRasterizationState state{};
    state.polygonMode = ABI::RequireEnum<EPolygonMode>(polygonMode, __func__, "polygonMode");
    state.cullMode = ABI::RequireEnum<ECullModeFlagBits>(cullMode, __func__, "cullMode");
    state.frontFace = ABI::RequireEnum<EFrontFace>(frontFace, __func__, "frontFace");
    state.depthClampEnable = false;
    state.rasterizerDiscardEnable = false;
    state.depthBiasEnable = false;
    state.lineWidth = 1.0f;
    pso->SetRasterizationState(state);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_SetRasterizationStateWithDepthBias(
    RHIPipelineState* pso,
    int polygonMode,
    int cullMode,
    int frontFace,
    float depthBiasConstantFactor,
    float depthBiasClamp,
    float depthBiasSlopeFactor)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
        RequireFinite(depthBiasConstantFactor, __func__, "depthBiasConstantFactor");
        RequireFinite(depthBiasClamp, __func__, "depthBiasClamp");
        RequireFinite(depthBiasSlopeFactor, __func__, "depthBiasSlopeFactor");

    RHIRasterizationState state{};
    state.polygonMode = ABI::RequireEnum<EPolygonMode>(polygonMode, __func__, "polygonMode");
    state.cullMode = ABI::RequireEnum<ECullModeFlagBits>(cullMode, __func__, "cullMode");
    state.frontFace = ABI::RequireEnum<EFrontFace>(frontFace, __func__, "frontFace");
    state.depthClampEnable = false;
    state.rasterizerDiscardEnable = false;
    state.depthBiasEnable = true;
    state.depthBiasConstantFactor = depthBiasConstantFactor;
    state.depthBiasClamp = depthBiasClamp;
    state.depthBiasSlopeFactor = depthBiasSlopeFactor;
    state.lineWidth = 1.0f;
    pso->SetRasterizationState(state);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_SetColorBlendState(RHIPipelineState* pso, int blendEnable, int srcColor, int dstColor,
                                                 int colorOp)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");

    RHIColorBlendState state{};
    RHIColorBlendAttachmentState attachment{};
    attachment.blendEnable = blendEnable != 0;
    attachment.srcColorBlendFactor = ABI::RequireEnum<EBlendFactor>(srcColor, __func__, "srcColor");
    attachment.dstColorBlendFactor = ABI::RequireEnum<EBlendFactor>(dstColor, __func__, "dstColor");
    attachment.colorBlendOp = ABI::RequireEnum<EBlendOp>(colorOp, __func__, "colorOp");
    attachment.srcAlphaBlendFactor = attachment.srcColorBlendFactor;
    attachment.dstAlphaBlendFactor = attachment.dstColorBlendFactor;
    attachment.alphaBlendOp = attachment.colorBlendOp;
    attachment.colorWriteMask = 0xF; // RGBA
    state.attachments.push_back(attachment);
    pso->SetColorBlendState(state);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_SetDepthStencilState(RHIPipelineState* pso, int depthTestEnable, int depthWriteEnable,
                                                   int depthCompareOp)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");

    RHIDepthStencilState state{};
    state.depthTestEnable = depthTestEnable != 0;
    state.depthWriteEnable = depthWriteEnable != 0;
    state.depthCompareOp = ABI::RequireEnum<ECompareOp>(depthCompareOp, __func__, "depthCompareOp");
    state.depthBoundsTestEnable = false;
    state.stencilTestEnable = false;
    pso->SetDepthStencilState(state);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_SetDynamicStateMask(RHIPipelineState* pso, uint64_t mask)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");

    pso->SetDynamicStateMask(
        ABI::RequireMask(mask, ABI::DynamicStateMask, true, __func__, "mask"));
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_SetRenderingFormats(RHIPipelineState* pso, const int* colorFormats, uint32_t colorCount,
                                                  int depthFormat)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
        RHI_ABI_REQUIRE_ARRAY(colorFormats, colorCount, "EFormat");
    ArisenEngine::Containers::Vector<EFormat> formats;
    for (uint32_t i = 0; i < colorCount; ++i)
        formats.push_back(ABI::RequireEnum<EFormat>(colorFormats[i], __func__, "colorFormats[i]"));
    pso->SetRenderingFormats(
        formats, ABI::RequireEnum<EFormat>(depthFormat, __func__, "depthFormat"),
        EFormat::FORMAT_UNDEFINED);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_UpdateDescriptorSetBuffer(RHIPipelineState* pso, uint32_t layoutIndex, uint32_t binding,
                                                        const uint32_t* indices, const uint32_t* generations,
                                                        uint32_t count)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
        RHI_ABI_REQUIRE_ARRAY(indices, count, "buffer handle index");
        RHI_ABI_REQUIRE_ARRAY(generations, count, "buffer handle generation");
        if (count == 0)
            ThrowInvalidParameter(__func__, "count", "Descriptor update must contain at least one buffer");
    ArisenEngine::Containers::Vector<RHIBufferHandle> buffers;
    buffers.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        RHIBufferHandle handle{indices[i], generations[i]};
        if (!pso->IsAlive(handle))
            ThrowInvalidHandle(__func__, "RHIBuffer", handle.index, handle.generation);
        buffers.push_back(handle);
    }
        EDescriptorType descriptorType = DESCRIPTOR_TYPE_MAX_ENUM;
        uint32_t descriptorCount = 0;
        if (!pso->TryGetDescriptorBinding(layoutIndex, binding, descriptorType, descriptorCount))
            ThrowInvalidParameter(__func__, "layoutIndex/binding", "Descriptor binding is not declared");
        if (!IsBufferDescriptorType(descriptorType))
            ThrowInvalidParameter(__func__, "binding", "Descriptor binding does not accept buffer handles");
        if (count != descriptorCount)
            ThrowInvalidParameter(__func__, "count", "Buffer count does not match the descriptor binding count");
    pso->UpdateDescriptorSet(layoutIndex, binding, std::move(buffers));
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_BuildDescriptorSetLayout(RHIPipelineState* pso)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
    pso->BuildDescriptorSetLayout();
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIPipelineState_Delete(RHIPipelineState* pso)
{
    RHI_ABI_GUARD()
    {
        const void* pipelineStateToken = pso;
        RHI_ABI_REQUIRE_POINTER(pso, "RHIPipelineState");
    InvalidateAbiOwner(pipelineStateToken, ERHIAbiOwnerType::PipelineState);
    delete pso;
    }
    RHI_ABI_CATCH_VOID()
}
} // extern "C"

ARISEN_BIND_END_BRIDGE()

