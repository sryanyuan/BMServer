Imports System.Text
Imports System.Runtime.InteropServices

Public Class SESDK
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
    Public Enum SEAppStatus
        SE_STATUS_INIT
        SE_STATUS_NORMAL
    End Enum
    <DllImport("SESDKDummy.dll", EntryPoint:="SECheckProtection", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SECheckProtection() As Boolean
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetAppStatus", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEGetAppStatus() As SEAppStatus
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SESetAppStatus", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Sub SESetAppStatus(ByVal NewStatus As SEAppStatus)
    End Sub

    <DllImport("SESDKDummy.dll", EntryPoint:="SEGetProtectionDate", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Sub SEGetProtectionDate(<Out()> ByVal pDate As SYSTEMTIME)
    End Sub

    <DllImport("SESDKDummy.dll", EntryPoint:="SEAddMemoryGuard", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEAddMemoryGuard(ByVal pData As Long, ByVal ulSize As ULong) As Boolean
    End Function

    <DllImport("SESDKDummy.dll", EntryPoint:="SEDelMemoryGuard", CallingConvention:=CallingConvention.StdCall)> _
    Public Shared Function SEDelMemoryGuard(ByVal pData As Long, ByVal ulSize As ULong) As Boolean
    End Function
End Class