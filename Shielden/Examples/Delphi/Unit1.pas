unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, SESDK, SELicenseSDK;

type
  TForm1 = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    Button4: TButton;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

procedure TForm1.Button1Click(Sender: TObject);
var Info: sSELicenseUserInfoA;
begin

  {$I SE_PROTECT_START.inc}
   SEGetLicenseUserInfoA(Info);
   ShowMessage(Info.UserID);
  {$I SE_PROTECT_END.inc}
end;

procedure TForm1.Button2Click(Sender: TObject);
var Info: sSELicenseUserInfoW;
begin
  {$I SE_PROTECT_START.inc}
   SEGetLicenseUserInfoW(Info);
   ShowMessage(Info.Remarks);
  {$I SE_PROTECT_END.inc}
end;

procedure TForm1.Button3Click(Sender: TObject);
begin                     
  {$I SE_PROTECT_START.inc}
  if SECheckProtection() then
    ShowMessage('Success.')
  else
    ShowMessage('Failed.');
  {$I SE_PROTECT_END.inc}
end;

procedure TForm1.Button4Click(Sender: TObject);
begin
  ShowMessage(SEDecodeStringA('Ansi String'));
  ShowMessage(SEDecodeStringW('Wide String'));
end;

end.
