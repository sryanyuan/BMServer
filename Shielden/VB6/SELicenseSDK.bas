Attribute VB_Name = "SELicenseSDK"

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

    Public Type SYSTEMTIME
        Year As Integer
        Month As Integer
        DayOfWeek As Integer
        Day As Integer
        Hour As Integer
        Minute As Integer
        Second As Integer
        Milliseconds As Integer
    End Type

    Public Type SELicenseUserInfoW
        UserID As String * 512
        Remarks As String * 2048
        LicenseDate As SYSTEMTIME
    End Type


    Public Type SELicenseTrialInfo
        NumDays As Long
        NumExec As Long
        ExpDate As SYSTEMTIME
        CountryId As Long
        ExecTime As Long
        TotalExecTime As Long
    End Type


    Public Type SELicenseHashInfo
        Hash As Byte
    End Type

    Public Declare Sub SEGetLicenseUserInfoW Lib "SESDKDummy.dll" (ByRef pInfo As SELicenseUserInfoW)

    Public Declare Sub SEGetLicenseTrialInfo Lib "SESDKDummy.dll" (ByRef pInfo As SELicenseTrialInfo)

    Public Declare Function SEGetNumExecUsed Lib "SESDKDummy.dll" () As Long

    Public Declare Function SEGetNumExecLeft Lib "SESDKDummy.dll" () As Long

    Public Declare Function SESetNumExecUsed Lib "SESDKDummy.dll" (ByVal Num As Long) As SEErrorType

    Public Declare Function SEGetExecTimeUsed Lib "SESDKDummy.dll" () As Long

    Public Declare Function SEGetExecTimeLeft Lib "SESDKDummy.dll" () As Long

    Public Declare Function SESetExecTime Lib "SESDKDummy.dll" (ByVal Num As Long) As SEErrorType

    Public Declare Function SEGetTotalExecTimeUsed Lib "SESDKDummy.dll" () As Long

    Public Declare Function SEGetTotalExecTimeLeft Lib "SESDKDummy.dll" () As Long

    Public Declare Function SESetTotalExecTime Lib "SESDKDummy.dll" (ByVal Num As Long) As SEErrorType

    Public Declare Function SEGetNumDaysUsed Lib "SESDKDummy.dll" () As Long

    Public Declare Function SEGetNumDaysLeft Lib "SESDKDummy.dll" () As Long

    Public Declare Function SECheckHardwareID Lib "SESDKDummy.dll" () As SEErrorType


    Public Declare Function SECheckExpDate Lib "SESDKDummy.dll" () As SEErrorType

    Public Declare Function SECheckExecTime Lib "SESDKDummy.dll" () As SEErrorType

    Public Declare Function SECheckTotalExecTime Lib "SESDKDummy.dll" () As SEErrorType

    Public Declare Function SECheckCountryID Lib "SESDKDummy.dll" () As SEErrorType

    Public Declare Function SEGetHardwareIDW Lib "SESDKDummy.dll" (ByVal pBuf As Long, ByVal MaxWChars As Integer) As SEErrorType

    Public Declare Function SECheckLicenseFileW Lib "SESDKDummy.dll" (ByVal LicenseFileName As Long) As SEErrorType

    Public Declare Function SEGetLicenseHash Lib "SESDKDummy.dll" (ByVal pInfo As SELicenseHashInfo) As SEErrorType

    Public Declare Sub SENotifyLicenseBanned Lib "SESDKDummy.dll" (ByVal bExit As Boolean)

    Public Declare Function SEResetTrial Lib "SESDKDummy.dll" () As SEErrorType
    
