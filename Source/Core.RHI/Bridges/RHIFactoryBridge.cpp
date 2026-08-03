#include "RHI/Diagnostics/RHIError.h"
#include "RHIAbiValidation.h"
// RHIFactoryBridge.cpp �?extern "C" bridge for RHIFactory pure virtual methods
#include "RHI/Core/RHIFactory.h"
#include "RHI/Core/RHIDevice.h"
#include "RHI/Definitions/CoreRHICommon.h"
#include "RHI/Handles/RHIHandle.h"
#include "RHI/Descriptors/RHIResourceDescriptors.h"
#include "RHI/Samplers/RHISampler.h"
#include "RHI/Core/RHICommon.h"
#include "Base/BindingMacros.h"
#include <cmath>

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHIFactory", "Core.RHI.dll", "Arisen.Native.RHI")

// Helper: construct a typed handle from (index, generation) pair
template <typename T>
static inline RHIHandle<T> MakeHandle(uint32_t index, uint32_t generation)
{
    RHIHandle<T> h;
    h.index = index;
    h.generation = generation;
    return h;
}

template <typename THandle>
static void RequireFactoryHandle(RHIFactory* factory, THandle handle, const char* operation, const char* objectType)
{
    if (!factory->IsAlive(handle))
        ThrowInvalidHandle(operation, objectType, handle.index, handle.generation);
}

template <typename THandle>
static void RequireCreatedHandle(THandle handle, const char* operation, const char* objectType)
{
    if (!handle.IsValid())
        ThrowInvalidState(operation, objectType, 0, "Backend returned an invalid resource handle");
}

static void RequireReleaseCommitted(bool committed,
                                    const char* operation,
                                    const char* objectType,
                                    uint32_t index,
                                    uint32_t generation)
{
    if (!committed)
    {
        ThrowInvalidState(operation, objectType, 0,
                          "Resource release did not commit", index, generation);
    }
}

