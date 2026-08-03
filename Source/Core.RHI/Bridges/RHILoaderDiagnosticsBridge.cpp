#include "RHI/Diagnostics/RHIError.h"
// RHILoaderDiagnosticsBridge.cpp - diagnostics exports for RHILoader.
#include "RHI/Loader/RHILoader.h"
#include "RHI/Definitions/CoreRHICommon.h"
#include "Base/BindingMacros.h"

using namespace ArisenEngine::RHI;

ARISEN_BIND_PACKAGE("com.arisen.rhi.core")
ARISEN_BIND_BEGIN_BRIDGE("RHILoaderDiagnostics", "Core.RHI.dll", "Arisen.Native.RHI")

extern "C" {
RHI_DLL const char* RHILoader_GetLastErrorMessage()
{
    RHI_ABI_GUARD()
    {
    static thread_local ArisenEngine::String value;
    value = RHILoader::GetLastErrorMessage();
    return value.c_str();
    }
    RHI_ABI_CATCH_RETURN()
}
} // extern "C"

ARISEN_BIND_END_BRIDGE()
