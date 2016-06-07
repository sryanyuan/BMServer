using System;
using System.Text;
using System.Runtime.InteropServices;

public class SELicenseSDK
{
    public enum SEErrorType
    {
        SE_ERR_SUCCESS,
        SE_ERR_INTERNAL_ERROR,
        SE_ERR_TOOL_DETECTION,
        SE_ERR_CHECKSUM_FAILED,
        SE_ERR_VIRTUALIZATION_FOUND,
        SE_ERR_LICENSE_NOT_FOUND,
        SE_ERR_LICENSE_CORRUPTED,
        SE_ERR_LICENSE_FILE_MISMATCH,
        SE_ERR_LICENSE_HARDWARE_ID_MISMATCH,

        SE_ERR_LICENSE_DAYS_EXPIRED,
        SE_ERR_LICENSE_EXEC_EXPIRED,
        SE_ERR_LICENSE_DATE_EXPIRED,
        SE_ERR_LICENSE_COUNTRY_ID_MISMATCH,
        SE_ERR_LICENSE_NO_MORE_EXEC_TIME,
        SE_ERR_LICENSE_NO_MORE_TOTALEXEC_TIME,
        SE_ERR_LICENSE_BANNED,
        SE_ERR_SERVER_ERROR
    }

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
    [StructLayout(LayoutKind.Sequential)]
    unsafe public struct SELicenseUserInfoW
    {
        public fixed char UserID[512];
        public fixed char Remarks[2048];
        public SYSTEMTIME LicenseDate;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SELicenseTrialInfo
    {
        public uint NumDays;
        public uint NumExec;
        public SYSTEMTIME ExpDate;
        public uint CountryId;
        public uint ExecTime;
        public uint TotalExecTime;
    }

    [StructLayout(LayoutKind.Sequential)]
    unsafe public struct SELicenseHashInfo
    {
        public fixed byte Hash[16];
    }

    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetLicenseUserInfoW", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
    public static extern void SEGetLicenseUserInfoW([Out] SELicenseUserInfoW pInfo);

    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetLicenseTrialInfo", CallingConvention = CallingConvention.StdCall)]
    public static extern void SEGetLicenseTrialInfo([Out] SELicenseTrialInfo pInfo);

    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetNumExecUsed", CallingConvention = CallingConvention.StdCall)]
    public static extern uint SEGetNumExecUsed();
    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetNumExecLeft", CallingConvention = CallingConvention.StdCall)]
    public static extern uint SEGetNumExecLeft();
    [DllImport("SESDKDummy.dll", EntryPoint = "SESetNumExecUsed", CallingConvention = CallingConvention.StdCall)]
    public static extern SEErrorType SESetNumExecUsed(uint Num);

    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetExecTimeUsed", CallingConvention = CallingConvention.StdCall)]
    public static extern uint SEGetExecTimeUsed();
    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetExecTimeLeft", CallingConvention = CallingConvention.StdCall)]
    public static extern uint SEGetExecTimeLeft();
    [DllImport("SESDKDummy.dll", EntryPoint = "SESetExecTime", CallingConvention = CallingConvention.StdCall)]
    public static extern SEErrorType SESetExecTime(uint Num);

    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetTotalExecTimeUsed", CallingConvention = CallingConvention.StdCall)]
    public static extern uint SEGetTotalExecTimeUsed();
    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetTotalExecTimeLeft", CallingConvention = CallingConvention.StdCall)]
    public static extern uint SEGetTotalExecTimeLeft();
    [DllImport("SESDKDummy.dll", EntryPoint = "SESetTotalExecTime", CallingConvention = CallingConvention.StdCall)]
    public static extern SEErrorType SESetTotalExecTime(uint Num);

    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetNumDaysUsed", CallingConvention = CallingConvention.StdCall)]
    public static extern uint SEGetNumDaysUsed();
    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetNumDaysLeft", CallingConvention = CallingConvention.StdCall)]
    public static extern uint SEGetNumDaysLeft();

    [DllImport("SESDKDummy.dll", EntryPoint = "SECheckHardwareID", CallingConvention = CallingConvention.StdCall)]
    public static extern SEErrorType SECheckHardwareID();

    [DllImport("SESDKDummy.dll", EntryPoint = "SECheckExpDate", CallingConvention = CallingConvention.StdCall)]
    public static extern SEErrorType SECheckExpDate();

    [DllImport("SESDKDummy.dll", EntryPoint = "SECheckExecTime", CallingConvention = CallingConvention.StdCall)]
    public static extern SEErrorType SECheckExecTime();

    [DllImport("SESDKDummy.dll", EntryPoint = "SECheckTotalExecTime", CallingConvention = CallingConvention.StdCall)]
    public static extern SEErrorType SECheckTotalExecTime();

    [DllImport("SESDKDummy.dll", EntryPoint = "SECheckCountryID", CallingConvention = CallingConvention.StdCall)]
    public static extern SEErrorType SECheckCountryID();

    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetHardwareIDW", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
    public static extern SEErrorType SEGetHardwareIDW([Out] StringBuilder pBuf, int MaxWChars);

    [DllImport("SESDKDummy.dll", EntryPoint = "SECheckLicenseFileW", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
    public static extern SEErrorType SECheckLicenseFileW(string LicenseFileName);

    [DllImport("SESDKDummy.dll", EntryPoint = "SEGetLicenseHash", CallingConvention = CallingConvention.StdCall)]
    public static extern SEErrorType SEGetLicenseHash([Out] SELicenseHashInfo pInfo);

    [DllImport("SESDKDummy.dll", EntryPoint = "SENotifyLicenseBanned", CallingConvention = CallingConvention.StdCall)]
    public static extern void SENotifyLicenseBanned(bool bExit);

    [DllImport("SESDKDummy.dll", EntryPoint = "SEResetTrial", CallingConvention = CallingConvention.StdCall)]
    public static extern SEErrorType SEResetTrial();
}
