#ifndef _INC_SSERVERENGINE_
#define _INC_SSERVERENGINE_
//////////////////////////////////////////////////////////////////////////
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <thread>
#include <process.h>
#include <string>
#include <list>
#include <stdio.h>
#include <atomic>
#include <mutex>
#include "IndexManager.h"
#include "IOConn.h"
#include "Def.h"
#include "namespace.h"

IONS_START
//////////////////////////////////////////////////////////////////////////
enum IOResultType
{
	IOResult_Ok	=	0,
	IOResult_InvalidParam,
	IOResult_CreateThreadFailed,
	IOResult_ListenFailed,
};
//////////////////////////////////////////////////////////////////////////
struct IOEvent
{
	int nEventId;
	void* pData;
	size_t uLength;
};

enum IOActionType
{
	IOAction_CloseUserConn,
	IOAction_CloseServerConn,
	IOAction_SendToUser,
	IOAction_SendToServer,
	IOAction_Connect
};

enum IOServerStatus
{
	IOServerStatus_Stop,
	IOServerStatus_Running,
};

struct IOAction
{
	unsigned short uAction;
	unsigned short uIndex;
	unsigned short uTag;
};

struct IOActionConnectContext
{
	sockaddr_in addr;
	FUNC_ONCONNECTSUCCESS fnSuccess;
	FUNC_ONCONNECTFAILED fnFailed;
	void* pArg;
};

struct IOInitDesc
{ 
	// limit the count of connection as server
	size_t uMaxConnUser;
	// limit the count of connection as client
	size_t uMaxConnServer;
	// limit the max packet length
	size_t uMaxPacketLength;
	// using iocp event mode
	bool bUseIOCP;

	//	callbacks
	FUNC_ONACCEPT pFuncOnAcceptUser;
	FUNC_ONDISCONNECTED pFuncOnDisconnctedUser;
	FUNC_ONRECV pFuncOnRecvUser;

	FUNC_ONACCEPT pFuncOnAcceptServer;
	FUNC_ONDISCONNECTED pFuncOnDisconnctedServer;
	FUNC_ONRECV pFuncOnRecvServer;

	// default constructor
	IOInitDesc()
	{
		memset(this, 0, sizeof(IOInitDesc));
		uMaxConnServer = DEF_DEFAULT_MAX_CONN;
		uMaxConnUser = DEF_DEFAULT_MAX_CONN;
		uMaxPacketLength = DEF_DEFAULT_MAX_PACKET_LENGTH;
		bUseIOCP = false;
	}
};

struct IOTimerJob
{
	unsigned int nJobId;
	unsigned int nLastTriggerTime;
	unsigned int nTriggerIntervalMS;
	FUNC_ONTIMER fnOnTimer;
};
using IOTimerJobList = std::list<IOTimerJob*>;
//////////////////////////////////////////////////////////////////////////
// All function starts with Sync can invoke in event callbacks
// In other threads, Functions without Sync prefix must be called
class IOServer
{
	friend class IOConn;

public:
	IOServer();
	~IOServer();

public:
	int Init(const IOInitDesc* _pDesc);
	int Start(const char* _pszAddr, unsigned short _uPort);
	int Stop();
	void Join();
	int Connect(const char* _pszAddr, unsigned short _sPort, FUNC_ONCONNECTSUCCESS _fnSuccess, FUNC_ONCONNECTFAILED _fnFailed, void* _pArg);
	int AddTimerJob(unsigned int _nJobId, unsigned int _nTriggerIntervalMS, FUNC_ONTIMER _fnOnTimer);
	int RemoveTimerJob(unsigned int _nJobId);
	int ClearTimerJob();

	// Thread-safe
	int SendPacketToUser(unsigned int _uConnIndex, char* _pData, size_t _uLength);
	int SendPacketToServer(unsigned int _uConnIndex, char* _pData, size_t _uLength);
	int CloseUserConnection(unsigned int _uConnIndex);
	int CloseServerConnection(unsigned int _uConnIndex);

