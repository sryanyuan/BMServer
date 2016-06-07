// RegisterGameRoomDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BackMirServer.h"
#include "RegisterGameRoomDlg.h"

// RegisterGameRoomDlg 对话框

IMPLEMENT_DYNAMIC(RegisterGameRoomDlg, CDialog)

RegisterGameRoomDlg::RegisterGameRoomDlg(CWnd* pParent /*=NULL*/)
	: CDialog(RegisterGameRoomDlg::IDD, pParent)
{

}

RegisterGameRoomDlg::~RegisterGameRoomDlg()
{
}

void RegisterGameRoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, m_xEditHostNameStr);
	DDX_Text(pDX, IDC_EDIT2, m_xEditHostIPStr);
	DDX_Text(pDX, IDC_EDIT3, m_xEditHostPortStr);
	DDX_Text(pDX, IDC_EDIT4, m_xEditHostPasswordStr);
}

BOOL RegisterGameRoomDlg::OnInitDialog()
{
	BOOL bRet = __super::OnInitDialog();

	if(bRet)
	{
		GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT3)->EnableWindow(FALSE);

		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT1);
		pEdit->SetLimitText(15);
		pEdit = (CEdit*)GetDlgItem(IDC_EDIT4);
		pEdit->SetLimitText(15);
	}

	return bRet;
}

void RegisterGameRoomDlg::OnOK()
{
	UpdateData();

	if(m_xEditHostNameStr.IsEmpty())
	{
		MessageBox("请输入主机名称", "错误", MB_ICONERROR | MB_OK);
		return;
	}

	__super::OnOK();
}


BEGIN_MESSAGE_MAP(RegisterGameRoomDlg, CDialog)
END_MESSAGE_MAP()


// RegisterGameRoomDlg 消息处理程序
