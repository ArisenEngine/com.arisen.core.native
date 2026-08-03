#include "RHIError.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <new>
#include <stdexcept>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace
{
    using namespace ArisenEngine::RHI;

    constexpr size_t EntryPointCapacity = 128;
    constexpr size_t OperationCapacity = 128;
    constexpr size_t ObjectTypeCapacity = 64;
    constexpr size_t MessageCapacity = 768;

    struct ThreadErrorState
    {
        EErrorCode code = EErrorCode::None;
        int32_t backendResult = 0;
        uint32_t handleIndex = UINT32_MAX;
        uint32_t handleGeneration = 0;
        uint64_t objectIdentity = 0;
        char entryPoint[EntryPointCapacity]{};
        char operation[OperationCapacity]{};
        char objectType[ObjectTypeCapacity]{};
        char message[MessageCapacity]{};
    };

    thread_local ThreadErrorState s_Error;

    template <size_t Capacity>
    void CopyText(char (&destination)[Capacity], const char* source) noexcept
    {
        if (!source)
        {
            destination[0] = '\0';
            return;
        }

        const size_t length = (std::min)(std::strlen(source), Capacity - 1);
        std::memcpy(destination, source, length);
        destination[length] = '\0';
    }

    const char* GetErrorString(EErrorCode code) noexcept
    {
        switch (code)
        {
        case EErrorCode::None: return "";
        case EErrorCode::OutOfMemory: return "Out of memory";
        case EErrorCode::InvalidHandle: return "Invalid or stale RHI handle";
        case EErrorCode::DeviceLost: return "GPU device was lost";
        case EErrorCode::ValidationFailed: return "RHI validation failed";
        case EErrorCode::InitializationFailed: return "RHI initialization failed";
        case EErrorCode::ShaderCompilationFailed: return "Shader compilation failed";
        case EErrorCode::PipelineCreationFailed: return "Pipeline creation failed";
        case EErrorCode::InvalidParameter: return "Invalid function parameter";
        case EErrorCode::UnsupportedFeature: return "Feature not supported";
        case EErrorCode::InvalidState: return "Invalid RHI object state";
        case EErrorCode::BackendFailure: return "RHI backend operation failed";
        case EErrorCode::NativeException: return "Native RHI exception";
        case EErrorCode::Unknown: return "Unknown RHI error";
        default: return "Unknown RHI error code";
        }
    }

    void PublishInteropStatus(EErrorCode code) noexcept
    {
#if defined(_WIN32)
        ::SetLastError(static_cast<DWORD>(code));
#else
        errno = static_cast<int>(code);
#endif
    }

    void BeginCall(const char* entryPoint) noexcept
    {
        s_Error = {};
        s_Error.handleIndex = UINT32_MAX;
        CopyText(s_Error.entryPoint, entryPoint);
        CopyText(s_Error.operation, entryPoint);
    }

    void SetExceptionError(EErrorCode code, const char* message) noexcept
    {
        if (s_Error.code != EErrorCode::None)
        {
            if (s_Error.message[0] == '\0')
                CopyText(s_Error.message, message);
            return;
        }

        s_Error.code = code;
        CopyText(s_Error.message, message ? message : GetErrorString(code));
    }
}

namespace ArisenEngine::RHI
{
    EErrorCode GetLastError() noexcept
    {
        return s_Error.code;
    }

    const char* GetLastErrorMessage() noexcept
    {
        return s_Error.message[0] != '\0' ? s_Error.message : GetErrorString(s_Error.code);
    }

    void GetLastErrorInfo(RHIErrorInfo& outInfo) noexcept
    {
        outInfo.code = static_cast<int32_t>(s_Error.code);
        outInfo.backendResult = s_Error.backendResult;
        outInfo.handleIndex = s_Error.handleIndex;
        outInfo.handleGeneration = s_Error.handleGeneration;
        outInfo.objectIdentity = s_Error.objectIdentity;
        outInfo.entryPoint = s_Error.entryPoint;
        outInfo.operation = s_Error.operation;
        outInfo.objectType = s_Error.objectType;
        outInfo.message = GetLastErrorMessage();
    }

    void ClearError() noexcept
    {
        BeginCall(nullptr);
        PublishInteropStatus(EErrorCode::None);
    }

    void SetLastError(EErrorCode code, const char* message) noexcept
    {
        SetLastErrorDetailed(code, s_Error.entryPoint, 0, nullptr, 0, UINT32_MAX, 0, message);
    }

    void SetLastErrorDetailed(EErrorCode code,
                              const char* operation,
                              int32_t backendResult,
                              const char* objectType,
                              uint64_t objectIdentity,
                              uint32_t handleIndex,
                              uint32_t handleGeneration,
                              const char* message) noexcept
    {
        s_Error.code = code;
        s_Error.backendResult = backendResult;
        s_Error.objectIdentity = objectIdentity;
        s_Error.handleIndex = handleIndex;
        s_Error.handleGeneration = handleGeneration;
        CopyText(s_Error.operation, operation ? operation : s_Error.entryPoint);
        CopyText(s_Error.objectType, objectType);
        CopyText(s_Error.message, message ? message : GetErrorString(code));
    }

