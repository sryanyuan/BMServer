// ServerDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "BackMirServer.h"
#include "ServerDlg.h"
#include "RegisterGameRoomDlg.h"
#include "IOServer/SServerEngine.h"
#include "CMainServer/CMainServer.h"
#include "Helper.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <Shlwapi.h>
#include "./GameWorld/struct.h"
#include "./GameWorld/GameWorld.h"
#include "./GameWorld/ObjectEngine.h"
#include "./GameWorld/ExceptionHandler.h"
#include "SettingDlg.h"
#include "../CommonModule/CommandLineHelper.h"
#include "../CommonModule/SimpleIni.h"
#include "../CommonModule/BMHttpManager.h"
#include "../CommonModule/cJSON.h"
#include "ConfigDlg.h"
#include "runarg.h"
#include "../CommonModule/version.h"

using std::string;
//////////////////////////////////////////////////////////////////////////
void SetRandomTitle(HWND _hWnd)
{
	if (CMainServer::GetInstance()->GetServerMode() == GM_LOGIN) {
		char szTitle[250];
		szTitle[0] = 0;
		sprintf(szTitle, "ServerID[%d] ServerName[%s]", GetServerID(), GetRunArg("servername"));
		SetWindowText(_hWnd, szTitle);
		return;
	}
	char str[]="ABCDEFGHIJKLMHOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	char szCaption[256]={0};
	INT i,leg;
	srand((unsigned)time(NULL));	//让每次产生的随机数不同

	for(i=0;i<rand()%4+10;i++){
		//标题长度由rand()%4+10控制,长度为10或11、12、13、14
		leg = rand()%strlen(str);
		szCaption[i]=str[leg];	//给标题赋值
	}

	SetWindowText(_hWnd, szCaption);
}

string GBKToUTF8(const std::string& strGBK)  
{  
	string strOutUTF8 = "";  
	WCHAR * str1;  
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);  
	str1 = new WCHAR[n];  
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);  
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);  
	char * str2 = new char[n];  
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);  
	strOutUTF8 = str2;  
	delete[]str1;  
	str1 = NULL;  
	delete[]str2;  
	str2 = NULL;  
	return strOutUTF8;  
}  
//////////////////////////////////////////////////////////////////////////
HWND g_hServerDlg = NULL;
HeroObjectList g_xWaitDeleteHeros;
// CServerDlg 对话框
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CServerDlg, CDialog)

CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerDlg::IDD, pParent)
{
	//	Generate the root path
	GetRootPath();
	m_pxMainServer = CMainServer::GetInstance();
	m_pxMainServer->SetServerShell(this);
	g_hServerDlg = GetSafeHwnd();
	m_hListBkBrush = ::CreateSolidBrush(RGB(0, 0, 0));
	m_bInitNetwork = FALSE;
	m_bVersionOK = FALSE;
	m_nGameRoomServerID = 0;
	m_nOnlinePlayerCount = 0;
	m_dwServerStartTime = 0;
}

