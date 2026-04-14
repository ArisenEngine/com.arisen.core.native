using System;
using ArisenKernel.Contracts;
using Arisen.Native.RHI;

namespace ArisenEngine.Core.Native;

public class VulkanRHIDevice : IRHIDevice
{
    private readonly IntPtr m_NativeHandle;

    public VulkanRHIDevice(IntPtr nativeHandle)
    {
        m_NativeHandle = nativeHandle;
    }

    public bool IsValid => m_NativeHandle != IntPtr.Zero;

    public void WaitIdle()
    {
        if (IsValid)
        {
            RHIDeviceAPI.RHIDevice_DeviceWaitIdle(m_NativeHandle);
        }
    }

    public ulong SubmitCommandList(IntPtr commandBufferHandle)
    {
        if (!IsValid) return 0;
        
        // In this architecture, SubmitCommandList maps to RHIDevice_Submit.
        // We use a simplified bridge for the command buffer submission.
        return RHIDeviceAPI.RHIDevice_Submit(m_NativeHandle, 0, 0, commandBufferHandle);
    }

    public void WaitQueueTicket(ulong ticket)
    {
        if (IsValid)
        {
            RHIDeviceAPI.RHIDevice_WaitQueueTicket(m_NativeHandle, ticket);
        }
    }

    public ulong GetCompletedTicket()
    {
        if (IsValid)
        {
            return RHIDeviceAPI.RHIDevice_GetCompletedSubmitTicket(m_NativeHandle);
        }
        return 0;
    }

    public IntPtr GetSharedWin32Handle(uint index, uint generation)
    {
        if (!IsValid) return IntPtr.Zero;
        return RHIDeviceAPI.RHIDevice_GetSharedWin32Handle(m_NativeHandle, index, generation);
    }
}
