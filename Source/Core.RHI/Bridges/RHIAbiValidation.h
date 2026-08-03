#pragma once

#include "RHI/Diagnostics/RHIError.h"
#include "RHI/Diagnostics/RHIAbiOwnerRegistry.h"
#include "RHI/Core/RHICommon.h"
#include "RHI/Enums/Attachment/EAttachmentLoadOp.h"
#include "RHI/Enums/Attachment/EAttachmentStoreOp.h"
#include "RHI/Enums/Buffer/EBufferCreateFlagBits.h"
#include "RHI/Enums/Buffer/EBufferUsage.h"
#include "RHI/Enums/Image/EFormat.h"
#include "RHI/Enums/Image/EImageAspectFlagBits.h"
#include "RHI/Enums/Image/EImageLayout.h"
#include "RHI/Enums/Image/EImageTiling.h"
#include "RHI/Enums/Image/EImageType.h"
#include "RHI/Enums/Image/EImageUsageFlagBits.h"
#include "RHI/Enums/Image/EImageViewType.h"
#include "RHI/Enums/Image/ESampleCountFlagBits.h"
#include "RHI/Enums/Memory/ERHIMemoryUsage.h"
#include "RHI/Enums/Memory/ESharingMode.h"
#include "RHI/Enums/Pipeline/EBlendFactor.h"
#include "RHI/Enums/Pipeline/EBlendOp.h"
#include "RHI/Enums/Pipeline/ECommandBufferLevel.h"
#include "RHI/Enums/Pipeline/ECullMode.h"
#include "RHI/Enums/Pipeline/EDescriptorType.h"
#include "RHI/Enums/Pipeline/EFrontFace.h"
#include "RHI/Enums/Pipeline/EIndexType.h"
#include "RHI/Enums/Pipeline/EPipelineBindPoint.h"
#include "RHI/Enums/Pipeline/EPipelineStageFlag.h"
#include "RHI/Enums/Pipeline/EPolygonMode.h"
#include "RHI/Enums/Pipeline/EPrimitiveTopology.h"
#include "RHI/Enums/Pipeline/EAccessFlag.h"
#include "RHI/Enums/Pipeline/EShaderStage.h"
#include "RHI/Enums/Pipeline/EVertexInputRate.h"
#include "RHI/Enums/Sampler/EBorderColor.h"
#include "RHI/Enums/Sampler/ECompareOp.h"
#include "RHI/Enums/Sampler/EFilter.h"
#include "RHI/Enums/Sampler/ESamplerAddressMode.h"
#include "RHI/Enums/Sampler/ESamplerMipmapMode.h"
#include "RHI/Enums/Subpass/EDependencyFlag.h"
#include "RHI/Enums/Subpass/ESubpassContents.h"
#include "RHI/Enums/Swapchain/EPresentMode.h"

#include <cstdint>
#include <type_traits>

namespace ArisenEngine::RHI::ABI
{
    namespace Detail
    {
        constexpr bool InRange(int64_t value, int64_t first, int64_t last) noexcept
        {
            return value >= first && value <= last;
        }

        template <typename... TValues>
        constexpr bool IsOneOf(int64_t value, TValues... candidates) noexcept
        {
            return ((value == static_cast<int64_t>(candidates)) || ...);
        }

        constexpr bool IsValid(ECommandBufferLevel*, int64_t value) noexcept
        {
            return InRange(value, COMMAND_BUFFER_LEVEL_PRIMARY, COMMAND_BUFFER_LEVEL_SECONDARY);
        }