CServerDlg::~CServerDlg()
{
	::DeleteObject(m_hListBkBrush);
	if(NULL != m_pxMainServer)
	{
		//delete m_pxMainServer;
		//m_pxMainServer = NULL;
	}
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


//////////////////////////////////////////////////////////////////////////
BOOL CServerDlg::OnInitDialog()
{
#ifdef _DEBUG
	//AfxMessageBox("DEBUG");
#endif
	g_xConsole.InitConsole();

	g_hServerDlg = GetSafeHwnd();
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	m_bInitNetwork = m_pxMainServer->InitNetWork() ? TRUE : FALSE;
	if(m_bInitNetwork)
	{
		AddTextToMessageBoardFmt("初始化服务器成功");
		if(m_pxMainServer->InitDatabase())
		{
			m_bVersionOK = TRUE;
			AutoRun();
		}
		else
		{
			AddTextToMessageBoardFmt("初始化服务器失败");
			LOG(ERROR) << "初始化服务器失败";
		}
	}
	else
	{
		AddTextToMessageBoardFmt("初始化服务器失败");
		LOG(FATAL) << "初始化服务器失败";
	}

	//	设置异常处理
	SetUnhandledExceptionFilter(&BM_UnhandledExceptionFilter);
	//	定时删除等待删除的玩家
	SetTimer(TIMER_DELETEPLAYERS, 5000, NULL);
	// Initialize message board timer check
	SetTimer(TIMER_MSGBOARD, 50, NULL);

	//	生成CRC信息
	if(!m_pxMainServer->InitCRCThread())
	{

	}

	SetRandomTitle(GetSafeHwnd());

#ifndef _DEBUG
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
#endif

	return __super::OnInitDialog();
}


//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CServerDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CServerDlg::OnBnStartClicked)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_HELPER_ADDINFO, &CServerDlg::OnAddInformation)
	ON_MESSAGE(WM_UPDATE_INFO, &CServerDlg::OnUpdateDialogInfo)
	ON_BN_CLICKED(IDC_BUTTON2, &CServerDlg::OnBnStopClicked)
	ON_BN_CLICKED(IDC_BUTTON3, &CServerDlg::OnBnClickedButton3)
	ON_MESSAGE(WM_INSERTMAPKEY, &CServerDlg::OnUserMessage)
	ON_MESSAGE(WM_CLOSECONNECTION, &CServerDlg::OnCloseConnection)
	ON_MESSAGE(WM_REMOVEPLAYER, &CServerDlg::OnRemoveHeroObject)
	ON_MESSAGE(WM_PLAYERCOUNT, &CServerDlg::OnUpdatePlayerCount)
	ON_MESSAGE(WM_DISTINCTIP, &CServerDlg::OnUpdateDistinctIP)
	ON_BN_CLICKED(IDC_BUTTON4, &CServerDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CServerDlg::OnBnClickedButton5)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON6, &CServerDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CServerDlg::OnBnClickedButton7)
END_MESSAGE_MAP()


// CServerDlg 消息处理程序

/************************************************************************/
/* 创建窗体
/************************************************************************/
int CServerDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//lpCreateStruct->style |= WS_OVERLAPPEDWINDOW;
	return  __super::OnCreate(lpCreateStruct);
}

void CServerDlg::OnDestroy()
{
	if(0 != m_nGameRoomServerID)
	{
		char szUrl[MAX_PATH] = {0};
		sprintf(szUrl, "%s/removegs?id=%d", m_xRegisterGameRoomUrl.c_str(), m_nGameRoomServerID);
		BMHttpManager::GetInstance()->DoGetRequestSync(szUrl, fastdelegate::bind(&CServerDlg::OnRemoveGsResult, this));
	}

	__super::OnDestroy();
}

/************************************************************************/
/* 窗体拉伸
/************************************************************************/
void CServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CListBox* pxList = static_cast<CListBox*>(GetDlgItem(IDC_LIST1));
	CHECK(pxList != NULL);
	//pxList->MoveWindow(10, 10, cx - 20, cy - 10 - 50);
}

/************************************************************************/
/* 设置ListBox背景颜色
/************************************************************************/
HBRUSH CServerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch(nCtlColor)
	{
	case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(RGB(0, 255, 0));
			pDC->SetBkMode(TRANSPARENT);
			return m_hListBkBrush;
		}break;
	default:
		{
			return __super::OnCtlColor(pDC, pWnd, nCtlColor);
		}break;
	}
}

/************************************************************************/
/* 开启服务器
/************************************************************************/
void CServerDlg::OnBnStartClicked()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_bInitNetwork)
	{
		AddTextToMessageBoardFmt("无法启动服务器");
		return;
	}

	if(!m_bVersionOK)
	{
		AddTextToMessageBoardFmt("无法启动服务器");
		return;
	}

	const char* szCfgFile = RunArgGetConfigFile();

	if(!PathFileExists(szCfgFile))
	{
		AddTextToMessageBoardFmt("无法读取服务器配置信息 (%s)", szCfgFile);
		LOG(INFO) << "无法读取服务器配置信息:" << szCfgFile;
		return;
	}

	//	读取配置
	char szValue[50];
	::GetPrivateProfileString("SERVER", "IP", "", szValue, sizeof(szValue), szCfgFile);
	if(strlen(szValue) == 0)
	{
		LOG(WARNING) << "配置文件IP读取值为空";
		return;
	}
	WORD wPort = 0;
	wPort = ::GetPrivateProfileInt("SERVER", "PORT", 0, szCfgFile);
	if(wPort == 0)
	{
		LOG(WARNING) << "配置文件PORT读取值为空";
		return;
	}

	if(m_pxMainServer->StartServer(szValue, wPort))
	{
		AddTextToMessageBoardFmt("服务器启动成功，开始监听");
		GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
		char szIpPort[100];
		sprintf(szIpPort, "服务器IP:%s 端口:%d",
			szValue, wPort);
		GetDlgItem(IDC_IPPORT)->SetWindowText(szIpPort);
		m_dwServerStartTime = GetTickCount();
		SetTimer(TIMER_UPDATERUNNINGTIME, 1 * 1000, NULL);
	}
	else
	{
		AddTextToMessageBoardFmt("服务器启动失败");
		LOG(ERROR) << "服务器启动失败 IP[" << szValue << "] PORT[" << wPort << "]";
	}
}

