using System;
using ArisenKernel.Packages;
using ArisenKernel.Services;
using ArisenKernel.Diagnostics;

namespace ArisenEngine.Core.Native;

/// <summary>
/// Provides the monolithic underlying C++ Core payload encompassing Foundation, HAL, and Diagnostics logic.
/// This package is a passive provider of native runtime handles and P/Invoke bindings.
/// </summary>
public class CoreNativePackage : IPackageEntry
{
    public void OnLoad(IServiceRegistry registry)
    {
        // RHI Initialization is now handled centrally by the Arisen Core Managed package (NativeRuntime).
        // This hybrid package simply ensures the native dependencies are available in the runtime directory.
        KernelLog.Info("[CoreNativePackage] Native provider loaded.");
    }

    public void OnUnload(IServiceRegistry registry)
    {
        KernelLog.Info("[CoreNativePackage] Native provider unloaded.");
    }
}
