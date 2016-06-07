using System;
using System.Text;
using System.Runtime.InteropServices;

public class SESDK
{
    [StructLayout(LayoutKind.Sequential)]
    public struct SYSTEMTIME
    {
        public ushort Year;
        public ushort Month;
        public ushort DayOfWeek;
        public ushort Day;
        public ushort Hour;
        public ushort Minute;
        public ushort Second;
        public ushort Milliseconds;
    }
    public enum SEAppStatus
    {
        SE_STATUS_INIT,
        SE_STATUS_NORMAL
    }
    [DllImport("SESDKDummy.dll", EntryPoint = "SECheckProtection", CallingConvention = CallingConvention.StdCall)]
    public static extern bool SECheckProtection();

    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetAppStatus", CallingConvention = CallingConvention.StdCall)]
    public static extern SEAppStatus SEGetAppStatus();

    [DllImport("SESDKDummy.dll", EntryPoint = "SESetAppStatus", CallingConvention = CallingConvention.StdCall)]
    public static extern void SESetAppStatus(SEAppStatus NewStatus);

    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetProtectionDate", CallingConvention = CallingConvention.StdCall)]
    public static extern void SEGetProtectionDate([Out] SYSTEMTIME pDate);
    
    [DllImport("SESDKDummy.dll", EntryPoint = "SEAddMemoryGuard", CallingConvention = CallingConvention.StdCall)]
    public static extern void SEAddMemoryGuard(void* pData, ulong ulSize);

    [DllImport("SESDKDummy.dll", EntryPoint = "SEDelMemoryGuard", CallingConvention = CallingConvention.StdCall)]
    public static extern void SEDelMemoryGuard(void* pData, ulong ulSize);
}
