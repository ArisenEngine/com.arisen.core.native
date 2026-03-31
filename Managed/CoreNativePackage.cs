using System;
using ArisenKernel.Packages;
using ArisenKernel.Services;
using ArisenKernel.Diagnostics;
using Arisen.Native.RHI;

namespace ArisenEngine.Core.Native;

public class CoreNativePackage : IPackageEntry
{
    private VulkanRHIDevice m_Device;

    public void OnLoad(IServiceRegistry registry)
    {
        KernelLog.Info("[CoreNativePackage] Initializing Native Arisen Core...");

        try
        {
            // 1. Set Vulkan as the active RHI API
            RHILoaderAPI.RHILoader_SetCurrentGraphicsAPI((int)GraphicsAPI.Vulkan);

            // 2. Initialize the RHI instance and device
            // Params: AppName, EngineName, ValidationLevel (0), Variant (0), Major, Minor, Patch...
            IntPtr nativeDevice = RHILoaderAPI.RHILoader_CreateInstance(
                "ArisenEditor", "ArisenEngine", 
                0, // Validation level
                0, // Variant
                1, 0, 0, // Engine version
                1, 0, 0, // App version
                1, 0, 0, // Patch
                2  // Max frames in flight
            );

            if (nativeDevice == IntPtr.Zero)
            {
                KernelLog.Error("[CoreNativePackage] Failed to initialize Native RHI Device (Vulkan). Viewport will be disabled.");
                return;
            }

            // 3. Wrap and Register the Service
            m_Device = new VulkanRHIDevice(nativeDevice);
            registry.RegisterService<ArisenKernel.Contracts.IRHIDevice>(m_Device);

            KernelLog.Info("[CoreNativePackage] Successfully initialized Volatile Vulkan RHI device and registered IRHIDevice.");
        }
        catch (Exception ex)
        {
            KernelLog.Error($"[CoreNativePackage] Exception during RHI initialization: {ex.Message}");
        }
    }

    public void OnUnload(IServiceRegistry registry)
    {
        KernelLog.Info("[CoreNativePackage] Shutting down Native Arisen Core...");
        
        // Ensure GPU is idle before disposing
        m_Device?.WaitIdle();
        
        RHILoaderAPI.RHILoader_Dispose();
    }
}
