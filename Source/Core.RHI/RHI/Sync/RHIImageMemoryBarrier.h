#pragma once
#include "RHIImageSubresourceRange.h"
#include "../Handles/RHIHandle.h"
#include "../Enums/Pipeline/EAccessFlag.h"
#include "../Enums/Image/EImageLayout.h"
#include "../Enums/Pipeline/EPipelineStageFlag.h"

namespace ArisenEngine::RHI
{
    // Mirrors VK_QUEUE_FAMILY_IGNORED (~0u). In-family / no ownership transfer.
    static constexpr UInt32 RHI_QUEUE_FAMILY_IGNORED = 0xFFFFFFFFu;
    // Mirrors VK_QUEUE_FAMILY_EXTERNAL_KHR (~0u - 1). Used when releasing/acquiring
    // ownership of a shared image to/from an external API (e.g. D3D11 via Win32 NT handle).
    static constexpr UInt32 RHI_QUEUE_FAMILY_EXTERNAL = 0xFFFFFFFEu;

    typedef struct RHIImageMemoryBarrier
    {
        EAccessFlag srcAccess;
        EAccessFlag dstAccess;
        EImageLayout oldLayout;
        EImageLayout newLayout;
        UInt32 srcQueueFamilyIndex;
        UInt32 dstQueueFamilyIndex;
        RHIImageHandle image;
        RHIImageSubresourceRange subresourceRange;
        EPipelineStageFlag srcStageMask;
        EPipelineStageFlag dstStageMask;
    } RHIImageMemoryBarrier;
}