extern "C" {
// ============================================================================
// Buffer
// ============================================================================

RHI_DLL void RHIFactory_CreateBuffer(RHIFactory* f,
                                     uint32_t createFlagBits, uint64_t size, uint32_t usage,
                                     int sharingMode, uint32_t queueFamilyIndexCount,
                                     int memoryUsage, const char* name,
                                     uint32_t* outIndex, uint32_t* outGeneration)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
        RHI_ABI_REQUIRE_POINTER(outIndex, "uint32_t");
        RHI_ABI_REQUIRE_POINTER(outGeneration, "uint32_t");
        if (size == 0)
            ThrowInvalidParameter(__func__, "size", "Buffer size must be greater than zero");
        if (queueFamilyIndexCount != 0)
            ThrowInvalidParameter(__func__, "queueFamilyIndexCount",
                                  "The bridge does not accept queue-family indices");
    RHIBufferDescriptor desc{};
    desc.createFlagBits = ABI::RequireMask(
        createFlagBits, ABI::BufferCreateMask, true, __func__, "createFlagBits");
    desc.size = size;
    desc.usage = ABI::RequireMask(usage, ABI::BufferUsageMask, false, __func__, "usage");
    desc.sharingMode = ABI::RequireEnum<ESharingMode>(sharingMode, __func__, "sharingMode");
    desc.queueFamilyIndexCount = queueFamilyIndexCount;
    desc.pQueueFamilyIndices = nullptr;
    desc.memoryUsage = ABI::RequireEnum<ERHIMemoryUsage>(memoryUsage, __func__, "memoryUsage");
    auto handle = f->CreateBuffer(std::move(desc), name ? name : "Anonymous");
    RequireCreatedHandle(handle, __func__, "RHIBuffer");
    *outIndex = handle.index;
    *outGeneration = handle.generation;
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_ReleaseBuffer(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIBufferTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIBuffer");
    RequireReleaseCommitted(
        f->ReleaseBuffer(handle), __func__, "RHIBuffer", index, generation);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_BufferMemoryCopy(RHIFactory* f, uint32_t index, uint32_t generation,
                                         const void* src, uint64_t size, uint64_t offset)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    RHI_ABI_REQUIRE_ARRAY(src, size, "byte");
    if (size == 0)
        ThrowInvalidParameter(__func__, "size", "Copy size must be greater than zero");
    auto handle = MakeHandle<RHIBufferTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIBuffer");
    const uint64_t bufferSize = f->GetBufferSize(handle);
    if (offset > bufferSize || size > bufferSize - offset)
        ThrowInvalidParameter(__func__, "size/offset", "Copy range exceeds the destination buffer");
    f->BufferMemoryCopy(handle, src, size, offset);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void* RHIFactory_MapBuffer(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIBufferTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIBuffer");
    return f->MapBuffer(handle);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIFactory_UnmapBuffer(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIBufferTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIBuffer");
    f->UnmapBuffer(handle);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL uint64_t RHIFactory_GetBufferSize(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIBufferTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIBuffer");
    return f->GetBufferSize(handle);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint64_t RHIFactory_GetBufferDeviceAddress(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIBufferTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIBuffer");
    return f->GetBufferDeviceAddress(handle);
    }
    RHI_ABI_CATCH_RETURN()
}

// ============================================================================
// Image
// ============================================================================

RHI_DLL void RHIFactory_CreateImage(RHIFactory* f,
                                    int imageType, uint32_t width, uint32_t height, uint32_t depth,
                                    uint32_t mipLevels, uint32_t arrayLayers, int format, int tiling,
                                    int imageLayout, uint32_t usage, int sampleCount, int sharingMode,
                                    int memoryUsage, const char* name,
                                    uint32_t* outIndex, uint32_t* outGeneration)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
        RHI_ABI_REQUIRE_POINTER(outIndex, "uint32_t");
        RHI_ABI_REQUIRE_POINTER(outGeneration, "uint32_t");
        if (width == 0 || height == 0 || depth == 0 || mipLevels == 0 || arrayLayers == 0)
            ThrowInvalidParameter(__func__, "dimensions", "Image dimensions, mip levels, and layers must be non-zero");
    RHIImageDescriptor desc{};
    desc.imageType = ABI::RequireEnum<EImageType>(imageType, __func__, "imageType");
    desc.width = width;
    desc.height = height;
    desc.depth = depth;
    desc.mipLevels = mipLevels;
    desc.arrayLayers = arrayLayers;
    desc.format = ABI::RequireEnum<EFormat>(format, __func__, "format");
    desc.tiling = ABI::RequireEnum<EImageTiling>(tiling, __func__, "tiling");
    desc.imageLayout = ABI::RequireEnum<EImageLayout>(imageLayout, __func__, "imageLayout");
    desc.usage = ABI::RequireMask(usage, ABI::ImageUsageMask, false, __func__, "usage");
    desc.sampleCount = ABI::RequireEnum<ESampleCountFlagBits>(sampleCount, __func__, "sampleCount");
    desc.sharingMode = ABI::RequireEnum<ESharingMode>(sharingMode, __func__, "sharingMode");
    desc.queueFamilyIndexCount = 0;
    desc.pQueueFamilyIndices = nullptr;
    desc.memoryUsage = ABI::RequireEnum<ERHIMemoryUsage>(memoryUsage, __func__, "memoryUsage");
    auto handle = f->CreateImage(std::move(desc), name ? name : "Anonymous");
    RequireCreatedHandle(handle, __func__, "RHIImage");
    *outIndex = handle.index;
    *outGeneration = handle.generation;
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_ReleaseImage(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIImageTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIImage");
    RequireReleaseCommitted(
        f->ReleaseImage(handle), __func__, "RHIImage", index, generation);
    }
    RHI_ABI_CATCH_VOID()
}

// ============================================================================
// ImageView
// ============================================================================

RHI_DLL void RHIFactory_CreateImageView(RHIFactory* f,
                                        uint32_t imageIndex, uint32_t imageGeneration,
                                        int viewType, int format, uint32_t aspectMask,
                                        uint32_t baseMipLevel, uint32_t levelCount,
                                        uint32_t baseArrayLayer, uint32_t layerCount,
                                        uint32_t* outIndex, uint32_t* outGeneration)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
        RHI_ABI_REQUIRE_POINTER(outIndex, "uint32_t");
        RHI_ABI_REQUIRE_POINTER(outGeneration, "uint32_t");
        if (levelCount == 0 || layerCount == 0)
            ThrowInvalidParameter(__func__, "levelCount/layerCount",
                                  "Image-view mip and layer counts must be non-zero");
        auto imageHandle = MakeHandle<RHIImageTag>(imageIndex, imageGeneration);
        RequireFactoryHandle(f, imageHandle, __func__, "RHIImage");
    RHIImageViewDesc desc{};
    desc.viewType = ABI::RequireEnum<EImageViewType>(viewType, __func__, "viewType");
    desc.format = ABI::RequireEnum<EFormat>(format, __func__, "format");
    desc.aspectMask = ABI::RequireMask(
        aspectMask, ABI::ImageAspectMask, false, __func__, "aspectMask");
    desc.baseMipLevel = baseMipLevel;
    desc.levelCount = levelCount;
    desc.baseArrayLayer = baseArrayLayer;
    desc.layerCount = layerCount;
    auto handle = f->CreateImageView(imageHandle, std::move(desc));
    RequireCreatedHandle(handle, __func__, "RHIImageView");
    *outIndex = handle.index;
    *outGeneration = handle.generation;
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_ReleaseImageView(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIImageViewTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIImageView");
    RequireReleaseCommitted(
        f->ReleaseImageView(handle), __func__, "RHIImageView", index, generation);
    }
    RHI_ABI_CATCH_VOID()
}

// ============================================================================
// Sampler
// ============================================================================

RHI_DLL void RHIFactory_CreateSampler(RHIFactory* f,
                                      int magFilter, int minFilter, int mipmapMode,
                                      int addressModeU, int addressModeV, int addressModeW,
                                      float mipLodBias, int anisotropyEnable, float maxAnisotropy,
                                      int compareEnable, int compareOp, float minLod, float maxLod, int borderColor,
                                      uint32_t* outIndex, uint32_t* outGeneration)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
        RHI_ABI_REQUIRE_POINTER(outIndex, "uint32_t");
        RHI_ABI_REQUIRE_POINTER(outGeneration, "uint32_t");
        if (!std::isfinite(mipLodBias) || !std::isfinite(maxAnisotropy) ||
            !std::isfinite(minLod) || !std::isfinite(maxLod))
            ThrowInvalidParameter(__func__, "sampler floats", "Sampler values must be finite");
        if (minLod > maxLod)
            ThrowInvalidParameter(__func__, "minLod/maxLod", "Sampler minLod must not exceed maxLod");
        if (anisotropyEnable != 0 && maxAnisotropy <= 0.0f)
            ThrowInvalidParameter(__func__, "maxAnisotropy",
                                  "Enabled anisotropy requires a positive maximum");
    RHISamplerDesc desc{};
    desc.magFilter = ABI::RequireEnum<EFilter>(magFilter, __func__, "magFilter");
    desc.minFilter = ABI::RequireEnum<EFilter>(minFilter, __func__, "minFilter");
    desc.mipmapMode = ABI::RequireEnum<ESamplerMipmapMode>(mipmapMode, __func__, "mipmapMode");
    desc.addressModeU = ABI::RequireEnum<ESamplerAddressMode>(addressModeU, __func__, "addressModeU");
    desc.addressModeV = ABI::RequireEnum<ESamplerAddressMode>(addressModeV, __func__, "addressModeV");
    desc.addressModeW = ABI::RequireEnum<ESamplerAddressMode>(addressModeW, __func__, "addressModeW");
    desc.mipLodBias = mipLodBias;
    desc.anisotropyEnable = anisotropyEnable != 0;
    desc.maxAnisotropy = maxAnisotropy;
    desc.compareEnable = compareEnable != 0;
    desc.compareOp = ABI::RequireEnum<ECompareOp>(compareOp, __func__, "compareOp");
    desc.minLod = minLod;
    desc.maxLod = maxLod;
    desc.borderColor = ABI::RequireEnum<EBorderColor>(borderColor, __func__, "borderColor");
    desc.unnormalizedCoordinates = false;

    auto handle = f->CreateSampler(std::move(desc));
    RequireCreatedHandle(handle, __func__, "RHISampler");
    *outIndex = handle.index;
    *outGeneration = handle.generation;
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_ReleaseSampler(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHISamplerTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHISampler");
    RequireReleaseCommitted(
        f->ReleaseSampler(handle), __func__, "RHISampler", index, generation);
    }
    RHI_ABI_CATCH_VOID()
}

// ============================================================================
// Sync Primitives
// ============================================================================

RHI_DLL void RHIFactory_CreateSemaphore(RHIFactory* f, uint32_t* outIndex, uint32_t* outGeneration)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
        RHI_ABI_REQUIRE_POINTER(outIndex, "uint32_t");
        RHI_ABI_REQUIRE_POINTER(outGeneration, "uint32_t");
    auto handle = f->CreateSemaphore();
    RequireCreatedHandle(handle, __func__, "RHISemaphore");
    *outIndex = handle.index;
    *outGeneration = handle.generation;
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_ReleaseSemaphore(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHISemaphoreTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHISemaphore");
    RequireReleaseCommitted(
        f->ReleaseSemaphore(handle), __func__, "RHISemaphore", index, generation);
    }
    RHI_ABI_CATCH_VOID()
}



