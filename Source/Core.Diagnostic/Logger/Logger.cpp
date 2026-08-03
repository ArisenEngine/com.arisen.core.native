#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <thread>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <ctime>

#if defined(__has_include) && __has_include(<stacktrace>) && __cpp_lib_stacktrace >= 202011
#define HAS_STD_STACKTRACE 1
#include <stacktrace>
#else
    #define HAS_STD_STACKTRACE 0
#endif

#include "Logger.h"
#include "../../Core.Foundation/Diagnostics/Log.h"

namespace ArisenEngine::Diagnostics
{
    static String GetStacktrace()
    {
#if HAS_STD_STACKTRACE
        std::stringstream trace_stream;
        auto trace = std::stacktrace::current();
        for (size_t i = 1; i < trace.size(); ++i)
        {
            const auto& entry = trace[i];
            if (!entry.description().empty() || !entry.source_file().empty())
            {
                trace_stream << i << "> " << entry.source_file() << "(" << entry.source_line() << "): " << entry.
                    description() << "\n";
            }
        }
        return String(trace_stream.str());
#else
        return "[stacktrace not available]";
#endif
    }

    Logger::Logger()
        : m_LifecycleState(LifecycleState::Stopped),
          m_ActiveLogs(0),
          m_LogCallback(nullptr)
    {
    }

#ifdef _WIN32
#include <windows.h>
#endif

    bool Logger::Initialize()
    {
        {
            std::unique_lock lock(m_LifecycleMutex);
            m_LifecycleChanged.wait(lock, [this]
            {
                return m_LifecycleState != LifecycleState::Initializing &&
                    m_LifecycleState != LifecycleState::StopRequested;
            });
            if (m_LifecycleState == LifecycleState::Accepting) return true;
            m_LifecycleState = LifecycleState::Initializing;
        }

        bool initialized = false;
        try
        {
            std::filesystem::path log_dir;
#ifdef _WIN32
            wchar_t exePathW[MAX_PATH]{};
            GetModuleFileNameW(nullptr, exePathW, MAX_PATH);
            log_dir = std::filesystem::path(exePathW).parent_path() / "logs";
#else
            log_dir = std::filesystem::absolute(std::filesystem::path("logs"));
#endif
            std::error_code _ec;
            std::filesystem::create_directories(log_dir, _ec);

            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm tm;
            localtime_s(&tm, &in_time_t); // Thread-safe on Windows

            std::stringstream ss;
            ss << "player_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".log";
            const auto log_file = (log_dir / ss.str()).string();

            constexpr size_t queue_size = 8192;
            constexpr size_t num_threads = 1;
            spdlog::init_thread_pool(queue_size, num_threads);

            auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("log", log_file, true);
            spdlog::set_default_logger(async_file);

#if _DEBUG
            spdlog::flush_every(std::chrono::seconds(1));
            spdlog::flush_on(spdlog::level::err);
#else
            // Production: Minimize I/O impact. Rely on OS page cache and fatal crash handling.
            spdlog::flush_every(std::chrono::seconds(5));
            spdlog::flush_on(spdlog::level::critical);
#endif

#if _DEBUG
            spdlog::set_level(spdlog::level::trace);
#else
            spdlog::set_level(spdlog::level::info);
#endif

            spdlog::set_pattern("[%Y-%m-%d %T.%e][process %p][thread %t][%l] %v");

            // Register with Foundation Bridge
            ArisenEngine::Diagnostics::Log::SetHandler(this);
            initialized = true;
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            std::printf("Log initialization failed: %s\n", ex.what());
        }
        catch (const std::exception& ex)
        {
            std::printf("Log initialization failed: %s\n", ex.what());
        }
        catch (...)
        {
            std::printf("Log initialization failed with an unknown native error.\n");
        }

        if (!initialized)
        {
            try
            {
                Log::SetHandler(nullptr);
            }
            catch (const std::exception& ex)
            {
                std::printf("Log handler rollback failed: %s\n", ex.what());
            }
            catch (...)
            {
                std::printf("Log handler rollback failed with an unknown native error.\n");
            }

            try
            {
                spdlog::shutdown();
            }
            catch (const std::exception& ex)
            {
                std::printf("Log queue rollback failed: %s\n", ex.what());
            }
            catch (...)
            {
                std::printf("Log queue rollback failed with an unknown native error.\n");
            }
        }

        {
            std::lock_guard lock(m_LifecycleMutex);
            m_LifecycleState = initialized
                ? LifecycleState::Accepting
                : LifecycleState::Stopped;
        }
        m_LifecycleChanged.notify_all();
        return initialized;
    }

    Logger& Logger::GetInstance()
    {
        static Logger _log_instnace;
        return _log_instnace;
    }

    void Logger::Flush()
    {
        if (auto logger = spdlog::default_logger())
        {
            logger->flush();
        }
    }

