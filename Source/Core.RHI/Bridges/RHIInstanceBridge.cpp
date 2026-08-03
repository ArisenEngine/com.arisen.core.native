#include "RHI/Diagnostics/RHIError.h"
#include "RHIAbiValidation.h"
// RHIInstanceBridge.cpp - extern "C" bridge for RHIInstance virtual methods
#include "RHI/Core/RHIInstance.h"
#include "RHI/Core/RHIDevice.h"
#include "RHI/Definitions/CoreRHICommon.h"
#include "Base/BindingMacros.h"

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHIInstance", "Core.RHI.dll", "Arisen.Native.RHI")

extern "C" {
RHI_DLL void RHIInstance_PickPhysicalDevice(RHIInstance* inst, int considerSurface)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    inst->PickPhysicalDevice(considerSurface != 0);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIInstance_InitLogicDevices(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    inst->InitLogicDevices();
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIInstance_CreateSurface(RHIInstance* inst, uint32_t windowId, uint32_t width, uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    if (width == 0 || height == 0)
        ThrowInvalidParameter(__func__, "width/height", "Surface resolution must be non-zero");
    inst->CreateSurface(windowId, width, height);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIInstance_DestroySurface(RHIInstance* inst, uint32_t windowId)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    auto* surface = &inst->GetSurface(windowId);
    inst->DestroySurface(windowId);
    InvalidateAbiOwnerByObject(surface, ERHIAbiOwnerType::Surface);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void RHIInstance_SetResolution(RHIInstance* inst, uint32_t windowId, uint32_t width, uint32_t height)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    if (width == 0 || height == 0)
        ThrowInvalidParameter(__func__, "width/height", "Surface resolution must be non-zero");
    inst->SetResolution(windowId, width, height);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void* RHIInstance_GetLogicalDevice(RHIInstance* inst, uint32_t windowId)
{
    RHI_ABI_GUARD()
    {
        const void* instanceToken = inst;
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    // windowId is now optional or 0 for main device
    auto* device = inst->GetLogicalDevice(windowId);
    if (!device)
        ThrowInvalidState(__func__, "RHIDevice", windowId, "Logical device is unavailable");
    return RegisterAbiOwner(device, ERHIAbiOwnerType::Device, instanceToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void* RHIInstance_GetSurface(RHIInstance* inst, uint32_t windowId)
{
    RHI_ABI_GUARD()
    {
        const void* instanceToken = inst;
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    return RegisterAbiOwner(&inst->GetSurface(windowId), ERHIAbiOwnerType::Surface, instanceToken);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIInstance_CreateLogicDevice(RHIInstance* inst, uint32_t windowId)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    // windowId 0 can be used for a default/main device
    inst->CreateLogicDevice(windowId);
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL int RHIInstance_IsPhysicalDeviceAvailable(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    return inst->IsPhysicalDeviceAvailable() ? 1 : 0;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL int RHIInstance_IsSurfacesAvailable(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    return inst->IsSurfacesAvailable() ? 1 : 0;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint32_t RHIInstance_GetMaxFramesInFlight(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    return inst->GetMaxFramesInFlight();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL int RHIInstance_IsEnableValidation(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    return inst->IsEnableValidation() ? 1 : 0;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL uint32_t RHIInstance_GetExternalIndex(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    return inst->GetExternalIndex();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL int RHIInstance_IsSupportLinearColorSpace(RHIInstance* inst, uint32_t windowId)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    return inst->IsSupportLinearColorSpace(windowId) ? 1 : 0;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL int RHIInstance_PresentModeSupported(RHIInstance* inst, uint32_t windowId, int mode)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    return inst->PresentModeSupported(
        windowId, ABI::RequireEnum<EPresentMode>(mode, __func__, "mode")) ? 1 : 0;
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHIInstance_SetCurrentPresentMode(RHIInstance* inst, uint32_t windowId, int mode)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    inst->SetCurrentPresentMode(
        windowId, ABI::RequireEnum<EPresentMode>(mode, __func__, "mode"));
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL int RHIInstance_GetSuitableSwapChainFormat(RHIInstance* inst, uint32_t windowId)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    return static_cast<int>(inst->GetSuitableSwapChainFormat(windowId));
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL int RHIInstance_GetSuitablePresentMode(RHIInstance* inst, uint32_t windowId)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    return static_cast<int>(inst->GetSuitablePresentMode(windowId));
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL const char* RHIInstance_GetAdapterName(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    static thread_local ArisenEngine::String value;
    value = inst != nullptr ? inst->GetAdapterName() : ArisenEngine::String();
    return value.c_str();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL const char* RHIInstance_GetAdapterTypeName(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    static thread_local ArisenEngine::String value;
    value = inst != nullptr ? inst->GetAdapterTypeName() : ArisenEngine::String();
    return value.c_str();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL const char* RHIInstance_GetAdapterDriverInfo(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    static thread_local ArisenEngine::String value;
    value = inst != nullptr ? inst->GetAdapterDriverInfo() : ArisenEngine::String();
    return value.c_str();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL const char* RHIInstance_GetEnabledInstanceExtensions(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    static thread_local ArisenEngine::String value;
    value = inst != nullptr ? inst->GetEnabledInstanceExtensions() : ArisenEngine::String();
    return value.c_str();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL const char* RHIInstance_GetEnabledDeviceExtensions(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    static thread_local ArisenEngine::String value;
    value = inst != nullptr ? inst->GetEnabledDeviceExtensions() : ArisenEngine::String();
    return value.c_str();
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL const char* RHIInstance_GetMissingDeviceExtensions(RHIInstance* inst)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(inst, "RHIInstance");
    static thread_local ArisenEngine::String value;
    value = inst != nullptr ? inst->GetMissingDeviceExtensions() : ArisenEngine::String();
    return value.c_str();
    }
    RHI_ABI_CATCH_RETURN()
}
} // extern "C"

ARISEN_BIND_END_BRIDGE()