// ============================================================================
// RenderPass / FrameBuffer
// ============================================================================

RHI_DLL void RHIFactory_CreateRenderPass(RHIFactory* f, uint32_t* outIndex, uint32_t* outGeneration)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
        RHI_ABI_REQUIRE_POINTER(outIndex, "uint32_t");
        RHI_ABI_REQUIRE_POINTER(outGeneration, "uint32_t");
    auto handle = f->CreateRenderPass();
    RequireCreatedHandle(handle, __func__, "RHIRenderPass");
    *outIndex = handle.index;
    *outGeneration = handle.generation;
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_ReleaseRenderPass(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIRenderPassTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIRenderPass");
    RequireReleaseCommitted(
        f->ReleaseRenderPass(handle), __func__, "RHIRenderPass", index, generation);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_CreateFrameBuffer(RHIFactory* f, uint32_t* outIndex, uint32_t* outGeneration)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
        RHI_ABI_REQUIRE_POINTER(outIndex, "uint32_t");
        RHI_ABI_REQUIRE_POINTER(outGeneration, "uint32_t");
    auto handle = f->CreateFrameBuffer();
    RequireCreatedHandle(handle, __func__, "RHIFrameBuffer");
    *outIndex = handle.index;
    *outGeneration = handle.generation;
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_ReleaseFrameBuffer(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIFrameBufferTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIFrameBuffer");
    RequireReleaseCommitted(
        f->ReleaseFrameBuffer(handle), __func__, "RHIFrameBuffer", index, generation);
    }
    RHI_ABI_CATCH_VOID()
}

