// BackMirServer.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "../common/glog.h"

#include "Helper.h"
#include "BackMirServer.h"
#include "MainFrm.h"

#include "../CMainServer/CMainServer.h"
#include "serverdlg.h"
#include "runarg.h"
#include <direct.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#include <vld.h>
#endif


// CBackMirServerApp

BEGIN_MESSAGE_MAP(CBackMirServerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CBackMirServerApp::OnAppAbout)
END_MESSAGE_MAP()


// CBackMirServerApp 构造

CBackMirServerApp::CBackMirServerApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CBackMirServerApp 对象

CBackMirServerApp theApp;

int CreateGameDirs()
{
	char szRoot[MAX_PATH];
	GetRootPath(szRoot, MAX_PATH);

	char szDir[MAX_PATH];
	sprintf(szDir, "%s/conf", szRoot);

	mkdir(szDir);

	return 0;
}

// CBackMirServerApp 初始化
static bool VerifyRunArg() {
	return true;
}

BOOL CBackMirServerApp::InitInstance()
{
#ifdef _DELAY_LOAD_DLL
	// set dll directory
	const char* pszAppPath = GetRootPath();
	char szDLLDir[MAX_PATH];
	strcpy(szDLLDir, pszAppPath);
#ifdef _DEBUG
	strcat_s(szDLLDir, "\\deps_d\\");
#else
	strcat_s(szDLLDir, "\\deps\\");
#endif
	if (TRUE != SetDllDirectory(szDLLDir))
	{
		::MessageBox(NULL, "无法初始化DLL模块", "错误", MB_ICONERROR | MB_OK);
		return FALSE;
	}
#endif
	CreateGameDirs();

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	//	初始化启动参数
	if (!InitRunArg())
	{
		::MessageBox(NULL, "初始化启动参数失败", "错误", MB_ICONERROR);
		return FALSE;
	}
	CServerDlg dlg;
	dlg.DoModal();

	google::FlushLogFiles(google::GLOG_INFO);
	google::ShutdownGoogleLogging();

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return TRUE;
}


// CBackMirServerApp 消息处理程序




// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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

// 用于运行对话框的应用程序命令
void CBackMirServerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CBackMirServerApp 消息处理程序