/************************************************************************/
/* 添加Listbox信息
/************************************************************************/
LRESULT CServerDlg::OnAddInformation(WPARAM wParam, LPARAM lParam)
{
	CListBox* pxList = static_cast<CListBox*>(GetDlgItem(IDC_LIST1));
	CHECK(pxList != NULL);

	int nIndex = pxList->GetCount();
	if(nIndex == LB_ERR)
	{
		return S_FALSE;
	}

	if(nIndex > 50)
	{
		pxList->ResetContent();
	}

	const char* pText = (const char*)wParam;
	pxList->InsertString(-1, pText);
	pxList->SetTopIndex(pxList->GetCount() - 1);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CServerDlg::AutoRun()
{
	//CommandLineHelper xHelper;
	//if(xHelper.InitParam())
	{
		const char* pszValue = GetRunArg("loginsvr");
		if(pszValue != NULL)
		{
			CMainServer::GetInstance()->SetServerMode(GM_LOGIN);
			string xLoginAddr = pszValue;
			CMainServer::GetInstance()->SetLoginAddr(xLoginAddr);
		}

		pszValue = GetRunArg("listenip");
		if(pszValue != NULL)
		{
			char szIP[16];
			char szBuf[16];
			szIP[15] = 0;
			WORD wPort = 0;
			int nPortPos = 0;
			for(int i = 0; i < 16; ++i)
			{
				if(pszValue[i] == ':')
				{
					nPortPos = i;
					szBuf[i] = 0;
					break;
				}
				else
				{
					szBuf[i] = pszValue[i];
				}
			}
			strcpy(szIP, szBuf);
			++nPortPos;

			for(int i = nPortPos; ; ++i)
			{
				if(pszValue[i] == 0)
				{
					szBuf[i - nPortPos] = 0;
					break;
				}
				szBuf[i - nPortPos] = pszValue[i];
			}
			wPort = (WORD)atoi(szBuf);

			LOG(INFO) << "IP:" << szIP << ", Port:" << wPort;

			if (m_pxMainServer->StartServer(szIP, wPort))
			{
				AddTextToMessageBoardFmt("服务器启动成功，开始监听");
				GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
				GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
				char szIpPort[100];
				sprintf(szIpPort, "服务器IP:%s 端口:%d",
					szIP, wPort);
				GetDlgItem(IDC_IPPORT)->SetWindowText(szIpPort);
				m_dwServerStartTime = GetTickCount();
				SetTimer(TIMER_UPDATERUNNINGTIME, 1 * 1000, NULL);

				ShowWindow(SW_HIDE);
			}
			else
			{
				AddTextToMessageBoardFmt("服务器启动失败");
				LOG(ERROR) << "服务器启动失败 IP[" << szIP << "] PORT[" << wPort << "]";
			}
		}
	}

	return;  
}
/************************************************************************/
/* 更新显示数据
/************************************************************************/
LRESULT CServerDlg::OnUpdateDialogInfo(WPARAM wParam, LPARAM lParam)
{
	extern const char* g_szMode[2];

	ServerState* pState = (ServerState*)wParam;
	char szOutput[MAX_PATH];
	/*sprintf(szOutput, "在线人数: %d",
		pState->wOnline);
	GetDlgItem(IDC_USERSUM)->SetWindowText(szOutput);*/

	sprintf(szOutput, "运行状态: %s",
		g_szMode[pState->bMode]);
	GetDlgItem(IDC_MODE)->SetWindowText(szOutput);

	return S_OK;
}
void CServerDlg::OnBnStopClicked()
{
	// TODO: 在此添加控件通知处理程序代码
	//m_bInitNetwork = false;
	//m_pxMainServer->StopServer();
	//bool bIsPaused = GameWorld
	//GameWorld::GetInstance().Pause();

	//GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	//GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
}

LRESULT CServerDlg::OnRemoveHeroObject(WPARAM wParam, LPARAM lParam)
{
	m_pxMainServer->DecOnlineUsers();
	m_pxMainServer->UpdateServerState();
	g_xWaitDeleteHeros.push_back((HeroObject*)lParam);
	return S_OK;
}

LRESULT CServerDlg::OnUpdatePlayerCount(WPARAM wParam, LPARAM lParam)
{
	char szOutput[MAX_PATH];
	sprintf(szOutput, "在线人数: %d",
		wParam);
	m_nOnlinePlayerCount = wParam;
	GetDlgItem(IDC_USERSUM)->SetWindowText(szOutput);

	sprintf(szOutput, "处理怪物逻辑: %d",
		lParam);
	GetDlgItem(IDC_SVRINFO)->SetWindowText(szOutput);

	return S_OK;
}

LRESULT CServerDlg::OnUpdateDistinctIP(WPARAM wParam, LPARAM lParam)
{
	char szOutput[MAX_PATH];
	sprintf(szOutput, "独立IP数: %d",
		wParam);
	m_nOnlinePlayerCount = wParam;
	GetDlgItem(IDC_DISTINCTIP)->SetWindowText(szOutput);

	return S_OK;
}

void CServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == TIMER_DELETEPLAYERS)
	{
		if(!g_xWaitDeleteHeros.empty())
		{
			HeroObjectList::iterator begIter = g_xWaitDeleteHeros.begin();
			HeroObject* pHero = NULL;

			for(begIter;
				begIter != g_xWaitDeleteHeros.end();
				)
			{
				pHero = *begIter;
				bool bErased = false;

				if(pHero->GetUserIndex() > 0 &&
					pHero->GetUserIndex() <= MAX_USER_NUMBER)
				{
					if(g_pxHeros[pHero->GetUserIndex()] != pHero)
					{
						LOG(INFO) << "Delete a hang up hero object...";
						delete pHero;
						bErased = true;
					}
				}

				if(bErased)
				{
					begIter = g_xWaitDeleteHeros.erase(begIter);
				}
				else
				{
					++begIter;
				}
			}
		}
	}
	else if(nIDEvent == TIMER_REGISTERGS)
	{
		RegisterGameRoom();
	}
	else if(nIDEvent == TIMER_UPDATERUNNINGTIME)
	{
		DWORD dwMs = GetTickCount() - m_dwServerStartTime;
		int nSec = dwMs / 1000;

		char szMsg[50] = {0};

		CWnd* pLabel = GetDlgItem(IDC_RUNNINGTIME);

		if(nSec < 60)
		{
			sprintf(szMsg, "运行时间: %d秒", nSec);
		}
		else if(nSec < 60 * 60)
		{
			int nMin = nSec / 60;
			int nSecLeft = nSec - nMin * 60;
			sprintf(szMsg, "运行时间: %d分 %d秒", nMin, nSecLeft);
		}
		else
		{
			int nHour = nSec / 60 / 60;
			int nMin = (nSec - nHour * 60 * 60) / 60;
			int nSecLeft = nSec - nHour * 3600 - nMin * 60;
			sprintf(szMsg, "运行时间: %d小时 %d分", nHour, nMin);
		}
		pLabel->SetWindowText(szMsg);
	}
	else if (nIDEvent == TIMER_MSGBOARD) {
		m_xMsgBoardMu.lock();
		// Apply message board texts
		for (auto &text : m_xMsgBoardTextList) {
			AddTextToMessageBoard(text.c_str());
		}

		m_xMsgBoardTextList.clear();
		m_xMsgBoardMu.unlock();
	}
}

