Imports System.Text
Imports System.Runtime.InteropServices

Public Class SELicenseSDK
    Public Enum SEErrorType
        SE_ERR_SUCCESS
        SE_ERR_INTERNAL_ERROR
        SE_ERR_TOOL_DETECTION
        SE_ERR_CHECKSUM_FAILED
        SE_ERR_VIRTUALIZATION_FOUND
        SE_ERR_LICENSE_NOT_FOUND
        SE_ERR_LICENSE_CORRUPTED
        SE_ERR_LICENSE_FILE_MISMATCH
        SE_ERR_LICENSE_HARDWARE_ID_MISMATCH

        SE_ERR_LICENSE_DAYS_EXPIRED
        SE_ERR_LICENSE_EXEC_EXPIRED
        SE_ERR_LICENSE_DATE_EXPIRED
        SE_ERR_LICENSE_COUNTRY_ID_MISMATCH
        SE_ERR_LICENSE_NO_MORE_EXEC_TIME
        SE_ERR_LICENSE_NO_MORE_TOTALEXEC_TIME
        SE_ERR_LICENSE_BANNED
        SE_ERR_SERVER_ERROR
    End Enum

    <StructLayout(LayoutKind.Sequential)> _
    Public Structure SYSTEMTIME
        Public Year As UShort
        Public Month As UShort
        Public DayOfWeek As UShort
        Public Day As UShort
        Public Hour As UShort
        Public Minute As UShort
        Public Second As UShort
        Public Milliseconds As UShort
    End Structure
    <StructLayout(LayoutKind.Sequential)> _
    Public Structure SELicenseUserInfoW
        Public UserID(512) As Char
        Public Remarks(2048) As Char
        Public LicenseDate As SYSTEMTIME
    End Structure

    <StructLayout(LayoutKind.Sequential)> _
    Public Structure SELicenseTrialInfo
        Public NumDays As UInteger
        Public NumExec As UInteger
        Public ExpDate As SYSTEMTIME
        Public CountryId As UInteger
        Public ExecTime As UInteger
        Public TotalExecTime As UInteger
    End Structure

    <StructLayout(LayoutKind.Sequential)> _
    Public Structure SELicenseHashInfo
        Public Hash As Byte
    End Structure

    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetLicenseUserInfoW", CallingConvention:=CallingConvention.StdCall, CharSet:=CharSet.Unicode)> _
    Public Shared Sub SEGetLicenseUserInfoW(<Out()> ByVal pInfo As SELicenseUserInfoW)
    End Sub

    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetLicenseTrialInfo", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Sub SEGetLicenseTrialInfo(<Out()> ByVal pInfo As SELicenseTrialInfo)
    End Sub

    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetNumExecUsed", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEGetNumExecUsed() As UInteger
    End Function
    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetNumExecLeft", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEGetNumExecLeft() As UInteger
    End Function
    <DllImport("SESDKDummy.dll", EntryPoint:="SESetNumExecUsed", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SESetNumExecUsed(ByVal Num As UInteger) As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetExecTimeUsed", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEGetExecTimeUsed() As UInteger
    End Function
    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetExecTimeLeft", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEGetExecTimeLeft() As UInteger
    End Function
    <DllImport("SESDKDummy.dll", EntryPoint:="SESetExecTime", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SESetExecTime(ByVal Num As UInteger) As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetTotalExecTimeUsed", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEGetTotalExecTimeUsed() As UInteger
    End Function
    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetTotalExecTimeLeft", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEGetTotalExecTimeLeft() As UInteger
    End Function
    <DllImport("SESDKDummy.dll", EntryPoint:="SESetTotalExecTime", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SESetTotalExecTime(ByVal Num As UInteger) As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetNumDaysUsed", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEGetNumDaysUsed() As UInteger
    End Function
    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetNumDaysLeft", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEGetNumDaysLeft() As UInteger
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SECheckHardwareID", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SECheckHardwareID() As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SECheckExpDate", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SECheckExpDate() As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SECheckExecTime", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SECheckExecTime() As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SECheckTotalExecTime", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SECheckTotalExecTime() As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SECheckCountryID", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SECheckCountryID() As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetHardwareIDW", CallingConvention:=CallingConvention.StdCall, CharSet:=CharSet.Unicode)> _
    Public Shared Function SEGetHardwareIDW(<Out()> ByVal pBuf As StringBuilder, ByVal MaxWChars As Integer) As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SECheckLicenseFileW", CallingConvention:=CallingConvention.StdCall, CharSet:=CharSet.Unicode)> _
    Public Shared Function SECheckLicenseFileW(ByVal LicenseFileName As String) As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetLicenseHash", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEGetLicenseHash(<Out()> ByVal pInfo As SELicenseHashInfo) As SEErrorType
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SENotifyLicenseBanned", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Sub SENotifyLicenseBanned(ByVal bExit As Boolean)
    End Sub

    <DllImport("SESDKDummy.dll", EntryPoint:="SEResetTrial", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEResetTrial() As SEErrorType
    End Function
End Class