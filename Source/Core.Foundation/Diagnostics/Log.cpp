#include "Log.h"
#include "../String/String.h"

namespace ArisenEngine::Diagnostics
{
    ILogHandler* Log::s_Handler = nullptr;
    std::mutex Log::s_HandlerMutex;
    std::condition_variable Log::s_HandlerDrained;
    uint32_t Log::s_ActiveHandlerCalls = 0;

    void Log::SetHandler(ILogHandler* handler)
    {
        std::unique_lock lock(s_HandlerMutex);
        if (s_Handler == handler)
        {
            if (handler == nullptr)
            {
                s_HandlerDrained.wait(lock, [] { return s_ActiveHandlerCalls == 0; });
            }
            return;
        }

        s_Handler = nullptr;
        s_HandlerDrained.wait(lock, [] { return s_ActiveHandlerCalls == 0; });
        s_Handler = handler;
    }

    ILogHandler* Log::AcquireHandler()
    {
        std::lock_guard lock(s_HandlerMutex);
        ILogHandler* handler = s_Handler;
        if (handler != nullptr)
        {
            ++s_ActiveHandlerCalls;
        }
        return handler;
    }

    void Log::ReleaseHandler()
    {
        std::lock_guard lock(s_HandlerMutex);
        assert(s_ActiveHandlerCalls > 0);
        --s_ActiveHandlerCalls;
        if (s_ActiveHandlerCalls == 0)
        {
            s_HandlerDrained.notify_all();
        }
    }

    void Log::InternalLog(LogLevel level, const char* msg, const std::source_location& loc, const char* thread_name)
    {
        ILogHandler* handler = AcquireHandler();
        if (handler == nullptr) return;

        try
        {
            LogSourceLocation location{
                loc.file_name(),
                loc.function_name(),
                loc.line()
            };

            switch (level)
            {
            case LogLevel::Trace: handler->Trace(msg, location, thread_name);
                break;
            case LogLevel::Debug: handler->Debug(msg, location, thread_name);
                break;
            case LogLevel::Info: handler->Info(msg, location, thread_name);
                break;
            case LogLevel::Warning: handler->Warning(msg, location, thread_name);
                break;
            case LogLevel::Error: handler->Error(msg, location, thread_name);
                break;
            case LogLevel::Fatal: handler->Fatal(msg, location, thread_name);
                break;
            default: handler->Log(level, msg, location, thread_name);
                break;
            }
        }
        catch (...)
        {
            ReleaseHandler();
            throw;
        }

        ReleaseHandler();
    }
}
