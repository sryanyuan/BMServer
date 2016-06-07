VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   3060
   ClientLeft      =   120
   ClientTop       =   450
   ClientWidth     =   2655
   LinkTopic       =   "Form1"
   ScaleHeight     =   3060
   ScaleWidth      =   2655
   StartUpPosition =   3
   Begin VB.CommandButton Command3 
      Caption         =   "Check Protection"
      Height          =   495
      Left            =   360
      TabIndex        =   2
      Top             =   2040
      Width           =   1935
   End
   Begin VB.CommandButton Command2 
      Caption         =   "User Info"
      Height          =   495
      Left            =   360
      TabIndex        =   1
      Top             =   1200
      Width           =   1935
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Hardware ID"
      Height          =   495
      Left            =   360
      TabIndex        =   0
      Top             =   360
      Width           =   1935
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Command1_Click()
    Dim HWID(256) As Byte
    SELicenseSDK.SEGetHardwareIDW VarPtr(HWID(0)), 256
    MsgBox HWID
End Sub

Private Sub Command2_Click()
    Dim UserInfo As SELicenseUserInfoW
    SELicenseSDK.SEGetLicenseUserInfoW UserInfo
    MsgBox UserInfo.UserID
End Sub

Private Sub Command3_Click()
    If SESDK.SECheckProtection Then
        MsgBox "Success"
    Else
        MsgBox "Fail"
    End If
    
End Sub