void CServerDlg::AddTextToMessageBoard(const char* fmt) {
	CListBox* pxList = static_cast<CListBox*>(GetDlgItem(IDC_LIST1));
	CHECK(pxList != NULL);

	int nIndex = pxList->GetCount();
	if (nIndex == LB_ERR)
	{
		return;
	}

	if (nIndex > 50)
	{
		pxList->ResetContent();
	}

	pxList->InsertString(-1, fmt);
	pxList->SetTopIndex(pxList->GetCount() - 1);
}

void CServerDlg::AddTextToMessageBoardFmt(const char* fmt, ...) {
	char buffer[MAX_PATH];

	va_list args;
	va_start(args, fmt);
	_vsnprintf(buffer, sizeof(buffer), fmt, args);

	SYSTEMTIME lpTime;
	GetLocalTime(&lpTime);

	char logTime[MAX_PATH];
	sprintf(logTime, "[%d-%d-%d %02d:%02d:%02d] ", lpTime.wYear, lpTime.wMonth, lpTime.wDay, lpTime.wHour, lpTime.wMinute, lpTime.wSecond);
	strcat(logTime, buffer);

	AddTextToMessageBoard(logTime);
}

void CServerDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	google::FlushLogFiles(google::GLOG_INFO);
}

LRESULT CServerDlg::OnUserMessage(WPARAM _wParam, LPARAM _lParam)
{
	return 1;
}

