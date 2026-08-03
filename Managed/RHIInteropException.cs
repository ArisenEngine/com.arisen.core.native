using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Arisen.Native.RHI;

public enum RHIErrorCode
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
}

public sealed class RHIInteropException : ExternalException
{
    internal RHIInteropException(NativeRHIErrorInfo info, string fallbackEntryPoint, int status)
        : base(BuildMessage(info, fallbackEntryPoint, status), info.BackendResult != 0 ? info.BackendResult : status)
    {
        RhiErrorCode = Enum.IsDefined(typeof(RHIErrorCode), info.Code)
            ? (RHIErrorCode)info.Code
            : RHIErrorCode.Unknown;
        BackendResult = info.BackendResult;
        HandleIndex = info.HandleIndex;
        HandleGeneration = info.HandleGeneration;
        ObjectIdentity = info.ObjectIdentity;
        EntryPoint = ReadUtf8(info.EntryPoint) ?? fallbackEntryPoint;
        Operation = ReadUtf8(info.Operation) ?? EntryPoint;
        ObjectType = ReadUtf8(info.ObjectType);
        NativeMessage = ReadUtf8(info.Message) ?? "Native RHI call failed.";
    }

    public RHIErrorCode RhiErrorCode { get; }
    public int BackendResult { get; }
    public uint HandleIndex { get; }
    public uint HandleGeneration { get; }
    public ulong ObjectIdentity { get; }
    public string EntryPoint { get; }
    public string Operation { get; }
    public string? ObjectType { get; }
    public string NativeMessage { get; }

    private static string BuildMessage(NativeRHIErrorInfo info, string fallbackEntryPoint, int status)
    {
        string entryPoint = ReadUtf8(info.EntryPoint) ?? fallbackEntryPoint;
        string operation = ReadUtf8(info.Operation) ?? entryPoint;
        string message = ReadUtf8(info.Message) ?? "Native RHI call failed.";
        string objectType = ReadUtf8(info.ObjectType) ?? "RHI object";
        string backend = info.BackendResult != 0 ? $", backend result {info.BackendResult}" : string.Empty;
        string handle = info.HandleIndex != uint.MaxValue
            ? $", {objectType} handle {info.HandleIndex}:{info.HandleGeneration}"
            : string.Empty;
        return $"{entryPoint} failed during {operation}: {message} (error {(RHIErrorCode)status}{backend}{handle}).";
    }

    private static string? ReadUtf8(IntPtr value) =>
        value == IntPtr.Zero ? null : Marshal.PtrToStringUTF8(value);
}

[StructLayout(LayoutKind.Sequential)]
internal struct NativeRHIErrorInfo
{
    public int Code;
    public int BackendResult;
    public uint HandleIndex;
    public uint HandleGeneration;
    public ulong ObjectIdentity;
    public IntPtr EntryPoint;
    public IntPtr Operation;
    public IntPtr ObjectType;
    public IntPtr Message;
}

internal static class RHIErrorNative
{
    [DllImport("Core.RHI.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern void RHIError_GetLastErrorInfo(out NativeRHIErrorInfo info);
}

internal static class RHIInterop
{
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    internal static void ThrowIfFailed(string entryPoint)
    {
        int status = Marshal.GetLastPInvokeError();
        if (status == 0)
            return;

        RHIErrorNative.RHIError_GetLastErrorInfo(out NativeRHIErrorInfo info);
        throw new RHIInteropException(info, entryPoint, status);
    }
}