    void Logger::Shutdown()
    {
        Logger& instance = GetInstance();
        {
            std::unique_lock lock(instance.m_LifecycleMutex);
            instance.m_LifecycleChanged.wait(lock, [&instance]
            {
                return instance.m_LifecycleState != LifecycleState::Initializing;
            });
            if (instance.m_LifecycleState == LifecycleState::StopRequested)
            {
                instance.m_LifecycleChanged.wait(lock, [&instance]
                {
                    return instance.m_LifecycleState == LifecycleState::Stopped;
                });
                return;
            }

            if (instance.m_LifecycleState == LifecycleState::Stopped)
            {
                lock.unlock();
                Log::SetHandler(nullptr);
                return;
            }

            instance.m_LifecycleState = LifecycleState::StopRequested;
        }

        Log::SetHandler(nullptr);

        {
            std::unique_lock lock(instance.m_LifecycleMutex);
            instance.m_LifecycleChanged.wait(lock, [&instance]
            {
                return instance.m_ActiveLogs == 0;
            });
            instance.m_LogCallback = nullptr;
        }

        std::exception_ptr shutdownFailure;
        try
        {
            if (auto* logger = spdlog::default_logger_raw())
            {
                logger->flush();
            }
            spdlog::shutdown();
        }
        catch (...)
        {
            shutdownFailure = std::current_exception();
        }

        {
            std::lock_guard lock(instance.m_LifecycleMutex);
            instance.m_LifecycleState = LifecycleState::Stopped;
        }
        instance.m_LifecycleChanged.notify_all();

        if (shutdownFailure)
        {
            std::rethrow_exception(shutdownFailure);
        }
    }

    void Logger::SetServerityLevel(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Error:
            spdlog::set_level(spdlog::level::err);
            break;
        case LogLevel::Fatal:
            spdlog::set_level(spdlog::level::critical);
            break;
        case LogLevel::Info:
            spdlog::set_level(spdlog::level::info);
            break;
        case LogLevel::Debug:
            spdlog::set_level(spdlog::level::debug);
            break;
        case LogLevel::Trace:
            spdlog::set_level(spdlog::level::trace);
            break;
        case LogLevel::Warning:
            spdlog::set_level(spdlog::level::warn);
            break;
        }
    }

    void Logger::BindCallback(LogCallback callback)
    {
        std::unique_lock lock(m_LifecycleMutex);
        if (callback != nullptr && m_LifecycleState != LifecycleState::Accepting)
        {
            return;
        }

        m_LogCallback = callback;
        if (callback == nullptr)
        {
            m_LifecycleChanged.wait(lock, [this] { return m_ActiveLogs == 0; });
        }
    }

    void Logger::Log(LogLevel level, const char* msg, const LogSourceLocation& location, const char* thread_name)
    {
        LogCallback callback = nullptr;
        if (!BeginLog(callback)) return;

        try
        {
            spdlog::level::level_enum spd_level;
            bool needs_trace = false;
            switch (level)
            {
            case LogLevel::Trace: spd_level = spdlog::level::trace;
                break;
            case LogLevel::Debug: spd_level = spdlog::level::debug;
                break;
            case LogLevel::Info: spd_level = spdlog::level::info;
                break;
            case LogLevel::Warning: spd_level = spdlog::level::warn;
                needs_trace = true;
                break;
            case LogLevel::Error: spd_level = spdlog::level::err;
                needs_trace = true;
                break;
            case LogLevel::Fatal: spd_level = spdlog::level::critical;
                needs_trace = true;
                break;
            default: spd_level = spdlog::level::info;
                break;
            }

            String full_msg = msg ? msg : "";
            String trace;
            if (needs_trace)
            {
                trace = GetStacktrace();
                if (!trace.IsEmpty())
                {
                    full_msg += "\n[stacktrace]\n" + trace;
                }
            }

            spdlog::source_loc loc(location.file, static_cast<int>(location.line), location.function);

            // Use spdlog's native logging with source location
            if (auto logger = spdlog::default_logger())
            {
                logger->log(loc, spd_level, full_msg.GetString());
            }

            if (callback)
            {
                // For callback, we still provide a thread ID string if not provided
                String tid;
                if (thread_name)
                {
                    tid = thread_name;
                }
                else
                {
                    std::stringstream ss;
                    ss << std::this_thread::get_id();
                    tid = ss.str().c_str();
                }

                callback(static_cast<UInt32>(level), tid.c_str(), msg ? msg : "", trace.c_str());
            }
        }
        catch (...)
        {
            EndLog();
            throw;
        }

        EndLog();
    }

    bool Logger::BeginLog(LogCallback& callback)
    {
        std::lock_guard lock(m_LifecycleMutex);
        if (m_LifecycleState != LifecycleState::Accepting) return false;

        ++m_ActiveLogs;
        callback = m_LogCallback;
        return true;
    }

    void Logger::EndLog()
    {
        std::lock_guard lock(m_LifecycleMutex);
        assert(m_ActiveLogs > 0);
        --m_ActiveLogs;
        if (m_ActiveLogs == 0)
        {
            m_LifecycleChanged.notify_all();
        }
    }
} // namespace ArisenEngine::Diagnostics

#include "LoggerAPI.h"

namespace ArisenEngine::Diagnostics
{
    void Logger_Log(LogLevel level, const char* msg, const LogSourceLocation* location, const char* thread_name)
    {
        LogSourceLocation default_loc = {"Unknown", "Unknown", 0};
        const LogSourceLocation* loc_to_use = location ? location : &default_loc;
        Logger::GetInstance().Log(level, msg, *loc_to_use, thread_name);
    }

    void Logger_SetServerityLevel(LogLevel level)
    {
        Logger::GetInstance().SetServerityLevel(level);
    }

    void Logger_BindCallback(void* callback)
    {
        Logger::GetInstance().BindCallback(reinterpret_cast<LogCallback>(callback));
    }

    bool Logger_Initialize(bool bindCallback)
    {
        return Logger::GetInstance().Initialize();
    }

    void Logger_Flush()
    {
        Logger::GetInstance().Flush();
    }

    void Logger_Shutdown()
    {
        Logger::Shutdown();
    }
}