LRESULT CServerDlg::OnCloseConnection(WPARAM _wParam, LPARAM _lParam)
{
	LOG(INFO) << "Force close connection[" << _wParam << "]";
	m_pxMainServer->GetIOServer()->CloseUserConnection(_wParam);
	return 1;
}

void CServerDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
#ifdef _DEBUG
	DelayedProcess dp;
	dp.uOp = DP_RELOADSCRIPT;
	GameWorld::GetInstance().AddDelayedProcess(&dp);
#endif
}


void CServerDlg::OnBnClickedButton5()
{
	RECORD_FUNCNAME_UI;
	// TODO: 在此添加控件通知处理程序代码
	//	管理游戏
	if(g_pSettingDlg == NULL)
	{
		g_pSettingDlg = new CSettingDlg(this);
		g_pSettingDlg->Create(IDD_DIALOG2, this);
		g_pSettingDlg->ShowWindow(SW_SHOW);
	}
}


void CServerDlg::OnClose()
{
	GameWorld* pWorld = GameWorld::GetInstancePtr();

	if(pWorld->GetWorldState() == WS_WORKING)
	{
		int nSelect = AfxMessageBox("服务器已运行，确认要关闭服务器吗？", MB_YESNO);
		if(IDYES == nSelect)
		{
			//	反注册游戏服务器
			if(0 != m_nGameRoomServerID)
			{
				KillTimer(TIMER_REGISTERGS);
			}

			//	等待服务器停止
			/*pWorld->Terminate(0);
			while(pWorld->GetWorldState() != WS_STOP)
			{
				Sleep(1);
			}*/

			// 等待网络引擎停止
			CMainServer::GetInstance()->WaitForStopEngine();
			
			__super::OnClose();
		}
		else
		{
			return;
		}
	}
	else
	{
		__super::OnClose();
	}
}

void CServerDlg::OnOK()
{
	//	nothing
}

void CServerDlg::OnCancel()
{
	__super::OnCancel();
}

BOOL CServerDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam)
		{
		case VK_ESCAPE:
			{
				GameWorld* pWorld = GameWorld::GetInstancePtr();

				if(pWorld->GetWorldState() == WS_WORKING)
				{
					int nSelect = AfxMessageBox("服务器已运行，确认要关闭服务器吗？", MB_YESNO);
					if(IDYES == nSelect)
					{
						//	反注册游戏服务器
						if(0 != m_nGameRoomServerID)
						{
							KillTimer(TIMER_REGISTERGS);
						}

						//	等待服务器停止
						pWorld->Terminate(0);
						while(pWorld->GetWorldState() != WS_STOP)
						{
							Sleep(1);
						}
					}
					else
					{
						return TRUE;
					}
				}
			}break;
		}
	}  

	return CDialog::PreTranslateMessage(pMsg);  
}

