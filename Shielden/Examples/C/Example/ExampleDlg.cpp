// ExampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Example.h"
#include "ExampleDlg.h"
#include "../../../C/SESDK.h"
#include "../../../C/SELicenseSDK.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CExampleDlg dialog




CExampleDlg::CExampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExampleDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CExampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CExampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CExampleDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CExampleDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CExampleDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CExampleDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CExampleDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CExampleDlg::OnBnClickedButton6)
END_MESSAGE_MAP()


// CExampleDlg message handlers

BOOL CExampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CExampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CExampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CExampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CExampleDlg::OnBnClickedButton1()
{
	SE_PROTECT_START
	SELicenseUserInfo Info = {0};
	SEGetLicenseUserInfo(&Info);
	MessageBox(Info.UserID);
	SE_PROTECT_END
	
}

void CExampleDlg::OnBnClickedButton2()
{
	SE_PROTECT_START
	SELicenseUserInfo Info = {0};
	SEGetLicenseUserInfo(&Info);
	MessageBox(Info.Remarks);
	SE_PROTECT_END
}

void CExampleDlg::OnBnClickedButton3()
{
	SE_PROTECT_START
	if(SECheckProtection())
	{
		MessageBox("Success.");
	}
	else
	{
		MessageBox("Failed.");
	}
	SE_PROTECT_END
}

#pragma optimize("", off)
void CExampleDlg::OnBnClickedButton4()
{
	MessageBox(SEDecodeStringA("Ansi String"), "TEST", 0);
	wchar_t* decoded = SEDecodeStringW(L"Wide String");
	MessageBoxW(this->GetSafeHwnd(), decoded, L"TEST", 0);
	SEFreeString(decoded);
}
#pragma optimize("", on)
void CExampleDlg::OnBnClickedButton5()
{
	char text[256];
	sprintf_s(text, "%d", SEGetNumDaysLeft());
	MessageBox(text, "Days Left", 0);
}

void CExampleDlg::OnBnClickedButton6()
{
	char text[256];
	SEGetHardwareID(text, 256);
	MessageBox(text, "Hardware ID", 0);
}
