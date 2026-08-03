#pragma once

#include "../Definitions/CoreRHICommon.h"
#include "RHIAbiOwnerRegistry.h"
#include <cstdint>

namespace ArisenEngine::RHI
{
    enum class EErrorCode : int32_t
    {
        None = 0,
        OutOfMemory = 1,
        InvalidHandle = 2,
        DeviceLost = 3,
        ValidationFailed = 4,
        InitializationFailed = 5,
        ShaderCompilationFailed = 6,
        PipelineCreationFailed = 7,
        InvalidParameter = 8,
        UnsupportedFeature = 9,
        InvalidState = 10,
        BackendFailure = 11,
        NativeException = 12,
        Unknown = 99
    };

    struct RHIErrorInfo
    {
        int32_t code;
        int32_t backendResult;
        uint32_t handleIndex;
        uint32_t handleGeneration;
        uint64_t objectIdentity;
        const char* entryPoint;
        const char* operation;
        const char* objectType;
        const char* message;
    };

    RHI_DLL EErrorCode GetLastError() noexcept;
    RHI_DLL const char* GetLastErrorMessage() noexcept;
    RHI_DLL void GetLastErrorInfo(RHIErrorInfo& outInfo) noexcept;
    RHI_DLL void ClearError() noexcept;

    RHI_DLL void SetLastError(EErrorCode code, const char* message = nullptr) noexcept;
    RHI_DLL void SetLastErrorDetailed(EErrorCode code,
                                      const char* operation,
                                      int32_t backendResult = 0,
                                      const char* objectType = nullptr,
                                      uint64_t objectIdentity = 0,
                                      uint32_t handleIndex = UINT32_MAX,
                                      uint32_t handleGeneration = 0,
                                      const char* message = nullptr) noexcept;

    [[noreturn]] RHI_DLL void ThrowInvalidParameter(const char* operation,
                                                    const char* parameter,
                                                    const char* message = nullptr);
    [[noreturn]] RHI_DLL void ThrowInvalidEnumValue(const char* operation,
                                                    const char* parameter,
                                                    int64_t value);
    [[noreturn]] RHI_DLL void ThrowInvalidFlagBits(const char* operation,
                                                  const char* parameter,
                                                  uint64_t value,
                                                  uint64_t invalidBits);
    [[noreturn]] RHI_DLL void ThrowInvalidHandle(const char* operation,
                                                const char* objectType,
                                                uint32_t handleIndex,
                                                uint32_t handleGeneration,
                                                const char* message = nullptr,
                                                uint64_t objectIdentity = 0);
    [[noreturn]] RHI_DLL void ThrowInvalidState(const char* operation,
                                               const char* objectType,
                                               uint64_t objectIdentity,
                                               const char* message,
                                               uint32_t handleIndex = UINT32_MAX,
                                               uint32_t handleGeneration = 0);
    RHI_DLL void RequireAbiPointer(const void* value,
                                   const char* operation,
                                   const char* objectType,
                                   const char* parameter);
    RHI_DLL void RequireAbiArray(const void* value,
                                 uint64_t count,
                                 const char* operation,
                                 const char* elementType,
                                 const char* parameter);

    class RHIAbiCallScope final
    {
    public:
        explicit RHIAbiCallScope(const char* entryPoint) noexcept;
        ~RHIAbiCallScope() noexcept;

        RHIAbiCallScope(const RHIAbiCallScope&) = delete;
        RHIAbiCallScope& operator=(const RHIAbiCallScope&) = delete;

        void CaptureCurrentException() noexcept;

    private:
        bool m_Failed = false;
    };
}

extern "C" RHI_DLL void RHIError_GetLastErrorInfo(ArisenEngine::RHI::RHIErrorInfo* outInfo) noexcept;

#define RHI_ABI_GUARD() \
    ::ArisenEngine::RHI::RHIAbiCallScope arisenRhiAbiCallScope(__func__); \
    try

#define RHI_ABI_CATCH_VOID() \
    catch (...) \
    { \
        arisenRhiAbiCallScope.CaptureCurrentException(); \
    }

#define RHI_ABI_CATCH_RETURN() \
    catch (...) \
    { \
        arisenRhiAbiCallScope.CaptureCurrentException(); \
        return {}; \
    }

#define RHI_ABI_REQUIRE_POINTER(value, objectType) \
    ::ArisenEngine::RHI::RequireAbiPointer((value), __func__, (objectType), #value)

#define RHI_ABI_REQUIRE_ARRAY(value, count, elementType) \
    ::ArisenEngine::RHI::RequireAbiArray((value), (count), __func__, (elementType), #value)