void CServerDlg::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	if(GameWorld::GetInstancePtr()->GetWorldState() != WS_WORKING)
	{
		AfxMessageBox("服务器还未启动，无法注册至联机大厅", MB_OK | MB_ICONERROR);
		return;
	}

	bool bIpValid = true;
	string& refListenIP = m_pxMainServer->GetListenIP();
	if(refListenIP.size() < 7)
	{
		bIpValid = false;
	}
	else
	{
		if(refListenIP[0] == '1' &&
			refListenIP[1] == '2' &&
			refListenIP[2] == '7')
		{
			bIpValid = false;
		}
	}
	if(!bIpValid)
	{
		AfxMessageBox("当前为单机模式，无法注册到联机服务器", MB_OK | MB_ICONERROR);
#ifdef _DEBUG
#else
		return;
#endif
	}

	//	获取请求的url
	char szDestPath[MAX_PATH] = {0};
	strcpy(szDestPath, GetRootPath());
	strcat(szDestPath, "/config/url_2.ini");

	if(!PathFileExists(szDestPath))
	{
		AfxMessageBox("无法获取请求地址", MB_OK | MB_ICONERROR);
		return;
	}

	CSimpleIniA xIniFile;
	if(SI_OK != xIniFile.LoadFile(szDestPath))
	{
		MessageBox("无法解析配置文件", "错误", MB_ICONERROR | MB_OK);
		return;
	}
	const char* pszUrl = xIniFile.GetValue("CONFIG", "GAMEROOMURL");
	if(NULL == pszUrl ||
		strlen(pszUrl) == 0)
	{
		MessageBox("无法解析配置文件", "错误", MB_ICONERROR | MB_OK);
		return;
	}
	m_xRegisterGameRoomUrl = pszUrl;

	RegisterGameRoomDlg dlg;
	int nDlgSel = dlg.DoModal();
	if(IDOK == nDlgSel)
	{
		m_xRegisterGameRoomName = dlg.m_xEditHostNameStr;
		m_xRegisterGameRoomPassword = dlg.m_xEditHostPasswordStr;
		m_xRegisterGameRoomIp = dlg.m_xEditHostIPStr;
		m_xRegisterGameRoomPort = dlg.m_xEditHostPortStr;

		//	将主机名转为utf8
		m_xRegisterGameRoomName = GBKToUTF8(m_xRegisterGameRoomName);

		char szPort[10] = {0};
		m_xRegisterGameRoomPort = itoa(m_pxMainServer->GetListenPort(), szPort, 10);

		RegisterGameRoom();
		//	开启定时器
		SetTimer(TIMER_REGISTERGS, 30 * 1000, NULL);
	}
}

void CServerDlg::RegisterGameRoom()
{
	char szUrl[MAX_PATH] = {0};
	sprintf(szUrl, "%s/registergs?address=%s&port=%s&note=%s&password=%s&online=%d&version=%s",
		m_xRegisterGameRoomUrl.c_str(),
		m_xRegisterGameRoomIp.c_str(),
		m_xRegisterGameRoomPort.c_str(),
		m_xRegisterGameRoomName.c_str(),
		m_xRegisterGameRoomPassword.c_str(),
		m_nOnlinePlayerCount,
		BACKMIR_CURVERSION);

	BMHttpManager::GetInstance()->DoGetRequestSync(szUrl, fastdelegate::bind(&CServerDlg::OnRegisterGsResult, this));
}

void CServerDlg::OnRegisterGsResult(const char *_pData, size_t _uLen)
{
	//	nothing
	if(NULL == _pData ||
		0 == _uLen)
	{
		if(0 != m_nGameRoomServerID)
		{
			AddTextToMessageBoardFmt("与游戏大厅服务器失去连接...");
			m_nGameRoomServerID = 0;
		}
		else
		{
			AddTextToMessageBoardFmt("注册至大厅服务器失败...");
		}
		GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE);
		return;
	}

	cJSON* pRoot = cJSON_Parse(_pData);
	if(NULL == pRoot)
	{
		return;
	}

	int nRet = cJSON_GetObjectItem(pRoot, "Result")->valueint;
	if(0 == nRet)
	{
		const char* pszServerID = cJSON_GetObjectItem(pRoot, "Msg")->valuestring;
		if(NULL != pszServerID)
		{
			if(0 == m_nGameRoomServerID)
			{
				AddTextToMessageBoardFmt("成功注册至游戏大厅");
			}
			m_nGameRoomServerID = atoi(pszServerID);
			GetDlgItem(IDC_BUTTON6)->EnableWindow(FALSE);
		}
	}
	else
	{
		const char* pszErrMsg = cJSON_GetObjectItem(pRoot, "Msg")->valuestring;
		if(NULL != pszErrMsg)
		{
			MessageBox(pszErrMsg, "错误", MB_ICONERROR | MB_OK);
		}
		m_nGameRoomServerID = 0;
		GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE);
	}

	cJSON_Delete(pRoot);
	pRoot = NULL;
}

void CServerDlg::OnRemoveGsResult(const char *_pData, size_t _uLen)
{
	//	nothing
}

void CServerDlg::OnBnClickedButton7()
{
	// TODO: 在此添加控件通知处理程序代码
	CConfigDlg dlg;
	if(IDOK == dlg.DoModal())
	{

	}
}
