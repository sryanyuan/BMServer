// CpuQueryDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BackMirServer.h"
#include "CpuQueryDlg.h"


// CCpuQueryDlg 对话框

IMPLEMENT_DYNAMIC(CCpuQueryDlg, CDialog)

CCpuQueryDlg::CCpuQueryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCpuQueryDlg::IDD, pParent)
{
	m_bUseHTTech = false;
}

CCpuQueryDlg::~CCpuQueryDlg()
{
}

void CCpuQueryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCpuQueryDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CCpuQueryDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCpuQueryDlg 消息处理程序

void CCpuQueryDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bUseHTTech = true;
	OnOK();
}
