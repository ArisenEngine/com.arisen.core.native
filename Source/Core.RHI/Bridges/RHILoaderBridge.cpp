#include "RHI/Diagnostics/RHIError.h"
#include "RHI/Diagnostics/RHIAbiOwnerRegistry.h"
// RHILoaderBridge.cpp �?extern "C" bridge for RHILoader static methods
// Auto-included by collect_sources �?compiled into Core.RHI.dll
#include "RHI/Loader/RHILoader.h"
#include "RHI/Core/RHIInstance.h"
#include "RHI/Definitions/CoreRHICommon.h"
#include "Base/BindingMacros.h"
#include <stdexcept>

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHILoader", "Core.RHI.dll", "Arisen.Native.RHI")

extern "C" {
RHI_DLL void RHILoader_SetCurrentGraphicsAPI(int apiType)
{
    RHI_ABI_GUARD()
    {
    if (apiType < static_cast<int>(GraphicsAPI::None) || apiType > static_cast<int>(GraphicsAPI::Metal))
        ThrowInvalidParameter(__func__, "apiType", "Graphics API is outside the supported range");
    RHILoader::SetCurrentGraphicsAPI(static_cast<GraphicsAPI>(apiType));
    }
    RHI_ABI_CATCH_VOID()
}

RHI_DLL void* RHILoader_CreateInstance(
    const char* name, const char* engineName, int validationLayer,
    uint32_t variant, uint32_t major_ver, uint32_t minor_ver, uint32_t patch,
    uint32_t appMajor, uint32_t appMinor, uint32_t appPatch,
    uint32_t engineMajor, uint32_t engineMinor, uint32_t enginePatch,
    uint32_t maxFramesInFlight)
{
    RHI_ABI_GUARD()
    {
        RHI_ABI_REQUIRE_POINTER(name, "char");
        RHI_ABI_REQUIRE_POINTER(engineName, "char");
        if (maxFramesInFlight == 0)
            ThrowInvalidParameter(__func__, "maxFramesInFlight", "Frames in flight must be greater than zero");
    RHIInstanceInfo info{};
    info.name = name;
    info.engineName = engineName;
    info.validationLayer = validationLayer != 0;
    info.variant = variant;
    info.major = major_ver;
    info.minor = minor_ver;
    info.patch = patch;
    info.appMajor = appMajor;
    info.appMinor = appMinor;
    info.appPatch = appPatch;
    info.engineMajor = engineMajor;
    info.engineMinor = engineMinor;
    info.enginePatch = enginePatch;
    info.maxFramesInFlight = maxFramesInFlight;
    auto* instance = RHILoader::CreateInstance(std::move(info));
    if (!instance)
    {
        auto message = RHILoader::GetLastErrorMessage();
        SetLastErrorDetailed(EErrorCode::InitializationFailed, __func__, 0, "RHIInstance", 0,
                             UINT32_MAX, 0, message.c_str());
        throw std::runtime_error(message.c_str());
    }
    return RegisterAbiOwner(instance, ERHIAbiOwnerType::Instance);
    }
    RHI_ABI_CATCH_RETURN()
}

RHI_DLL void RHILoader_Dispose()
{
    RHI_ABI_GUARD()
    {
    RHILoader::Dispose();
    }
    RHI_ABI_CATCH_VOID()
}
} // extern "C"

ARISEN_BIND_END_BRIDGE()