    [[noreturn]] void ThrowInvalidParameter(const char* operation,
                                            const char* parameter,
                                            const char* message)
    {
        char detail[MessageCapacity]{};
        std::snprintf(detail, sizeof(detail), "%s%s%s",
                      message ? message : "Required parameter is null or invalid",
                      parameter ? ": " : "",
                      parameter ? parameter : "");
        SetLastErrorDetailed(EErrorCode::InvalidParameter, operation, 0, "Parameter", 0, UINT32_MAX, 0, detail);
        throw std::invalid_argument(detail);
    }

    [[noreturn]] void ThrowInvalidEnumValue(const char* operation,
                                            const char* parameter,
                                            int64_t value)
    {
        char detail[MessageCapacity]{};
        std::snprintf(detail, sizeof(detail), "Enum parameter '%s' has undefined value %lld",
                      parameter ? parameter : "<unknown>", static_cast<long long>(value));
        SetLastErrorDetailed(EErrorCode::InvalidParameter, operation, 0, "Enum", 0,
                             UINT32_MAX, 0, detail);
        throw std::invalid_argument(detail);
    }

    [[noreturn]] void ThrowInvalidFlagBits(const char* operation,
                                           const char* parameter,
                                           uint64_t value,
                                           uint64_t invalidBits)
    {
        char detail[MessageCapacity]{};
        std::snprintf(detail, sizeof(detail),
                      "Flag parameter '%s' has value 0x%llX with unsupported bits 0x%llX",
                      parameter ? parameter : "<unknown>",
                      static_cast<unsigned long long>(value),
                      static_cast<unsigned long long>(invalidBits));
        SetLastErrorDetailed(EErrorCode::InvalidParameter, operation, 0, "Flags", 0,
                             UINT32_MAX, 0, detail);
        throw std::invalid_argument(detail);
    }

    [[noreturn]] void ThrowInvalidHandle(const char* operation,
                                         const char* objectType,
                                         uint32_t handleIndex,
                                         uint32_t handleGeneration,
                                         const char* message,
                                         uint64_t objectIdentity)
    {
        char detail[MessageCapacity]{};
        std::snprintf(detail, sizeof(detail), "%s. Handle=%u:%u",
                      message ? message : "Invalid or stale RHI handle",
                      handleIndex,
                      handleGeneration);
        SetLastErrorDetailed(EErrorCode::InvalidHandle, operation, 0, objectType, objectIdentity,
                             handleIndex, handleGeneration, detail);
        throw std::invalid_argument(detail);
    }

    [[noreturn]] void ThrowInvalidState(const char* operation,
                                        const char* objectType,
                                        uint64_t objectIdentity,
                                        const char* message,
                                        uint32_t handleIndex,
                                        uint32_t handleGeneration)
    {
        SetLastErrorDetailed(EErrorCode::InvalidState, operation, 0, objectType, objectIdentity,
                             handleIndex, handleGeneration, message);
        throw std::logic_error(message ? message : "Invalid RHI object state");
    }

    void RequireAbiPointer(const void* value,
                           const char* operation,
                           const char* objectType,
                           const char* parameter)
    {
        if (!value)
            ThrowInvalidParameter(operation, parameter, objectType);
    }

    void RequireAbiArray(const void* value,
                         uint64_t count,
                         const char* operation,
                         const char* elementType,
                         const char* parameter)
    {
        if (count > 0 && !value)
            ThrowInvalidParameter(operation, parameter, elementType);
    }

    RHIAbiCallScope::RHIAbiCallScope(const char* entryPoint) noexcept
    {
        BeginCall(entryPoint);
    }

    RHIAbiCallScope::~RHIAbiCallScope() noexcept
    {
        if (!m_Failed)
            PublishInteropStatus(EErrorCode::None);
    }

    void RHIAbiCallScope::CaptureCurrentException() noexcept
    {
        m_Failed = true;
        try
        {
            throw;
        }
        catch (const std::bad_alloc& exception)
        {
            SetExceptionError(EErrorCode::OutOfMemory, exception.what());
        }
        catch (const std::invalid_argument& exception)
        {
            SetExceptionError(EErrorCode::InvalidParameter, exception.what());
        }
        catch (const std::logic_error& exception)
        {
            SetExceptionError(EErrorCode::InvalidState, exception.what());
        }
        catch (const std::exception& exception)
        {
            SetExceptionError(EErrorCode::NativeException, exception.what());
        }
        catch (...)
        {
            SetExceptionError(EErrorCode::Unknown, "Unknown native exception");
        }

        PublishInteropStatus(s_Error.code == EErrorCode::None ? EErrorCode::Unknown : s_Error.code);
    }
}

extern "C" RHI_DLL void RHIError_GetLastErrorInfo(ArisenEngine::RHI::RHIErrorInfo* outInfo) noexcept
{
    if (outInfo)
        ArisenEngine::RHI::GetLastErrorInfo(*outInfo);
}
