#pragma once

#include <string>
using std::string;

// RegisterGameRoomDlg 对话框

class RegisterGameRoomDlg : public CDialog
{
	DECLARE_DYNAMIC(RegisterGameRoomDlg)

public:
	RegisterGameRoomDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~RegisterGameRoomDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnOK();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	CString m_xEditHostNameStr;
	CString m_xEditHostPortStr;
	CString m_xEditHostPasswordStr;
	CString m_xEditHostIPStr;
};
