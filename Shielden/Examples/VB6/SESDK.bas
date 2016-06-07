Attribute VB_Name = "SESDK"

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

    Public Enum SEAppStatus
        SE_STATUS_INIT
        SE_STATUS_NORMAL
    End Enum
    Public Declare Function SECheckProtection Lib "SESDKDummy.dll" () As Boolean
    
    Public Declare Function SEGetAppStatus Lib "SESDKDummy.dll" () As SEAppStatus

    Public Declare Sub SESetAppStatus Lib "SESDKDummy.dll" (ByVal NewStatus As SEAppStatus)

    Public Declare Sub SEGetProtectionDate Lib "SESDKDummy.dll" (ByVal pDate As SYSTEMTIME)