// ============================================================================
// CommandBufferPool
// ============================================================================

RHI_DLL void RHIFactory_CreateCommandBufferPool(RHIFactory* f, int queueType,
                                                uint32_t* outIndex, uint32_t* outGeneration)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
        RHI_ABI_REQUIRE_POINTER(outIndex, "uint32_t");
        RHI_ABI_REQUIRE_POINTER(outGeneration, "uint32_t");
        if (queueType < static_cast<int>(RHIQueueType::Graphics) ||
            queueType > static_cast<int>(RHIQueueType::Present))
            ThrowInvalidParameter(__func__, "queueType", "Queue type is outside the supported range");
    auto handle = f->CreateCommandBufferPool(static_cast<RHIQueueType>(queueType));
    RequireCreatedHandle(handle, __func__, "RHICommandBufferPool");
    *outIndex = handle.index;
    *outGeneration = handle.generation;
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_ReleaseCommandBufferPool(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        const void* factoryToken = f;
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHICommandBufferPoolTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHICommandBufferPool");
    auto* device = ResolveAbiOwnerParent<RHIDevice>(
        factoryToken, ERHIAbiOwnerType::Device, __func__);
    auto* pool = device->GetCommandBufferPool(handle);
    if (!pool)
        ThrowInvalidHandle(__func__, "RHICommandBufferPool", index, generation);
    RequireReleaseCommitted(
        f->ReleaseCommandBufferPool(handle), __func__,
        "RHICommandBufferPool", index, generation);
    InvalidateAbiOwnerByObject(pool, ERHIAbiOwnerType::CommandBufferPool);
    }
    RHI_ABI_CATCH_VOID()
}

// ============================================================================
// ImageView Queries
// ============================================================================