	// Synchronize send packet , within event callback
	int SyncSendPacketToUser(unsigned int _uConnIndex, char* _pData, size_t _uLength);
	int SyncSendPacketToServer(unsigned int _uConnIndex, char* _pData, size_t _uLength);
	int SyncConnect(const char* _pszAddr, unsigned short _sPort, FUNC_ONCONNECTSUCCESS _fnSuccess, FUNC_ONCONNECTFAILED _fnFailed, void* _pArg);

public:
	inline IOServerStatus GetServerStatus()						{return m_eStatus.load();}
	inline unsigned int GetMaxConnUser()						{return m_uMaxConnUser;}
	inline void SetMaxConnUser(unsigned int _uConn)				{m_uMaxConnUser = _uConn;}
	inline size_t GetMaxPacketLength()							{return m_uMaxPacketLength;}

	inline int GetConnectedServerCount()					{return m_nConnectedServerCount;}
	inline int GetConnectedUserCount()						{return m_nConnectedUserCount;}

	inline IOConn* GetUserConn(unsigned int _uConnIndex);
	inline void SetUserConn(unsigned int _uConnIndex, IOConn* conn);

	inline IOConn* GetServerConn(unsigned int _uConnIndex);
	inline void SetServerConn(unsigned int _uConnIndex, IOConn* conn);

	inline void LockSendBuffer();
	inline void UnlockSendBuffer();

	void Callback_OnAcceptUser(unsigned int _uIndex);
	void Callback_OnAcceptServer(unsigned int _uIndex);
	void Callback_OnDisconnectedUser(unsigned int _uIndex);
	void Callback_OnDisconnectedServer(unsigned int _uIndex);
	void Callback_OnRecvUser(unsigned int _uIndex, char* _pData, unsigned int _uLength);
	void Callback_OnRecvServer(unsigned int _uIndex,  char* _pData, unsigned int _uLength);

protected:
	void onConnectionClosed(IOConn* _pConn);
	void onUserConnectionClosed(IOConn* _pConn);
	void onServerConnectionClosed(IOConn* _pConn);
	void processConnEvent();
	void awake();

	void processConnectAction(IOActionConnectContext* _pAction);
	void processTimerJob();

public:
	static unsigned int __stdcall __threadEntry(void*);
	static void __onAcceptConn(struct evconnlistener *pEvListener, evutil_socket_t sock, struct sockaddr *pAddr, int iLen, void *ptr);
	static void __onAcceptErr(struct evconnlistener *pEvListener, void *ptr);
	static void __onConnErr();

	static void __onConnRead(struct bufferevent* pEv, void* pCtx);
	static void __onConnEvent(struct bufferevent* pEv, short what, void* pCtx);

	static void __onThreadRead(struct bufferevent* pEv, void* pCtx);

	static void __onEventTimer(evutil_socket_t, short _nEvents, void * _pCxt);

protected:
	uintptr_t m_stThreadId;
	event_base* m_pEventBase;
	evconnlistener* m_pConnListener;

	std::string m_xAddr;
	unsigned short m_uPort;

	// options
	size_t m_uMaxConnUser;
	size_t m_uMaxConnServer;
	bool m_bUseIOCP;
	size_t m_uMaxPacketLength;

	IndexManager m_xUserIndexMgr;
	IndexManager m_xServerIndexMgr;
	IOConn** m_pUserConnArray;
	IOConn** m_pServerConnArray;

	evutil_socket_t m_arraySocketPair[2];
	bufferevent* m_pBvEvent;

	std::mutex m_xSendMutex;
	IOBuffer m_xEventBuffer;

	//	callbacks
	FUNC_ONACCEPT m_pFuncOnAcceptUser;
	FUNC_ONACCEPT m_pFuncOnAcceptServer;
	FUNC_ONDISCONNECTED m_pFuncOnDisconnectedUser;
	FUNC_ONDISCONNECTED m_pFuncOnDisconnectedServer;
	FUNC_ONRECV m_pFuncOnRecvUser;
	FUNC_ONRECV m_pFuncOnRecvServer;

	//	connected count
	int m_nConnectedUserCount;
	int m_nConnectedServerCount;

	//	timer list
	event* m_pTimerEvent;
	std::mutex m_xTimerMutex;
	IOTimerJobList m_xTimerJobs;

	// status
	std::atomic<IOServerStatus> m_eStatus;
	int m_nWorkingTid;

	// thread
	std::thread m_trd;
};

IONS_END
//////////////////////////////////////////////////////////////////////////
#endif