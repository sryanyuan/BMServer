//#pragma once
#ifndef _INC_SERVERDLG
#define _INC_SERVERDLG

//////////////////////////////////////////////////////////////////////////
#include <list>
#include <afxwin.h>
#include <string>
#include <mutex>
#include "../Interface/ServerShell.h"
#include "resource.h"
#include "../common/cmsg.h"
// CServerDlg 对话框
class CMainServer;
class HeroObject;

extern HWND g_hServerDlg;

#define TIMER_DELETEPLAYERS		1
#define TIMER_REGISTERGS		2
#define TIMER_UPDATERUNNINGTIME 3
#define TIMER_MSGBOARD			4
//////////////////////////////////////////////////////////////////////////
using MsgBoardTextList = std::list < std::string > ;

class CServerDlg : public CDialog, public ServerShell
{
	DECLARE_DYNAMIC(CServerDlg)

public:
	CServerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CServerDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG1 };

public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// Override ServerShell
	virtual void AddInformation(const char* _szText) {
		m_xMsgBoardMu.lock();
		m_xMsgBoardTextList.emplace_back(_szText);
		m_xMsgBoardMu.unlock();
	}
	virtual const char* GetConfig(const char* _pszKey);
	virtual const char* GetRootPath();
	virtual void UpdateServerState(const ServerState* _pState);
	virtual const ServerBaseInfo* GetServerBaseInfo();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct); 
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	afx_msg void OnBnStartClicked();
	afx_msg LRESULT OnAddInformation(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateDialogInfo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRemoveHeroObject(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdatePlayerCount(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateDistinctIP(WPARAM wParam, LPARAM lParam);

	afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
	void AutoRun();
	void RegisterGameRoom();

	void AddTextToMessageBoardFmt(const char* fmt, ...);
	void AddTextToMessageBoard(const char* fmt);

protected:
	CMainServer* m_pxMainServer;

private:
	HBRUSH m_hListBkBrush;
	BOOL m_bInitNetwork;
	BOOL m_bVersionOK;

	unsigned int m_dwServerStartTime;

	int m_nGameRoomServerID;
	int m_nOnlinePlayerCount;
	std::string m_xRegisterGameRoomUrl;
	std::string m_xRegisterGameRoomName;
	std::string m_xRegisterGameRoomPassword;
	std::string m_xRegisterGameRoomIp;
	std::string m_xRegisterGameRoomPort;

	MsgBoardTextList m_xMsgBoardTextList;
	std::mutex m_xMsgBoardMu;

	ServerBaseInfo m_stServerBaseInfo;

public:
	void OnRegisterGsResult(const char *_pData, size_t _uLen);
	void OnRemoveGsResult(const char *_pData, size_t _uLen);

public:
	afx_msg void OnBnStopClicked();
	afx_msg void OnBnClickedButton3();
	afx_msg LRESULT OnUserMessage(WPARAM _wParam, LPARAM _lParam);
	afx_msg LRESULT OnCloseConnection(WPARAM _wParam, LPARAM _lParam);
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
};

#endif