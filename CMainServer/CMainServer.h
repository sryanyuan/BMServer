#ifndef CMAINSERVER_H_
#define CMAINSERVER_H_
#if _MSC_VER == 1800
#include <set>
#include "../IOServer/netbase.h"
#else
#include "netbase.h"
#endif
#include "../CMainServer/CUser.h"
#include "../../CommonModule/ByteBuffer.h"
#include "../../CommonModule/GamePacket.h"
#include "../GameWorld/WatcherThread.h"
#include "../common/shared.h"
#include <string>

class CNetBase;
struct ServerState;
struct PacketBase;
struct PacketHeader;
class GameObject;
//////////////////////////////////////////////////////////////////////////
typedef std::map<DWORD, DWORD> Index2UserIDMap;
//////////////////////////////////////////////////////////////////////////
#define MODE_STOP		0
#define MODE_RUNNING	1

#define NEW_USER_LOGIN(VERSION, HERO)\
	if(LoadHumData##VERSION(HERO, g_xMainBuffer))\
{\
	DelayedProcess dp;\
	dp.uOp = DP_USERLOGIN;\
	dp.uParam0 = (unsigned int)pObj;\
	dp.uParam1 = 1;\
	GameWorld::GetInstance().AddDelayedProcess(&dp);\
	++m_dwUserNumber;\
	UpdateServerState();\
	sprintf(szText, "%s[%s]", s_pszUserLogin, req.stHeader.szName);\
	AddInfomation(szText);\
}\
	else\
	{\
		sprintf(szText, "%s[%d]", s_pszInvalid, uVersion);\
		AddInfomation(szText);\
		SAFE_DELETE(HERO);\
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";\
		return false;\
	}\
}

enum NetThreadEventType
{
	kNetThreadEvent_None = 0,
	kNetThreadEvent_SmallQuit,
};

struct NetThreadEvent
{
	int nEventId;
	int nArg;
};
typedef std::list<NetThreadEvent> NetThreadEventList;
//////////////////////////////////////////////////////////////////////////

class HeroObject;

class CMainServer
{
public:
	CMainServer();
	virtual ~CMainServer();

public:
	static CMainServer* GetInstance()
	{
		static CMainServer s_xServer;
		
		return &s_xServer;
	}

public:
	bool InitNetWork();
	bool StartServer(char* _szIP, WORD _wPort);
	void StopServer();
	void StopEngine();
	void WaitForStopEngine();
	bool InitDatabase();
	bool InitCRCThread();

	void SendPacket(DWORD _dwIndex, PacketBase* _pPacket);
	CNetbase* GetEngine();
	void InsertUserConnectionMapKey(WPARAM _wParam, LPARAM _lParam);

	inline GAME_MODE GetServerMode()						{return m_eMode;}
	inline void SetServerMode(GAME_MODE _eMode)				{m_eMode = _eMode;}
	inline void SetLoginAddr(const std::string& _xLoginAddr)		{m_xLoginAddr = _xLoginAddr;}
	inline DWORD GetLSConnIndex()							{return m_dwLsConnIndex;}

	inline void IncOnlineUsers()							{++m_dwUserNumber;}
	inline void DecOnlineUsers()							{if(m_dwUserNumber > 0) --m_dwUserNumber;}

	inline void SetAppException()							{m_bAppException = true;}
	inline bool GetAppException()							{return m_bAppException;}

	inline bool IsLoginServerMode()							{return m_eMode == GM_LOGIN;}

	void UpdateServerState();

	void ForceCloseConnection(DWORD _dwIndex);

	inline WORD GetListenPort()								{return m_dwListenPort;}
	inline std::string& GetListenIP()						{return m_xListenIP;}

	void SendNetThreadEvent(const NetThreadEvent& _refEvt);
	void ProcessNetThreadEvent();

	inline int GetNetEngineVersion()						{return m_nNetEngineVersion;}

public:
	inline void SetRunningMode(BYTE _bMode)					{m_bMode = _bMode;}
	inline BYTE GetRunningMode()							{return m_bMode;}
	inline DWORD GetMainThreadID()							{return m_dwThreadID;} 

public:
	void OnAcceptUser(DWORD _dwIndex);
	void OnDisconnectUser(DWORD _dwIndex);
	void OnRecvFromUserTCP(DWORD _dwIndex, ByteBuffer* _xBuf);
	void OnRecvFromServerTCP(DWORD _dwIndex, ByteBuffer* _xBuf);
	void OnRecvFromServerTCPProtobuf(DWORD _dwIndex, ByteBuffer* _xBuf);

protected:
	//	玩家连接
	static void STDCALL _OnAcceptUser(DWORD _dwIndex);
	//	玩家离开
	static void STDCALL _OnDisconnectUser(DWORD _dwIndex);
	//	玩家发送数据包
	static void STDCALL _OnRecvFromUserTCP(DWORD _dwIndex, char* _pMsg, DWORD _dwLen);
	//	登陆服务器连接成功
	static void STDCALL _OnLsConnSuccess(DWORD _dwIndex, void* _pParam);
	static void STDCALL _OnLsConnFailed(DWORD _dwIndex, void* _pParam);
	static void STDCALL _OnRecvFromServerTCP(DWORD _dwIndex, char* _pMsg, DWORD _dwLen);
	static void STDCALL _OnAcceptServer(DWORD _dwIndex);
	static void STDCALL _OnDisconnectServer(DWORD _dwIndex);
	//	循环时间
	static void STDCALL _OnGameLoop(DWORD _dwEvtIndex);

private:
	bool InitLogFile();
	void MakeServerState(ServerState* _pState);
	bool CheckUserValid(GameObject* _pObj);
	bool ConnectToLoginSvr();

	bool OnPreProcessPacket(DWORD _dwIndex, DWORD _dwLSIndex, DWORD _dwUID, const char* _pExtendInfo, PkgUserLoginReq& req);
	bool LoadHumData110(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData111(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData112(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData113(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData114(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData115(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData116(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData117(HeroObject* _pHero, ByteBuffer& _xBuf);

	bool LoadHumData200(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData201(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData202(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData203(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData204(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData205(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData206(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData207(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData208(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData210(HeroObject* _pHero, ByteBuffer& _xBuf);

protected:
	CNetbase* m_pxServer;
	PlayerMap m_xPlayers;

	//	服务器运行模式
	BYTE m_bMode;
	DWORD m_dwUserNumber;

	DWORD m_dwThreadID;

	//	CRC检测线程
	WatcherThread* m_pWatcherThread;

	//	服务器游戏模式
	GAME_MODE m_eMode;
	bool m_bLoginConnected;
	std::string m_xLoginAddr;
	DWORD m_dwLsConnIndex;
	bool m_bAppException;

	//	服务器的监听端口
	WORD m_dwListenPort;
	std::string m_xListenIP;

	//	CPU是否开启了HT
	bool m_bUseHTTech;

	// Distinct ip set
	std::map<std::string, int> m_xIPs;

	//	NetThreadEvent processed by timer loop
	NetThreadEventList m_xNetThreadEventList;
	CRITICAL_SECTION m_csNetThreadEventList;

	int m_nNetEngineVersion;

public:
	HWND m_hDlgHwnd;
};

#endif