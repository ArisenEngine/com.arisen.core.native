#pragma once
#include "RHIPipeline.h"
#include "../Handles/RHIHandle.h"
#include "RHIShaderProgram.h"
#include "Base/FoundationMinimal.h"
#include "RHI/Enums/Pipeline/EDynamicState.h"
#include "RHI/Enums/Pipeline/EPrimitiveTopology.h"
#include "RHI/Definitions/CoreRHICommon.h"

namespace ArisenEngine::RHI
{
    class RHIDevice;
    class RHIPipelineState;

    // TODO
    struct SpecializationInfoDesc
    {
    };

    struct PipelineShaderStageDesc
    {
        UInt32 flag;
        EShaderStage stage;
        RHIShaderProgram& program;
        std::optional<SpecializationInfoDesc> specializationInfo;
    };

    class RHI_DLL RHIPipelineCache
    {
    public:
        NO_COPY_NO_MOVE_NO_DEFAULT(RHIPipelineCache)
        RHIPipelineCache(RHIDevice* device, UInt32 maxFramesInFlight);
        virtual ~RHIPipelineCache() noexcept = default;
        RHIDevice* GetOwnerDevice() const { return m_Device; }
        virtual RHIPipelineHandle GetGraphicsPipeline(RHIPipelineState* pso) = 0;
        virtual RHIPipelineHandle GetComputePipeline(RHIPipelineState* pso) = 0;
        virtual RHIPipelineHandle GetRayTracingPipeline(RHIPipelineState* pso) = 0;
        virtual bool ReleasePipeline(RHIPipelineHandle handle) = 0;
        virtual bool IsAlive(RHIPipelineHandle handle) const = 0;

        virtual std::unique_ptr<RHIPipelineState> GetPipelineState() = 0;

    protected:
        RHIDevice* m_Device;
        UInt32 m_MaxFramesInFlight;
    };
}
