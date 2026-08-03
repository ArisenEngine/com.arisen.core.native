#include "RHI/Diagnostics/RHIError.h"
#include "RHIAbiValidation.h"
#include "RHI/Commands/RHICommandBuffer.h"
#include "Base/BindingMacros.h"
#include "Containers/Containers.h"

#include <cmath>

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHICommandBuffer", "Core.RHI.dll", "Arisen.Native.RHI")

namespace
{
    template <typename THandle>
    void RequireCommandHandle(RHICommandBuffer* commandBuffer, THandle handle,
                              const char* operation, const char* objectType)
    {
        if (!commandBuffer->IsAlive(handle))
            ThrowInvalidHandle(operation, objectType, handle.index, handle.generation);
    }

    void RequireDescriptorPool(RHICommandBuffer* commandBuffer, RHIDescriptorPoolHandle handle,
                               uint32_t poolId, const char* operation)
    {
        if (!commandBuffer->IsAlive(handle, poolId))
            ThrowInvalidHandle(operation, "RHIDescriptorPool", handle.index, handle.generation,
                               "Descriptor pool handle or pool ID is not live for this command buffer");
    }

    void RequireDescriptorSet(RHICommandBuffer* commandBuffer, RHIDescriptorPoolHandle handle,
                              uint32_t poolId, uint32_t setIndex, const char* operation)
    {
        if (!commandBuffer->IsAlive(handle, poolId, setIndex))
            ThrowInvalidHandle(operation, "RHIDescriptorPool", handle.index, handle.generation,
                               "Descriptor pool handle, pool ID, or set index is not live for this command buffer");
    }

    void RequireFinite(float value, const char* operation, const char* parameter)
    {
        if (!std::isfinite(value))
            ThrowInvalidParameter(operation, parameter, "Value must be finite");
    }

    void RequireBufferOffset(RHICommandBuffer* commandBuffer, RHIBufferHandle handle,
                             uint64_t offset, const char* operation, const char* objectType)
    {
        RequireCommandHandle(commandBuffer, handle, operation, objectType);
        if (!commandBuffer->IsBufferRangeValid(handle, offset, 1))
            ThrowInvalidParameter(operation, "offset", "Buffer offset must address a live byte");
    }
}

