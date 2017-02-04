// SettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BackMirServer.h"
#include "SettingDlg.h"
//#include "afxdialogex.h"
#include "./GameWorld/GameWorld.h"
#include "./CMainServer/CMainServer.h"
#include "./GameWorld/GameSceneManager.h"
#include "Helper.h"
#include "../CommonModule/SimpleIni.h"

//////////////////////////////////////////////////////////////////////////
CDialog* g_pSettingDlg = NULL;


// CSettingDlg 对话框

IMPLEMENT_DYNAMIC(CSettingDlg, CDialog)

CSettingDlg::CSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingDlg::IDD, pParent)
{

}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST1, m_xListCtrl);
	DDX_Control(pDX, IDC_EDIT1, m_xEdit);
}

void CSettingDlg::OnCancel()
{
	DestroyWindow();
}

void CSettingDlg::OnOK()
{
	//	Nothing
}

void CSettingDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	delete this;
	g_pSettingDlg = NULL;
}

BOOL CSettingDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_ESCAPE ||
			pMsg->wParam == VK_RETURN)
		{
			return FALSE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CSettingDlg::OnInitDialog()
{
	BOOL bRet = CDialog::OnInitDialog();

	CListCtrl* pListCtrl = static_cast<CListCtrl*>(GetDlgItem(IDC_LIST1));

	if(NULL != pListCtrl)
	{
		LONG lStyle; 
		lStyle = GetWindowLong(pListCtrl->GetSafeHwnd(), GWL_STYLE);// 获取当前窗口style 
		lStyle &= ~LVS_TYPEMASK; // 清除显示方式位 
		lStyle |= LVS_REPORT; // 设置style 
		SetWindowLong(pListCtrl->GetSafeHwnd(), GWL_STYLE, lStyle);// 设置style 
		DWORD dwStyle = pListCtrl->GetExtendedStyle(); 
		dwStyle |= LVS_EX_FULLROWSELECT;// 选中某行使整行高亮（只适用与report 风格的listctrl ） 
		dwStyle |= LVS_EX_GRIDLINES;// 网格线（只适用与report 风格的listctrl ） 
		pListCtrl->SetExtendedStyle(dwStyle);

		m_xListCtrl.InsertColumn(0, "玩家昵称", LVCFMT_CENTER, 100);
		m_xListCtrl.InsertColumn(1, "玩家IP", LVCFMT_CENTER, 150);
		m_xListCtrl.InsertColumn(2, "所在地图", LVCFMT_CENTER, 200);
		m_xListCtrl.InsertColumn(3, "延迟", LVCFMT_CENTER, 80);

		OnBnClickedButton1();
	}

	return bRet;
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CSettingDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSettingDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CSettingDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CSettingDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CSettingDlg 消息处理程序


void CSettingDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	//	刷新
	/*DelayedProcess dp;
	dp.uOp = DP_GETUSERINFO;
	GameWorld::GetInstance().AddDelayedProcess(&dp);*/
	m_xOnlineUserInfoList.clear();
	m_xListCtrl.DeleteAllItems();

	OnlineUserInfo info;
	char szIP[20];

	char szPath[MAX_PATH];
	sprintf(szPath, "%s\\Config\\map.ini",
		GetRootPath());
	CSimpleIniA xIniFile;
	xIniFile.LoadFile(szPath);

	for(int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if(g_pxHeros[i] != NULL)
		{
			ZeroMemory(&info, sizeof(info));

			info.dwCnnIndex = i;
			CMainServer::GetInstance()->GetEngine()->GetUserAddress(i, szIP, &info.uPort);
			info.xIP = szIP;
			ObjectValid::GetItemName(&(g_pxHeros[i]->GetUserData()->stAttrib), szIP);
			info.xName = szIP;

			int nMapID = g_pxHeros[i]->GetUserData()->wMapID;
			GameScene* pScene = g_pxHeros[i]->GetLocateScene();
			if (NULL == pScene)
			{
				continue;
			}
			nMapID = pScene->GetMapID();
			int nMapResID = pScene->GetMapResID();
			itoa(nMapID, szIP, 10);
			//const char* pszMapName = xIniFile.GetValue("MapNameInfo", szIP);
			const char* pszMapName = GameSceneManager::GetInstance()->GetRunMap(nMapID);
			const char* pszChMapName = xIniFile.GetValue("MapNameInfo", pszMapName);
			const LuaMapInfo* pMapInfo = GameSceneManager::GetInstance()->GetMapConfigManager().GetLuaMapInfo(nMapResID);

			if (NULL == pMapInfo)
			{
				continue;
			}

			info.xMap = pMapInfo->szMapChName;
			info.xMap += " ID:";
			info.xMap += szIP;

			itoa(nMapResID, szIP, 10);
			info.xMap += " RESID:";
			info.xMap += szIP;

			info.dwDelay = g_pxHeros[i]->GetServerNetDelay();

			m_xOnlineUserInfoList.push_back(info);
		}
	}

	if(!m_xOnlineUserInfoList.empty())
	{
		OnlineUserInfoList::const_iterator begIter = m_xOnlineUserInfoList.begin();
		OnlineUserInfoList::const_iterator endIter = m_xOnlineUserInfoList.end();
		char szText[13];

		for(begIter;
			begIter != endIter;
			++begIter)
		{
			int nRow = m_xListCtrl.InsertItem(0, begIter->xName.c_str());
			m_xListCtrl.SetItemText(nRow, 1, begIter->xIP.c_str());
			m_xListCtrl.SetItemText(nRow, 2, begIter->xMap.c_str());
			sprintf(szText, "%d ms", begIter->dwDelay);
			m_xListCtrl.SetItemText(nRow, 3, szText);
		}
	}
}


void CSettingDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	//	踢出
	int nSelIndex = -1;
	DWORD dwCnnIndex = 0xFFFFFFFF;

	for(int i = 0; i < m_xListCtrl.GetItemCount(); ++i)
	{
		if(m_xListCtrl.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
		{
			nSelIndex = i;
			break;
		}
	}

	if(nSelIndex != -1)
	{
		char szBuf[100];
		LVITEM item;
		item.iItem = nSelIndex;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = szBuf;
		item.cchTextMax = 100;

		if(m_xListCtrl.GetItem(&item))
		{
			//	Find the connection index
			OnlineUserInfoList::const_iterator begIter = m_xOnlineUserInfoList.begin();
			OnlineUserInfoList::const_iterator endIter = m_xOnlineUserInfoList.end();

			for(begIter;
				begIter != endIter;
				++begIter)
			{
				++nSelIndex;

				if(begIter->xName == szBuf)
				{
					dwCnnIndex = begIter->dwCnnIndex;
					break;
				}
			}

			//	Kick out
			if(dwCnnIndex != 0xFFFFFFFF)
			{
				CMainServer::GetInstance()->GetEngine()->CompulsiveDisconnectUser(dwCnnIndex);
			}
		}
	}
}


void CSettingDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	//	发送公告
	static DWORD dwLastSendTime = 0;
	if(GetTickCount() - dwLastSendTime < 1000)
	{
		AfxMessageBox("您的发言太快");
		return;
	}

	dwLastSendTime = GetTickCount();

	char* pMsg = NULL;
	CString xWndText;

	m_xEdit.GetWindowText(xWndText);

	if(xWndText.GetLength() != 0)
	{
		pMsg = new char[xWndText.GetLength() + 1];
		strcpy(pMsg, xWndText);
	}
	DelayedProcess dp;
	dp.uOp = DP_SENDSYSMSG;
	dp.uParam0 = (unsigned int)pMsg;
	GameWorld::GetInstance().AddDelayedProcess(&dp);
}

void CSettingDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	//	踢出所有
	//	Find the connection index
	OnlineUserInfoList::const_iterator begIter = m_xOnlineUserInfoList.begin();
	OnlineUserInfoList::const_iterator endIter = m_xOnlineUserInfoList.end();

	for(begIter;
		begIter != endIter;
		++begIter)
	{
		DWORD dwCnnIndex = begIter->dwCnnIndex;
		CMainServer::GetInstance()->GetEngine()->CompulsiveDisconnectUser(dwCnnIndex);

		Sleep(1);
	}
}