RHI_DLL int RHIFactory_GetImageViewFormat(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIImageViewTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIImageView");
    return static_cast<int>(f->GetImageViewFormat(handle));
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint32_t RHIFactory_GetImageViewWidth(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIImageViewTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIImageView");
    return f->GetImageViewWidth(handle);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint32_t RHIFactory_GetImageViewHeight(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIImageViewTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIImageView");
    return f->GetImageViewHeight(handle);
    }
    RHI_ABI_CATCH_RETURN()
}

// ============================================================================
// Shader Program
// ============================================================================

RHI_DLL void RHIFactory_CreateGPUProgram(RHIFactory* f, uint32_t* outIndex, uint32_t* outGeneration)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
        RHI_ABI_REQUIRE_POINTER(outIndex, "uint32_t");
        RHI_ABI_REQUIRE_POINTER(outGeneration, "uint32_t");
    auto handle = f->CreateGPUProgram();
    RequireCreatedHandle(handle, __func__, "RHIShaderProgram");
    *outIndex = handle.index;
    *outGeneration = handle.generation;
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIFactory_ReleaseGPUProgram(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIShaderProgramTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIShaderProgram");
    RequireReleaseCommitted(
        f->ReleaseGPUProgram(handle), __func__, "RHIShaderProgram", index, generation);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL int RHIFactory_AttachProgramByteCode(RHIFactory* f, uint32_t index, uint32_t generation,
                                             int stage, const void* code, uint64_t size, const char* entryPoint)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    if (size == 0 || size % sizeof(uint32_t) != 0)
        ThrowInvalidParameter(__func__, "size",
                              "Shader bytecode size must be a non-zero multiple of four bytes");
    RHI_ABI_REQUIRE_ARRAY(code, size, "shader bytecode");
    auto handle = MakeHandle<RHIShaderProgramTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIShaderProgram");
    RHIShaderProgramDesc desc{};
    desc.stage = ABI::RequireEnum<EShaderStage>(stage, __func__, "stage");
    desc.byteCode = const_cast<void*>(code);
    desc.codeSize = size;
    desc.entry = entryPoint ? entryPoint : "main";
    return f->AttachProgramByteCode(handle, std::move(desc)) ? 1 : 0;
    }
    RHI_ABI_CATCH_RETURN()
}

// ============================================================================
// Bindless
// ============================================================================

RHI_DLL uint32_t RHIFactory_RegisterBindlessResourceImage(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIImageViewTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIImageView");
    const uint32_t bindlessIndex = f->RegisterBindlessResource(handle);
    if (bindlessIndex == UINT32_MAX)
        ThrowInvalidState(__func__, "RHIBindlessImageTable", 0, "Bindless image table is full");
    return bindlessIndex;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint32_t RHIFactory_RegisterBindlessResourceBuffer(RHIFactory* f, uint32_t index, uint32_t generation)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    auto handle = MakeHandle<RHIBufferTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIBuffer");
    const uint32_t bindlessIndex = f->RegisterBindlessResource(handle);
    if (bindlessIndex == UINT32_MAX)
        ThrowInvalidState(__func__, "RHIBindlessBufferTable", 0, "Bindless buffer table is full");
    return bindlessIndex;
    }
    RHI_ABI_CATCH_RETURN()
}

// ============================================================================
// Async Transfer API
// ============================================================================

RHI_DLL uint64_t RHIFactory_BufferMemoryCopyAsync(RHIFactory* f, uint32_t index, uint32_t generation,
                                                   const void* src, uint64_t size, uint64_t offset)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    RHI_ABI_REQUIRE_ARRAY(src, size, "byte");
    if (size == 0)
        ThrowInvalidParameter(__func__, "size", "Copy size must be greater than zero");
    auto handle = MakeHandle<RHIBufferTag>(index, generation);
    RequireFactoryHandle(f, handle, __func__, "RHIBuffer");
    const uint64_t bufferSize = f->GetBufferSize(handle);
    if (offset > bufferSize || size > bufferSize - offset)
        ThrowInvalidParameter(__func__, "size/offset", "Copy range exceeds the destination buffer");
    return f->BufferMemoryCopyAsync(handle, src, size, offset);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint64_t RHIFactory_FlushTransfers(RHIFactory* f)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    return f->FlushTransfers();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIFactory_UpdateTransfers(RHIFactory* f)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(f, "RHIFactory");
    f->UpdateTransfers();
    }
    RHI_ABI_CATCH_VOID()
}

} // extern "C"

ARISEN_BIND_END_BRIDGE()

