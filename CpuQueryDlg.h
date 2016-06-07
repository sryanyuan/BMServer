#pragma once


// CCpuQueryDlg 对话框

class CCpuQueryDlg : public CDialog
{
	DECLARE_DYNAMIC(CCpuQueryDlg)

public:
	CCpuQueryDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCpuQueryDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG5 };

	inline bool GetUseHTTech()
	{
		return m_bUseHTTech;
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	bool m_bUseHTTech;
public:
	afx_msg void OnBnClickedOk();
};