        constexpr bool IsValid(ESubpassContents*, int64_t value) noexcept
        {
            return InRange(value, SUBPASS_CONTENTS_INLINE, SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        }

        constexpr bool IsValid(EIndexType*, int64_t value) noexcept
        {
            return IsOneOf(value, INDEX_TYPE_UINT16, INDEX_TYPE_UINT32,
                           INDEX_TYPE_NONE_KHR, INDEX_TYPE_UINT8_KHR);
        }

        constexpr bool IsValid(EImageLayout*, int64_t value) noexcept
        {
            return InRange(value, IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_PREINITIALIZED) ||
                IsOneOf(value,
                        IMAGE_LAYOUT_PRESENT_SRC_KHR,
                        IMAGE_LAYOUT_SHARED_PRESENT_KHR,
                        IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
                        IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
                        IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) ||
                InRange(value, IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR) ||
                InRange(value, IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
                        IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) ||
                InRange(value, IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL) ||
                InRange(value, IMAGE_LAYOUT_READ_ONLY_OPTIMAL, IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
#ifdef VK_ENABLE_BETA_EXTENSIONS
                || InRange(value, IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR)
#endif
                ;
        }

        constexpr bool IsValid(EPipelineBindPoint*, int64_t value) noexcept
        {
            return IsOneOf(value, PIPELINE_BIND_POINT_GRAPHICS, PIPELINE_BIND_POINT_COMPUTE,
                           PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                           PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI)
#ifdef VK_ENABLE_BETA_EXTENSIONS
                || value == PIPELINE_BIND_POINT_EXECUTION_GRAPH_AMDX
#endif
                ;
        }

        constexpr bool IsValid(EAttachmentLoadOp*, int64_t value) noexcept
        {
            return InRange(value, ATTACHMENT_LOAD_OP_LOAD, ATTACHMENT_LOAD_OP_DONT_CARE) ||
                value == ATTACHMENT_LOAD_OP_NONE_EXT;
        }

        constexpr bool IsValid(EAttachmentStoreOp*, int64_t value) noexcept
        {
            return InRange(value, ATTACHMENT_STORE_OP_STORE, ATTACHMENT_STORE_OP_DONT_CARE) ||
                value == ATTACHMENT_STORE_OP_NONE;
        }

        constexpr bool IsValid(EPresentMode*, int64_t value) noexcept
        {
            return InRange(value, PRESENT_MODE_IMMEDIATE, PRESENT_MODE_FIFO_RELAXED) ||
                InRange(value, PRESENT_MODE_SHARED_DEMAND_REFRESH,
                        PRESENT_MODE_SHARED_CONTINUOUS_REFRESH);
        }

        constexpr bool IsValid(EPrimitiveTopology*, int64_t value) noexcept
        {
            return InRange(value, PRIMITIVE_TOPOLOGY_POINT_LIST, PRIMITIVE_TOPOLOGY_PATCH_LIST);
        }

        constexpr bool IsValid(EVertexInputRate*, int64_t value) noexcept
        {
            return InRange(value, VERTEX_INPUT_RATE_VERTEX, VERTEX_INPUT_RATE_INSTANCE);
        }

        constexpr bool IsValid(EFormat*, int64_t value) noexcept
        {
            return InRange(value, FORMAT_UNDEFINED, FORMAT_ASTC_12x12_SRGB_BLOCK) ||
                InRange(value, FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG) ||
                InRange(value, FORMAT_ASTC_4x4_SFLOAT_BLOCK, FORMAT_ASTC_12x12_SFLOAT_BLOCK) ||
                InRange(value, FORMAT_G8B8G8R8_422_UNORM, FORMAT_G16_B16_R16_3PLANE_444_UNORM) ||
                InRange(value, FORMAT_G8_B8R8_2PLANE_444_UNORM, FORMAT_G16_B16R16_2PLANE_444_UNORM) ||
                InRange(value, FORMAT_A4R4G4B4_UNORM_PACK16, FORMAT_A4B4G4R4_UNORM_PACK16) ||
                value == FORMAT_R16G16_S10_5_NV ||
                InRange(value, FORMAT_A1B5G5R5_UNORM_PACK16, FORMAT_A8_UNORM);
        }

        constexpr bool IsValid(EPolygonMode*, int64_t value) noexcept
        {
            return InRange(value, EPOLYGON_MODE_FILL, EPOLYGON_MODE_POINT) ||
                value == EPOLYGON_MODE_FILL_RECTANGLE_NV;
        }

        constexpr bool IsValid(ECullModeFlagBits*, int64_t value) noexcept
        {
            return InRange(value, CULL_MODE_NONE, CULL_MODE_FRONT_AND_BACK);
        }

        constexpr bool IsValid(EFrontFace*, int64_t value) noexcept
        {
            return InRange(value, FRONT_FACE_COUNTER_CLOCKWISE, FRONT_FACE_CLOCKWISE);
        }

        constexpr bool IsValid(EBlendFactor*, int64_t value) noexcept
        {
            return InRange(value, BLEND_FACTOR_ZERO, BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA);
        }

        constexpr bool IsValid(EBlendOp*, int64_t value) noexcept
        {
            return InRange(value, BLEND_OP_ADD, BLEND_OP_MAX) ||
                InRange(value, BLEND_OP_ZERO_EXT, BLEND_OP_BLUE_EXT);
        }

        constexpr bool IsValid(ECompareOp*, int64_t value) noexcept
        {
            return InRange(value, COMPARE_OP_NEVER, COMPARE_OP_ALWAYS);
        }

        constexpr bool IsValid(ERHIObjectType*, int64_t value) noexcept
        {
            return InRange(value, static_cast<int64_t>(ERHIObjectType::Buffer),
                           static_cast<int64_t>(ERHIObjectType::Unknown));
        }

        constexpr bool IsValid(ESharingMode*, int64_t value) noexcept
        {
            return InRange(value, SHARING_MODE_EXCLUSIVE, SHARING_MODE_CONCURRENT);
        }

        constexpr bool IsValid(ERHIMemoryUsage*, int64_t value) noexcept
        {
            return InRange(value, static_cast<int64_t>(ERHIMemoryUsage::GpuOnly),
                           static_cast<int64_t>(ERHIMemoryUsage::Transient));
        }

        constexpr bool IsValid(EImageType*, int64_t value) noexcept
        {
            return InRange(value, IMAGE_TYPE_1D, IMAGE_TYPE_3D);
        }

        constexpr bool IsValid(EImageTiling*, int64_t value) noexcept
        {
            return IsOneOf(value, IMAGE_TILING_OPTIMAL, IMAGE_TILING_LINEAR,
                           IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT);
        }

        constexpr bool IsValid(ESampleCountFlagBits*, int64_t value) noexcept
        {
            return IsOneOf(value, SAMPLE_COUNT_1_BIT, SAMPLE_COUNT_2_BIT, SAMPLE_COUNT_4_BIT,
                           SAMPLE_COUNT_8_BIT, SAMPLE_COUNT_16_BIT, SAMPLE_COUNT_32_BIT,
                           SAMPLE_COUNT_64_BIT);
        }

        constexpr bool IsValid(EImageViewType*, int64_t value) noexcept
        {
            return InRange(value, IMAGE_VIEW_TYPE_1D, IMAGE_VIEW_TYPE_CUBE_ARRAY);
        }

        constexpr bool IsValid(EFilter*, int64_t value) noexcept
        {
            return IsOneOf(value, FILTER_NEAREST, FILTER_LINEAR, FILTER_CUBIC_EXT);
        }

        constexpr bool IsValid(ESamplerMipmapMode*, int64_t value) noexcept
        {
            return InRange(value, SAMPLER_MIPMAP_MODE_NEAREST, SAMPLER_MIPMAP_MODE_LINEAR);
        }

        constexpr bool IsValid(ESamplerAddressMode*, int64_t value) noexcept
        {
            return InRange(value, SAMPLER_ADDRESS_MODE_REPEAT,
                           SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE);
        }

        constexpr bool IsValid(EBorderColor*, int64_t value) noexcept
        {
            return InRange(value, BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
                           BORDER_COLOR_INT_OPAQUE_WHITE) ||
                InRange(value, BORDER_COLOR_FLOAT_CUSTOM_EXT, BORDER_COLOR_INT_CUSTOM_EXT);
        }

        constexpr bool IsValid(EShaderStage*, int64_t value) noexcept
        {
            return IsOneOf(value,
                           SHADER_STAGE_VERTEX_BIT,
                           SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                           SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                           SHADER_STAGE_GEOMETRY_BIT,
                           SHADER_STAGE_FRAGMENT_BIT,
                           SHADER_STAGE_COMPUTE_BIT,
                           SHADER_STAGE_TASK_BIT_EXT,
                           SHADER_STAGE_MESH_BIT_EXT,
                           SHADER_STAGE_RAYGEN_BIT,
                           SHADER_STAGE_ANY_HIT_BIT,
                           SHADER_STAGE_CLOSEST_HIT_BIT,
                           SHADER_STAGE_MISS_BIT,
                           SHADER_STAGE_INTERSECTION_BIT,
                           SHADER_STAGE_CALLABLE_BIT,
                           SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI,
                           SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI);
        }

        constexpr bool IsValid(EDescriptorType*, int64_t value) noexcept
        {
            return InRange(value, DESCRIPTOR_TYPE_SAMPLER, DESCRIPTOR_TYPE_INPUT_ATTACHMENT) ||
                IsOneOf(value,
                        DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
                        DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                        DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
                        DESCRIPTOR_TYPE_MUTABLE_EXT) ||
                InRange(value, DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM,
                        DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM);
        }
    }

    template <typename TEnum, typename TValue>
    TEnum RequireEnum(TValue value, const char* operation, const char* parameter)
    {
        static_assert(std::is_enum_v<TEnum>);
        static_assert(std::is_integral_v<TValue>);
        const int64_t numericValue = static_cast<int64_t>(value);
        if (!Detail::IsValid(static_cast<TEnum*>(nullptr), numericValue))
            ThrowInvalidEnumValue(operation, parameter, numericValue);
        return static_cast<TEnum>(value);
    }

    template <typename TEnum, typename TValue>
    TEnum RequireFlags(TValue value,
                       uint64_t allowedMask,
                       bool allowZero,
                       const char* operation,
                       const char* parameter)
    {
        static_assert(std::is_enum_v<TEnum>);
        static_assert(std::is_integral_v<TValue>);
        const uint64_t numericValue = static_cast<uint64_t>(value);
        const uint64_t invalidBits = numericValue & ~allowedMask;
        if (invalidBits != 0)
            ThrowInvalidFlagBits(operation, parameter, numericValue, invalidBits);
        if (!allowZero && numericValue == 0)
            ThrowInvalidParameter(operation, parameter, "Flag value must contain at least one bit");
        return static_cast<TEnum>(value);
    }

    template <typename TValue>
    TValue RequireMask(TValue value,
                       uint64_t allowedMask,
                       bool allowZero,
                       const char* operation,
                       const char* parameter)
    {
        static_assert(std::is_integral_v<TValue>);
        const uint64_t numericValue = static_cast<uint64_t>(value);
        const uint64_t invalidBits = numericValue & ~allowedMask;
        if (invalidBits != 0)
            ThrowInvalidFlagBits(operation, parameter, numericValue, invalidBits);
        if (!allowZero && numericValue == 0)
            ThrowInvalidParameter(operation, parameter, "Flag value must contain at least one bit");
        return value;
    }

    inline void RequireSubmittedTicket(uint64_t ticket,
                                       uint64_t latestTicket,
                                       const char* operation,
                                       const char* parameter = "ticket")
    {
        if (ticket > latestTicket)
            ThrowInvalidParameter(operation, parameter,
                                  "Ticket exceeds the latest submitted queue ticket");
    }

    inline constexpr uint64_t PipelineStageMask = 0x03FFFFFFULL;
    inline constexpr uint64_t AccessMask =
        ACCESS_INDIRECT_COMMAND_READ_BIT |
        ACCESS_INDEX_READ_BIT |
        ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
        ACCESS_UNIFORM_READ_BIT |
        ACCESS_INPUT_ATTACHMENT_READ_BIT |
        ACCESS_SHADER_READ_BIT |
        ACCESS_SHADER_WRITE_BIT |
        ACCESS_COLOR_ATTACHMENT_READ_BIT |
        ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
        ACCESS_TRANSFER_READ_BIT |
        ACCESS_TRANSFER_WRITE_BIT |
        ACCESS_HOST_READ_BIT |
        ACCESS_HOST_WRITE_BIT |
        ACCESS_MEMORY_READ_BIT |
        ACCESS_MEMORY_WRITE_BIT |
        ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT |
        ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT |
        ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT |
        ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT |
        ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT |
        ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR |
        ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR |
        ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT |
        ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR |
        ACCESS_COMMAND_PREPROCESS_READ_BIT_NV |
        ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV;
    inline constexpr uint64_t ImageAspectMask = 0x000007FFULL;
    inline constexpr uint64_t DependencyMask = 0x0000000FULL;
    inline constexpr uint64_t DynamicStateMask = (1ULL << 18U) - 1ULL;
    inline constexpr uint64_t ShaderStageMask = 0x00087FFFULL;

    inline constexpr uint64_t BufferCreateMask =
        BUFFER_CREATE_SPARSE_BINDING_BIT |
        BUFFER_CREATE_SPARSE_RESIDENCY_BIT |
        BUFFER_CREATE_SPARSE_ALIASED_BIT |
        BUFFER_CREATE_PROTECTED_BIT |
        BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT |
        BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT |
        BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;

    inline constexpr uint64_t BufferUsageMask =
        BUFFER_USAGE_TRANSFER_SRC_BIT |
        BUFFER_USAGE_TRANSFER_DST_BIT |
        BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
        BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
        BUFFER_USAGE_UNIFORM_BUFFER_BIT |
        BUFFER_USAGE_STORAGE_BUFFER_BIT |
        BUFFER_USAGE_INDEX_BUFFER_BIT |
        BUFFER_USAGE_VERTEX_BUFFER_BIT |
        BUFFER_USAGE_INDIRECT_BUFFER_BIT |
        BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT |
        BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
        BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT |
        BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT |
        BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR |
        BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR |
        BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR |
        BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR |
        BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
        BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
        BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
        BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT |
        BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT |
        BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT
#ifdef VK_ENABLE_BETA_EXTENSIONS
        | VK_BUFFER_USAGE_EXECUTION_GRAPH_SCRATCH_BIT_AMDX
#endif
        ;

    inline constexpr uint64_t ImageUsageMask =
        IMAGE_USAGE_TRANSFER_SRC_BIT |
        IMAGE_USAGE_TRANSFER_DST_BIT |
        IMAGE_USAGE_SAMPLED_BIT |
        IMAGE_USAGE_STORAGE_BIT |
        IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
        IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
        IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
        IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT |
        IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT |
        IMAGE_USAGE_VIDEO_DECODE_DST_BIT |
        IMAGE_USAGE_VIDEO_DECODE_SRC_BIT |
        IMAGE_USAGE_VIDEO_DECODE_DPB_BIT |
        IMAGE_USAGE_VIDEO_ENCODE_DST_BIT |
        IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT |
        IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT |
        IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI |
        IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT |
        IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM |
        IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM |
        IMAGE_USAGE_HOST_TRANSFER_BIT_EXT;

    static_assert(Detail::IsValid(static_cast<EFormat*>(nullptr), FORMAT_R8G8B8A8_UNORM));
    static_assert(Detail::IsValid(static_cast<EFormat*>(nullptr), FORMAT_A8_UNORM));
    static_assert(!Detail::IsValid(static_cast<EFormat*>(nullptr), 999999999));
    static_assert(Detail::IsValid(static_cast<EImageLayout*>(nullptr), IMAGE_LAYOUT_PRESENT_SRC_KHR));
    static_assert(!Detail::IsValid(static_cast<EImageLayout*>(nullptr), 9));
}

#define RHI_ABI_RESOLVE_OWNER(value, ownerType) \
    (value) = ::ArisenEngine::RHI::ResolveAbiOwner( \
        (value), ::ArisenEngine::RHI::ERHIAbiOwnerType::ownerType, __func__)
