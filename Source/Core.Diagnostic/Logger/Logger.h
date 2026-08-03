#pragma once

#include "Base/FoundationMinimal.h"
#include "../CoreDiagnosticCommon.h"
#include "../../Core.Foundation/Diagnostics/ILogHandler.h"

namespace ArisenEngine::Diagnostics
{
    using LogCallback = void(*)(UInt32, const char*, const char*, const char*);

    class DIAGNOSTIC_DLL Logger final : public ILogHandler
    {
        enum class LifecycleState : UInt8
        {
            Stopped,
            Initializing,
            Accepting,
            StopRequested
        };

    public:
        NO_COPY_NO_MOVE(Logger)
        NO_COMPARE(Logger)

        // Implementation of ILogHandler
        void Log(LogLevel level, const char* msg, const LogSourceLocation& location,
                 const char* thread_name = nullptr) override;

        void SetServerityLevel(LogLevel level);
        void BindCallback(LogCallback callback);
        bool Initialize();
        void Flush();

        static Logger& GetInstance();
        static void Shutdown();

    private:
        bool BeginLog(LogCallback& callback);
        void EndLog();

        std::mutex m_LifecycleMutex;
        std::condition_variable m_LifecycleChanged;
        LifecycleState m_LifecycleState;
        UInt32 m_ActiveLogs;
        LogCallback m_LogCallback;
        Logger();
    };
}