extern "C" {
RHI_DLL void RHICommandBuffer_Begin(RHICommandBuffer* cb, uint32_t frameIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
    cb->Begin(frameIndex, 0, nullptr);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_End(RHICommandBuffer* cb)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
    cb->End();
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_BeginRenderPass(RHICommandBuffer* cb,
                                              RHIRenderPassHandle renderPass, RHIFrameBufferHandle frameBuffer,
                                              int subpassContents, uint32_t clearValueCount,
                                              const RHIClearValue* pClearValues)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RHI_ABI_REQUIRE_ARRAY(pClearValues, clearValueCount, "RHIClearValue");
        RequireCommandHandle(cb, renderPass, __func__, "RHIRenderPass");
        RequireCommandHandle(cb, frameBuffer, __func__, "RHIFrameBuffer");
    RenderPassBeginDesc desc{};
    desc.renderPass = renderPass;
    desc.frameBuffer = frameBuffer;
    desc.subpassContents = ABI::RequireEnum<ESubpassContents>(
        subpassContents, __func__, "subpassContents");
    desc.clearValueCount = clearValueCount;
    desc.pClearValues = pClearValues;
    cb->BeginRenderPass(std::move(desc));
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_EndRenderPass(RHICommandBuffer* cb)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
    cb->EndRenderPass();
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_BindPipeline(RHICommandBuffer* cb, RHIPipelineHandle pipeline)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RequireCommandHandle(cb, pipeline, __func__, "RHIPipeline");
    cb->BindPipeline(pipeline);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_SetViewport(RHICommandBuffer* cb, float x, float y, float width, float height,
                                          float minDepth, float maxDepth)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RequireFinite(x, __func__, "x");
        RequireFinite(y, __func__, "y");
        RequireFinite(width, __func__, "width");
        RequireFinite(height, __func__, "height");
        RequireFinite(minDepth, __func__, "minDepth");
        RequireFinite(maxDepth, __func__, "maxDepth");
        if (width <= 0.0f || height == 0.0f)
            ThrowInvalidParameter(__func__, "width/height", "Viewport width must be positive and height non-zero");
        if (minDepth < 0.0f || maxDepth > 1.0f || minDepth > maxDepth)
            ThrowInvalidParameter(__func__, "minDepth/maxDepth",
                                  "Depth range must satisfy 0 <= minDepth <= maxDepth <= 1");
    cb->SetViewport(x, y, width, height, minDepth, maxDepth);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_SetScissor(RHICommandBuffer* cb, uint32_t offsetX, uint32_t offsetY, uint32_t width,
                                         uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Scissor extent must be non-zero");
    cb->SetScissor(offsetX, offsetY, width, height);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_BindVertexBuffers(RHICommandBuffer* cb, RHIBufferHandle buffer, uint64_t offset)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RequireBufferOffset(cb, buffer, offset, __func__, "RHIBuffer");
    cb->BindVertexBuffers(buffer, offset);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_BindIndexBuffer(RHICommandBuffer* cb, RHIBufferHandle buffer, uint64_t offset,
                                              int indexType)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RequireBufferOffset(cb, buffer, offset, __func__, "RHIBuffer");
    cb->BindIndexBuffer(buffer, offset,
                        ABI::RequireEnum<EIndexType>(indexType, __func__, "indexType"));
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_Draw(RHICommandBuffer* cb, uint32_t vertexCount, uint32_t instanceCount,
                                   uint32_t firstVertex, uint32_t firstInstance, uint32_t firstBinding)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
    cb->Draw(vertexCount, instanceCount, firstVertex, firstInstance, firstBinding);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_DrawIndexed(RHICommandBuffer* cb, uint32_t indexCount, uint32_t instanceCount,
                                          uint32_t firstIndex, int vertexOffset, uint32_t firstInstance,
                                          uint32_t firstBinding)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
    cb->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance, firstBinding);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_PipelineBarrier(RHICommandBuffer* cb, int srcStage, int dstStage, uint32_t dependency,
                                              const RHIMemoryBarrier* pMemoryBarriers, uint32_t memoryBarrierCount,
                                              const RHIImageMemoryBarrier* pImageMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount,
                                              const RHIBufferMemoryBarrier* pBufferMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RHI_ABI_REQUIRE_ARRAY(pMemoryBarriers, memoryBarrierCount, "RHIMemoryBarrier");
        RHI_ABI_REQUIRE_ARRAY(pImageMemoryBarriers, imageMemoryBarrierCount, "RHIImageMemoryBarrier");
        RHI_ABI_REQUIRE_ARRAY(pBufferMemoryBarriers, bufferMemoryBarrierCount, "RHIBufferMemoryBarrier");
        for (uint32_t i = 0; i < memoryBarrierCount; ++i)
        {
            const auto& barrier = pMemoryBarriers[i];
            ABI::RequireFlags<EAccessFlag>(static_cast<uint32_t>(barrier.srcAccessMask),
                                           ABI::AccessMask, true, __func__, "memoryBarriers[i].srcAccessMask");
            ABI::RequireFlags<EAccessFlag>(static_cast<uint32_t>(barrier.dstAccessMask),
                                           ABI::AccessMask, true, __func__, "memoryBarriers[i].dstAccessMask");
            ABI::RequireFlags<EPipelineStageFlag>(static_cast<uint32_t>(barrier.srcStageMask),
                                                  ABI::PipelineStageMask, true, __func__,
                                                  "memoryBarriers[i].srcStageMask");
            ABI::RequireFlags<EPipelineStageFlag>(static_cast<uint32_t>(barrier.dstStageMask),
                                                  ABI::PipelineStageMask, true, __func__,
                                                  "memoryBarriers[i].dstStageMask");
        }
        for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i)
        {
            const auto& barrier = pImageMemoryBarriers[i];
            RequireCommandHandle(cb, barrier.image, __func__, "RHIImage");
            ABI::RequireFlags<EAccessFlag>(static_cast<uint32_t>(barrier.srcAccess), ABI::AccessMask,
                                           true, __func__, "imageMemoryBarriers[i].srcAccess");
            ABI::RequireFlags<EAccessFlag>(static_cast<uint32_t>(barrier.dstAccess), ABI::AccessMask,
                                           true, __func__, "imageMemoryBarriers[i].dstAccess");
            ABI::RequireEnum<EImageLayout>(static_cast<int>(barrier.oldLayout), __func__,
                                           "imageMemoryBarriers[i].oldLayout");
            ABI::RequireEnum<EImageLayout>(static_cast<int>(barrier.newLayout), __func__,
                                           "imageMemoryBarriers[i].newLayout");
            ABI::RequireFlags<EImageAspectFlagBits>(
                static_cast<uint32_t>(barrier.subresourceRange.aspectMask), ABI::ImageAspectMask,
                false, __func__, "imageMemoryBarriers[i].subresourceRange.aspectMask");
            if (barrier.subresourceRange.levelCount == 0 || barrier.subresourceRange.layerCount == 0)
                ThrowInvalidParameter(__func__, "imageMemoryBarriers[i].subresourceRange",
                                      "Image barrier mip and layer counts must be non-zero");
            ABI::RequireFlags<EPipelineStageFlag>(static_cast<uint32_t>(barrier.srcStageMask),
                                                  ABI::PipelineStageMask, true, __func__,
                                                  "imageMemoryBarriers[i].srcStageMask");
            ABI::RequireFlags<EPipelineStageFlag>(static_cast<uint32_t>(barrier.dstStageMask),
                                                  ABI::PipelineStageMask, true, __func__,
                                                  "imageMemoryBarriers[i].dstStageMask");
        }
        for (uint32_t i = 0; i < bufferMemoryBarrierCount; ++i)
        {
            const auto& barrier = pBufferMemoryBarriers[i];
            RequireCommandHandle(cb, barrier.buffer, __func__, "RHIBuffer");
            ABI::RequireFlags<EAccessFlag>(static_cast<uint32_t>(barrier.srcAccessMask), ABI::AccessMask,
                                           true, __func__, "bufferMemoryBarriers[i].srcAccessMask");
            ABI::RequireFlags<EAccessFlag>(static_cast<uint32_t>(barrier.dstAccessMask), ABI::AccessMask,
                                           true, __func__, "bufferMemoryBarriers[i].dstAccessMask");
            ABI::RequireFlags<EPipelineStageFlag>(static_cast<uint32_t>(barrier.srcStageMask),
                                                  ABI::PipelineStageMask, true, __func__,
                                                  "bufferMemoryBarriers[i].srcStageMask");
            ABI::RequireFlags<EPipelineStageFlag>(static_cast<uint32_t>(barrier.dstStageMask),
                                                  ABI::PipelineStageMask, true, __func__,
                                                  "bufferMemoryBarriers[i].dstStageMask");
        }
    cb->PipelineBarrier(ABI::RequireFlags<EPipelineStageFlag>(
                            srcStage, ABI::PipelineStageMask, true, __func__, "srcStage"),
                        ABI::RequireFlags<EPipelineStageFlag>(
                            dstStage, ABI::PipelineStageMask, true, __func__, "dstStage"),
                        ABI::RequireMask(dependency, ABI::DependencyMask, true,
                                         __func__, "dependency"),
                        pMemoryBarriers, memoryBarrierCount,
                        pImageMemoryBarriers, imageMemoryBarrierCount,
                        pBufferMemoryBarriers, bufferMemoryBarrierCount);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_TransitionImageLayout(RHICommandBuffer* cb, RHIImageHandle image, int targetLayout)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RequireCommandHandle(cb, image, __func__, "RHIImage");
    cb->TransitionImageLayout(image,
                              ABI::RequireEnum<EImageLayout>(targetLayout, __func__, "targetLayout"));
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_TransitionImageLayoutExplicit(RHICommandBuffer* cb, RHIImageHandle image, int oldLayout,
                                                            int targetLayout)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RequireCommandHandle(cb, image, __func__, "RHIImage");
    cb->TransitionImageLayout(image,
                              ABI::RequireEnum<EImageLayout>(oldLayout, __func__, "oldLayout"),
                              ABI::RequireEnum<EImageLayout>(targetLayout, __func__, "targetLayout"));
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_TransitionImageLayoutWithQueueFamily(RHICommandBuffer* cb, RHIImageHandle image,
                                                                    int oldLayout, int targetLayout,
                                                                    uint32_t srcQueueFamilyIndex,
                                                                    uint32_t dstQueueFamilyIndex)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RequireCommandHandle(cb, image, __func__, "RHIImage");
    cb->TransitionImageLayout(image,
                              ABI::RequireEnum<EImageLayout>(oldLayout, __func__, "oldLayout"),
                              ABI::RequireEnum<EImageLayout>(targetLayout, __func__, "targetLayout"),
                              srcQueueFamilyIndex, dstQueueFamilyIndex);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_BindDescriptorSets(RHICommandBuffer* cb, int bindPoint, uint32_t firstSet,
                                                 RHIDescriptorPoolHandle poolHandle, uint32_t poolId)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RequireDescriptorPool(cb, poolHandle, poolId, __func__);
    cb->BindDescriptorSets(ABI::RequireEnum<EPipelineBindPoint>(bindPoint, __func__, "bindPoint"),
                           firstSet, poolHandle, poolId);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_PushConstants(RHICommandBuffer* cb, uint32_t offset, uint32_t size, const void* data,
                                            uint32_t stageFlags)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RHI_ABI_REQUIRE_ARRAY(data, size, "push-constant byte");
        if (!cb->IsPushConstantRangeValid(offset, size))
            ThrowInvalidParameter(__func__, "offset/size",
                                  "Push-constant offset and size must be aligned and within the device limit");
    cb->PushConstants(offset, size, data,
                      ABI::RequireMask(stageFlags, ABI::ShaderStageMask, false,
                                       __func__, "stageFlags"));
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_CopyBuffer(RHICommandBuffer* cb, RHIBufferHandle src, uint64_t srcOffset,
                                         RHIBufferHandle dst, uint64_t dstOffset, uint64_t size)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RequireCommandHandle(cb, src, __func__, "RHIBuffer");
        RequireCommandHandle(cb, dst, __func__, "RHIBuffer");
        if (!cb->IsBufferRangeValid(src, srcOffset, size) ||
            !cb->IsBufferRangeValid(dst, dstOffset, size))
            ThrowInvalidParameter(__func__, "offset/size", "Copy range must fit both buffers and be non-zero");
    cb->CopyBuffer(src, srcOffset, dst, dstOffset, size);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_CopyBufferToImage2DSubresource(RHICommandBuffer* cb, RHIBufferHandle src,
                                                              RHIImageHandle dst, int dstImageLayout,
                                                              uint64_t bufferOffset, uint32_t mipLevel,
                                                              uint32_t width, uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Copy extent must be non-zero");
        RequireBufferOffset(cb, src, bufferOffset, __func__, "RHIBuffer");
        RequireCommandHandle(cb, dst, __func__, "RHIImage");

    RHIBufferImageCopy region{};
    region.bufferOffset = bufferOffset;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = mipLevel;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.offsetX = 0;
    region.offsetY = 0;
    region.offsetZ = 0;
    region.width = width;
    region.height = height;
    region.depth = 1;

    ArisenEngine::Containers::Vector<RHIBufferImageCopy> regions;
    regions.emplace_back(region);
    cb->CopyBufferToImage(src, dst,
                          ABI::RequireEnum<EImageLayout>(dstImageLayout, __func__, "dstImageLayout"),
                          std::move(regions));

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_CopyBufferToImage2D(RHICommandBuffer* cb, RHIBufferHandle src, RHIImageHandle dst,
                                                   int dstImageLayout, uint64_t bufferOffset,
                                                   uint32_t width, uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Copy extent must be non-zero");
        RequireBufferOffset(cb, src, bufferOffset, __func__, "RHIBuffer");
        RequireCommandHandle(cb, dst, __func__, "RHIImage");

    RHIBufferImageCopy region{};
    region.bufferOffset = bufferOffset;
    region.imageSubresource.aspectMask = IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.width = width;
    region.height = height;
    region.depth = 1;

    ArisenEngine::Containers::Vector<RHIBufferImageCopy> regions;
    regions.emplace_back(region);
    cb->CopyBufferToImage(src, dst,
                          ABI::RequireEnum<EImageLayout>(dstImageLayout, __func__, "dstImageLayout"),
                          std::move(regions));

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_CopyImageToBuffer2D(RHICommandBuffer* cb, RHIImageHandle src,
                                                   int srcImageLayout, uint32_t srcImageAspect, RHIBufferHandle dst,
                                                   uint64_t bufferOffset, uint32_t width, uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Copy extent must be non-zero");
        RequireCommandHandle(cb, src, __func__, "RHIImage");
        RequireBufferOffset(cb, dst, bufferOffset, __func__, "RHIBuffer");

    RHIBufferImageCopy region{};
    region.bufferOffset = bufferOffset;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = ABI::RequireFlags<EImageAspectFlagBits>(
        srcImageAspect, ABI::ImageAspectMask, false, __func__, "srcImageAspect");
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.offsetX = 0;
    region.offsetY = 0;
    region.offsetZ = 0;
    region.width = width;
    region.height = height;
    region.depth = 1;

    ArisenEngine::Containers::Vector<RHIBufferImageCopy> regions;
    regions.emplace_back(region);
    cb->CopyImageToBuffer(src,
                          ABI::RequireEnum<EImageLayout>(srcImageLayout, __func__, "srcImageLayout"),
                          dst, std::move(regions));

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_BeginDebugLabel(RHICommandBuffer* cb, const char* label, const float color[4])
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RHI_ABI_REQUIRE_POINTER(label, "UTF-8 debug label");
        RHI_ABI_REQUIRE_POINTER(color, "float[4]");
        for (uint32_t i = 0; i < 4; ++i)
            RequireFinite(color[i], __func__, "color[i]");

    cb->BeginDebugLabel(label, color);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_EndDebugLabel(RHICommandBuffer* cb)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
    cb->EndDebugLabel();
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_InsertDebugMarker(RHICommandBuffer* cb, const char* label, const float color[4])
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RHI_ABI_REQUIRE_POINTER(label, "UTF-8 debug label");
        RHI_ABI_REQUIRE_POINTER(color, "float[4]");
        for (uint32_t i = 0; i < 4; ++i)
            RequireFinite(color[i], __func__, "color[i]");
    cb->InsertDebugMarker(label, color);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_Dispatch(RHICommandBuffer* cb, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
    cb->Dispatch(groupCountX, groupCountY, groupCountZ);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_BindDescriptorSet(RHICommandBuffer* cb, int bindPoint, uint32_t firstSet, RHIDescriptorPoolHandle poolHandle, uint32_t poolId, uint32_t setIdx)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        RequireDescriptorSet(cb, poolHandle, poolId, setIdx, __func__);
    // The native BindDescriptorSets signature varies, but RHIAsyncComputeTest.h uses:
    // cmd->BindDescriptorSet(RHI::PIPELINE_BIND_POINT_COMPUTE, 0, m_DescriptorPoolHandle, m_PoolId, setIdx);
    cb->BindDescriptorSet(ABI::RequireEnum<EPipelineBindPoint>(bindPoint, __func__, "bindPoint"),
                          firstSet, poolHandle, poolId, setIdx);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_BeginRendering(RHICommandBuffer* cb,
    uint32_t imgViewIdx, uint32_t imgViewGen,
    int imageLayout, int loadOp, int storeOp,
    float clearR, float clearG, float clearB, float clearA,
    int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Rendering extent must be non-zero");
        if (x < 0 || y < 0)
            ThrowInvalidParameter(__func__, "x/y", "Rendering offset must be non-negative");
        RHIImageViewHandle colorView{imgViewIdx, imgViewGen};
        RequireCommandHandle(cb, colorView, __func__, "RHIImageView");
        RequireFinite(clearR, __func__, "clearR");
        RequireFinite(clearG, __func__, "clearG");
        RequireFinite(clearB, __func__, "clearB");
        RequireFinite(clearA, __func__, "clearA");
    RHIRenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView = { imgViewIdx, imgViewGen };
    colorAttachment.imageLayout = ABI::RequireEnum<EImageLayout>(imageLayout, __func__, "imageLayout");
    colorAttachment.loadOp = ABI::RequireEnum<EAttachmentLoadOp>(loadOp, __func__, "loadOp");
    colorAttachment.storeOp = ABI::RequireEnum<EAttachmentStoreOp>(storeOp, __func__, "storeOp");
    colorAttachment.clearValue.float32[0] = clearR;
    colorAttachment.clearValue.float32[1] = clearG;
    colorAttachment.clearValue.float32[2] = clearB;
    colorAttachment.clearValue.float32[3] = clearA;

    RHIRenderingInfo info{};
    info.pColorAttachments = &colorAttachment;
    info.colorAttachmentCount = 1;
    info.pResolveAttachments = nullptr;
    info.pDepthAttachment = nullptr;
    info.pStencilAttachment = nullptr;
    info.layerCount = 1;
    info.RHIRenderArea.x = x;
    info.RHIRenderArea.y = y;
    info.RHIRenderArea.width = width;
    info.RHIRenderArea.height = height;

    cb->BeginRendering(info);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_BeginRenderingWithDepth(RHICommandBuffer* cb,
    uint32_t imgViewIdx, uint32_t imgViewGen,
    int imageLayout, int loadOp, int storeOp,
    float clearR, float clearG, float clearB, float clearA,
    uint32_t depthViewIdx, uint32_t depthViewGen,
    int depthLayout, int depthLoadOp, int depthStoreOp,
    float clearDepth, uint32_t clearStencil,
    int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Rendering extent must be non-zero");
        if (x < 0 || y < 0)
            ThrowInvalidParameter(__func__, "x/y", "Rendering offset must be non-negative");
        RHIImageViewHandle colorView{imgViewIdx, imgViewGen};
        RHIImageViewHandle depthView{depthViewIdx, depthViewGen};
        RequireCommandHandle(cb, colorView, __func__, "RHIImageView");
        RequireCommandHandle(cb, depthView, __func__, "RHIImageView");
        RequireFinite(clearR, __func__, "clearR");
        RequireFinite(clearG, __func__, "clearG");
        RequireFinite(clearB, __func__, "clearB");
        RequireFinite(clearA, __func__, "clearA");
        RequireFinite(clearDepth, __func__, "clearDepth");
        if (clearDepth < 0.0f || clearDepth > 1.0f)
            ThrowInvalidParameter(__func__, "clearDepth", "Depth clear value must be within [0, 1]");

    RHIRenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView = { imgViewIdx, imgViewGen };
    colorAttachment.imageLayout = ABI::RequireEnum<EImageLayout>(imageLayout, __func__, "imageLayout");
    colorAttachment.loadOp = ABI::RequireEnum<EAttachmentLoadOp>(loadOp, __func__, "loadOp");
    colorAttachment.storeOp = ABI::RequireEnum<EAttachmentStoreOp>(storeOp, __func__, "storeOp");
    colorAttachment.clearValue.float32[0] = clearR;
    colorAttachment.clearValue.float32[1] = clearG;
    colorAttachment.clearValue.float32[2] = clearB;
    colorAttachment.clearValue.float32[3] = clearA;

    RHIRenderingAttachmentInfo depthAttachment{};
    depthAttachment.imageView = { depthViewIdx, depthViewGen };
    depthAttachment.imageLayout = ABI::RequireEnum<EImageLayout>(depthLayout, __func__, "depthLayout");
    depthAttachment.loadOp = ABI::RequireEnum<EAttachmentLoadOp>(depthLoadOp, __func__, "depthLoadOp");
    depthAttachment.storeOp = ABI::RequireEnum<EAttachmentStoreOp>(depthStoreOp, __func__, "depthStoreOp");
    depthAttachment.clearValue.float32[0] = clearDepth;
    depthAttachment.clearValue.uint32[1] = clearStencil;

    RHIRenderingInfo info{};
    info.pColorAttachments = &colorAttachment;
    info.colorAttachmentCount = 1;
    info.pResolveAttachments = nullptr;
    info.pDepthAttachment = &depthAttachment;
    info.pStencilAttachment = nullptr;
    info.layerCount = 1;
    info.RHIRenderArea.x = x;
    info.RHIRenderArea.y = y;
    info.RHIRenderArea.width = width;
    info.RHIRenderArea.height = height;

    cb->BeginRendering(info);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_BeginRenderingDepthOnly(RHICommandBuffer* cb,
    uint32_t depthViewIdx, uint32_t depthViewGen,
    int depthLayout, int depthLoadOp, int depthStoreOp,
    float clearDepth, uint32_t clearStencil,
    int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");
        if (width == 0 || height == 0)
            ThrowInvalidParameter(__func__, "width/height", "Rendering extent must be non-zero");
        if (x < 0 || y < 0)
            ThrowInvalidParameter(__func__, "x/y", "Rendering offset must be non-negative");
        RHIImageViewHandle depthView{depthViewIdx, depthViewGen};
        RequireCommandHandle(cb, depthView, __func__, "RHIImageView");
        RequireFinite(clearDepth, __func__, "clearDepth");
        if (clearDepth < 0.0f || clearDepth > 1.0f)
            ThrowInvalidParameter(__func__, "clearDepth", "Depth clear value must be within [0, 1]");

    RHIRenderingAttachmentInfo depthAttachment{};
    depthAttachment.imageView = { depthViewIdx, depthViewGen };
    depthAttachment.imageLayout = ABI::RequireEnum<EImageLayout>(depthLayout, __func__, "depthLayout");
    depthAttachment.loadOp = ABI::RequireEnum<EAttachmentLoadOp>(depthLoadOp, __func__, "depthLoadOp");
    depthAttachment.storeOp = ABI::RequireEnum<EAttachmentStoreOp>(depthStoreOp, __func__, "depthStoreOp");
    depthAttachment.clearValue.float32[0] = clearDepth;
    depthAttachment.clearValue.uint32[1] = clearStencil;

    RHIRenderingInfo info{};
    info.pColorAttachments = nullptr;
    info.colorAttachmentCount = 0;
    info.pResolveAttachments = nullptr;
    info.pDepthAttachment = &depthAttachment;
    info.pStencilAttachment = nullptr;
    info.layerCount = 1;
    info.RHIRenderArea.x = x;
    info.RHIRenderArea.y = y;
    info.RHIRenderArea.width = width;
    info.RHIRenderArea.height = height;

    cb->BeginRendering(info);

    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHICommandBuffer_EndRendering(RHICommandBuffer* cb)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(cb, "RHICommandBuffer");

    cb->EndRendering();

    }
    RHI_ABI_CATCH_VOID()
}
} // extern "C"

ARISEN_BIND_END_BRIDGE()

