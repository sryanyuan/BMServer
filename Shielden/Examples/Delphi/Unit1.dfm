object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'Form1'
  ClientHeight = 259
  ClientWidth = 313
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Button1: TButton
    Left = 104
    Top = 48
    Width = 97
    Height = 25
    Caption = 'User ID'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 104
    Top = 95
    Width = 97
    Height = 25
    Caption = 'Remarks'
    TabOrder = 1
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 104
    Top = 142
    Width = 97
    Height = 25
    Caption = 'Check Protection'
    TabOrder = 2
    OnClick = Button3Click
  end
  object Button4: TButton
    Left = 104
    Top = 184
    Width = 97
    Height = 25
    Caption = 'SEDecodeString'
    TabOrder = 3
    OnClick = Button4Click
  end
end
