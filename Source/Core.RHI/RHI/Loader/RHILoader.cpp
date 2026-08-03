#include "RHILoader.h"
#include "RHI/Diagnostics/RHIAbiOwnerRegistry.h"
#include "Logger/Logger.h"
#include "Base/FoundationMinimal.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")

namespace
{
    ArisenEngine::String FormatWin32Error(DWORD error)
    {
        if (error == 0)
        {
            return ArisenEngine::String("Win32Error=0");
        }

        return ArisenEngine::String::Format("Win32Error=%lu", error);
    }
}

namespace ArisenEngine::RHI
{
    void RHILoader::DestroyCurrentInstance() noexcept
    {
        if (_current_instance == nullptr)
        {
            return;
        }

        LOG_INFO("[RHILoader::DestroyCurrentInstance] Destroying active RHI instance.");
        auto* instance = _current_instance;
        _current_instance = nullptr;
        InvalidateAbiOwnerByObject(instance, ERHIAbiOwnerType::Instance);
        delete instance;
    }

    void RHILoader::DestroyInstance(RHIInstance* instance) noexcept
    {
        if (instance == nullptr)
        {
            return;
        }

        if (instance != _current_instance)
        {
            LOG_WARN("[RHILoader::DestroyInstance] Ignoring a non-current RHI instance.");
            return;
        }

        DestroyCurrentInstance();
    }

    void RHILoader::SetCurrentGraphicsAPI(GraphicsAPI api_type)
    {
        _last_error = String();

        if (_rhi_dll != nullptr && _api_type == api_type)
        {
            return;
        }

        if (_rhi_dll != nullptr)
        {
            Dispose();
        }

        switch (api_type)
        {
        case GraphicsAPI::Vulkan:
            _rhi_dll = LoadLibraryA("RHI.Vulkan.dll");
            break;

        default:
            _last_error = String::Format("Unsupported graphics api: %d", static_cast<int>(api_type));
            LOG_FATAL(_last_error);
            return;
        }

        if (!_rhi_dll)
        {
            const DWORD error = ::GetLastError();
            _last_error = String::Format(
                "Failed to load RHI.Vulkan.dll (%s). Ensure the Vulkan native package payload was deployed and its dependent runtime DLLs are available.",
                FormatWin32Error(error).GetString());
            LOG_FATAL(_last_error);
            return;
        }

        _api_type = api_type;

        char dllPath[MAX_PATH];
        DWORD result = GetModuleFileNameA((HMODULE)_rhi_dll, dllPath, MAX_PATH);
        if (result != 0)
        {
            LOG_INFO(String::Format("[RHILoader::SetCurrentGraphicsAPI] RHI dll loaded: %s", dllPath));
        }

        HANDLE process = GetCurrentProcess();

        // Initialize symbol handler
        if (!_symbols_initialized && !SymInitialize(process, nullptr, FALSE))
        {
            DWORD error = GetLastError();
            if (error == ERROR_INVALID_FUNCTION)
            {
                LOG_DEBUG("Symbols are already initialized.");
            }
            else
            {
                LOG_FATAL(String::Format("SymInitialize failed. Error code: %lu", error));
            }
        }
        else if (!_symbols_initialized)
        {
            _symbols_initialized = true;
            LOG_DEBUG("Symbols initialized successfully.");
        }

        if (_symbols_initialized)
        {
            // Unload previous symbols for this module if they exist (to handle reloads)
            DWORD64 moduleBase = (DWORD64)_rhi_dll;
            SymUnloadModule64(process, moduleBase);

            // Load symbols for the module
            if (SymLoadModuleEx(
                process,
                nullptr,
                dllPath,
                nullptr,
                moduleBase,
                0,
                nullptr,
                0))
            {
                LOG_INFO(String::Format("[RHILoader::SetCurrentGraphicsAPI] %s Symbols loaded.", dllPath));
                SymRefreshModuleList(process);

                IMAGEHLP_MODULE64 moduleInfo = {sizeof(IMAGEHLP_MODULE64)};
                if (SymGetModuleInfo64(process, moduleBase, &moduleInfo))
                {
                    LOG_INFO(
                        String::Format("Loaded symbols: %s, Loaded PDB Name: %s", moduleInfo.LoadedImageName, moduleInfo.
                            LoadedPdbName));
                }
            }
            else
            {
                LOG_WARN(String::Format("Failed to load symbols for: %s. Error: %lu", dllPath, GetLastError()));
            }
        }
    }

    RHIInstance* RHILoader::CreateInstance(RHIInstanceInfo&& app_info)
    {
        if (_rhi_dll == nullptr)
        {
            _last_error = "RHI dll not loaded. Call RHILoader::SetCurrentGraphicsAPI before creating an instance.";
            LOG_FATAL(_last_error);
            return nullptr;
        }

        typedef RHIInstance* (__fastcall*InstanceCreate)(RHIInstanceInfo&& app_info);
        InstanceCreate createInstance = (InstanceCreate)GetProcAddress((HMODULE)_rhi_dll, "CreateInstance");

        if (!createInstance)
        {
            const DWORD error = ::GetLastError();
            _last_error = String::Format(
                "Failed to find 'CreateInstance' in RHI dll (%s). The native RHI payload may be stale or incompatible with Core.RHI.",
                FormatWin32Error(error).GetString());
            LOG_FATAL(_last_error);
            return nullptr;
        }

        try
        {
            DestroyCurrentInstance();

            auto* instance = createInstance(std::move(app_info));
            if (instance == nullptr)
            {
                _last_error = "Native RHI CreateInstance returned null.";
            }
            else
            {
                _current_instance = instance;
                _last_error = String();
            }

            return instance;
        }
        catch (const std::exception& e)
        {
            _last_error = String::Format("Native RHI CreateInstance failed: %s", e.what());
            LOG_FATAL(_last_error);
            return nullptr;
        }
        catch (...)
        {
            _last_error = "Native RHI CreateInstance failed with an unknown native exception.";
            LOG_FATAL(_last_error);
            return nullptr;
        }
    }

    void RHILoader::Dispose()
    {
        DestroyCurrentInstance();

        if (_rhi_dll != nullptr)
        {
            if (_symbols_initialized)
            {
                SymUnloadModule64(GetCurrentProcess(), (DWORD64)_rhi_dll);
            }

            FreeLibrary((HMODULE)_rhi_dll);
            _rhi_dll = nullptr;
        }

        if (_symbols_initialized)
        {
            HANDLE process = GetCurrentProcess();
            SymCleanup(process);
            _symbols_initialized = false;
        }

        _api_type = GraphicsAPI::None;
    }

    String RHILoader::GetLastErrorMessage()
    {
        return _last_error;
    }
} // namespace ArisenEngine::RHI
