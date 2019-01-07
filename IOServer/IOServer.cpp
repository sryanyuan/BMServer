#include "IOServer.h"
#include <event2\thread.h>
#include <assert.h>
#include "Logger.h"

IONS_START
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32

#if _MSC_VER == 1800
#define LIBPATH_LIBEVENT 
#else
#define LIBPATH_LIBEVENT "lib/libevent/"
#endif

#pragma comment(lib, "ws2_32.lib")

#if _MSC_VER == 1700
#ifdef _DEBUG
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_vc11_d.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_core_vc11_d.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_extras_vc11_d.lib")
#else
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_vc11.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_core_vc11.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_extras_vc11.lib")
#endif
#elif _MSC_VER == 1500
#ifdef _DEBUG
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_vc9_d.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_core_vc9_d.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_extras_vc9_d.lib")
#else
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_vc9.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_core_vc9.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_extras_vc9.lib")
#endif
#elif _MSC_VER == 1800
#ifdef _DEBUG
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_vc12_d.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_core_vc12_d.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_extras_vc12_d.lib")
#else
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_vc12.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_core_vc12.lib")
#pragma comment(lib, LIBPATH_LIBEVENT"libevent_extras_vc12.lib")
#endif
#else
#error VS version not support
#endif

#endif
//////////////////////////////////////////////////////////////////////////
IOServer::IOServer()
{
	m_eStatus = IOServerStatus_Stop;
	m_nWorkingTid = 0;

	m_pBvEvent = NULL;
	m_pEventBase = NULL;
	m_pConnListener = NULL;
	m_uPort = 0;
	memset(&m_stThreadId, 0, sizeof(m_stThreadId));
	memset(m_arraySocketPair, 0, sizeof(m_arraySocketPair));
	m_pUserConnArray = NULL;
	m_xEventBuffer.AllocBuffer(DEF_DEFAULT_ENGINE_WRITEBUFFERSIZE);

	m_pFuncOnAcceptUser = NULL;
	m_pFuncOnDisconnectedUser = NULL;
	m_pFuncOnRecvUser = NULL;

	m_pFuncOnAcceptServer = NULL;
	m_pFuncOnDisconnectedServer = NULL;
	m_pFuncOnRecvServer = NULL;

	m_nConnectedServerCount = m_nConnectedUserCount = 0;

	m_pTimerEvent = NULL;

	// options
	m_bUseIOCP = false;
	m_uMaxConnUser = DEF_DEFAULT_MAX_CONN;
	m_uMaxConnServer = DEF_DEFAULT_MAX_CONN;
	m_uMaxPacketLength = DEF_DEFAULT_MAX_PACKET_LENGTH;
}

IOServer::~IOServer()
{
	if (NULL != m_pUserConnArray) {
		delete[] m_pUserConnArray;
		m_pUserConnArray = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
//
int IOServer::Init(const IOInitDesc* _pDesc)
{
	m_uMaxConnUser = _pDesc->uMaxConnUser;
	if (0 == m_uMaxConnUser) {
		m_uMaxConnUser = DEF_DEFAULT_MAX_CONN;
	}
	if (0 == m_uMaxConnServer) {
		m_uMaxConnServer = DEF_DEFAULT_MAX_CONN;
	}

	m_pUserConnArray = new IOConn*[m_uMaxConnUser + 1];
	memset(m_pUserConnArray, 0, sizeof(IOConn*) * (m_uMaxConnUser + 1));
	m_pServerConnArray = new IOConn*[m_uMaxConnServer + 1];
	memset(m_pServerConnArray, 0, sizeof(IOConn*) * (m_uMaxConnServer + 1));

	//	init index manager
	m_xUserIndexMgr.Init(m_uMaxConnUser);
	m_xServerIndexMgr.Init(m_uMaxConnServer);

	//	init callback functions
	m_pFuncOnAcceptUser = _pDesc->pFuncOnAcceptUser;
	m_pFuncOnDisconnectedUser = _pDesc->pFuncOnDisconnctedUser;
	m_pFuncOnRecvUser = _pDesc->pFuncOnRecvUser;

	m_pFuncOnAcceptServer = _pDesc->pFuncOnAcceptServer;
	m_pFuncOnDisconnectedServer = _pDesc->pFuncOnDisconnctedServer;
	m_pFuncOnRecvServer = _pDesc->pFuncOnRecvServer;

	//	use iocp
	m_bUseIOCP = _pDesc->bUseIOCP;

	// limit max packet length
	m_uMaxPacketLength = (size_t)_pDesc->uMaxPacketLength;

	return IOResult_Ok;
}

int IOServer::Start(const char* _pszAddr, unsigned short _uPort)
{
	m_uPort = _uPort;
	m_xAddr = _pszAddr;

	// Create worker thread
	try
	{
		m_trd = std::thread(&IOServer::__threadEntry, this);
	}
	catch (std::system_error& e)
	{
		LOGERROR("Create DB thread failed: %s", e.what());
		return 0;
	}

	return IOResult_Ok;
}

int IOServer::Stop()
{
	if(m_eStatus != IOServerStatus_Running)
	{
		return 1;
	}

	if (NULL != m_pEventBase)
	{
		// The thread will be exited at the next loop, only call Stop in event callback functions
		int nRet = event_base_loopbreak(m_pEventBase);
		return nRet;
	}

	return 2;
}

void IOServer::Join() {
	m_trd.join();
}

int IOServer::Connect(const char* _pszAddr, unsigned short _sPort, FUNC_ONCONNECTSUCCESS _fnSuccess, FUNC_ONCONNECTFAILED _fnFailed, void* _pArg)
{
	IOAction action = {0};
	IOActionConnectContext ctx = {0};
	action.uAction = IOAction_Connect;
	ctx.addr.sin_family = AF_INET;
	ctx.addr.sin_port = htons(_sPort);
	ctx.addr.sin_addr.s_addr = inet_addr(_pszAddr);
	ctx.fnSuccess =_fnSuccess;
	ctx.fnFailed = _fnFailed;

	{
		std::unique_lock<std::mutex> locker(m_xSendMutex);
		m_xEventBuffer.Write((char*)&action, sizeof(IOAction));
		m_xEventBuffer.Write((char*)&ctx, sizeof(IOActionConnectContext));
	}
	
	//	awake and process event
	awake();

	return 0;
}

int IOServer::SyncConnect(const char* _pszAddr, unsigned short _sPort, FUNC_ONCONNECTSUCCESS _fnSuccess, FUNC_ONCONNECTFAILED _fnFailed, void* _pArg)
{
	IOAction action = {0};
	IOActionConnectContext ctx = {0};
	action.uAction = IOAction_Connect;
	ctx.addr.sin_family = AF_INET;
	ctx.addr.sin_port = htons(_sPort);
	ctx.addr.sin_addr.s_addr = inet_addr(_pszAddr);
	ctx.fnSuccess =_fnSuccess;
	ctx.fnFailed = _fnFailed;

	processConnectAction(&ctx);
	return 0;
}

int IOServer::SendPacketToServer(unsigned int _uConnIndex, char* _pData, size_t _uLength)
{
	IOConn* pConn = GetServerConn(_uConnIndex);
	if (NULL == pConn) {
		return -1;
	}

	LockSendBuffer();

	IOAction action = {0};
	action.uAction = IOAction_SendToServer;
	action.uIndex = (unsigned short)_uConnIndex;
	action.uTag = (unsigned short)_uLength;
	size_t uRet = m_xEventBuffer.Write((char*)&action, sizeof(IOAction));
	m_xEventBuffer.Write(_pData, _uLength);

	UnlockSendBuffer();

	//	awake and process event
	awake();

	return int(uRet);
}

int IOServer::SendPacketToUser(unsigned int _uConnIndex, char* _pData, size_t _uLength)
{
	IOConn* pConn = GetUserConn(_uConnIndex);
	if (NULL == pConn) {
		return -1;
	}

	LockSendBuffer();

	IOAction action = {0};
	action.uAction = IOAction_SendToUser;
	action.uIndex = (unsigned short)_uConnIndex;
	action.uTag = (unsigned short)_uLength;
	size_t uRet = m_xEventBuffer.Write((char*)&action, sizeof(IOAction));
	m_xEventBuffer.Write(_pData, _uLength);

	UnlockSendBuffer();

	//	awake and process event
	awake();

	return int(uRet);
}

int IOServer::SyncSendPacketToServer(unsigned int _uConnIndex, char* _pData, size_t _uLength)
{
	IOConn* pConn = GetServerConn(_uConnIndex);
	if (NULL == pConn) {
		LOGERROR("Failed to get conn, index:%d", _uConnIndex);
		return 1;
	}
	if (pConn->eConnState != IOConnState_Connected)
	{
		LOGERROR("Trying to send to a disconnected connection");
		return 2;
	}

	//	write head
	unsigned int uNetLength = DEF_NETPROTOCOL_HEADER_LENGTH + _uLength;
	uNetLength = htonl(uNetLength);

	//	try send
	int nDataSended = bufferevent_write(pConn->pEv, &uNetLength, DEF_NETPROTOCOL_HEADER_LENGTH);
	if (0 != nDataSended) {
		return nDataSended;
	}
	nDataSended = bufferevent_write(pConn->pEv, _pData, _uLength);
	if (0 != nDataSended) {
		return nDataSended;
	}

	return 0;
}

int IOServer::SyncSendPacketToUser(unsigned int _uConnIndex, char* _pData, size_t _uLength)
{
	IOConn* pConn = GetUserConn(_uConnIndex);
	if (NULL == pConn) {
		LOGERROR("Failed to get conn, index:%d", _uConnIndex);
		return 1;
	}
	if (pConn->eConnState != IOConnState_Connected)
	{
		LOGERROR("Trying to send to a disconnected connection");
		return 2;
	}

	//	write head
	unsigned int uNetLength = DEF_NETPROTOCOL_HEADER_LENGTH + _uLength;
	uNetLength = htonl(uNetLength);

	//	try send
	int nDataSended = bufferevent_write(pConn->pEv, &uNetLength, DEF_NETPROTOCOL_HEADER_LENGTH);
	if (0 != nDataSended) {
		return nDataSended;
	}
	nDataSended = bufferevent_write(pConn->pEv, _pData, _uLength);
	if (0 != nDataSended) {
		return nDataSended;
	}

	return 0;
}

int IOServer::CloseUserConnection(unsigned int _uConnIndex)
{
	IOAction action = {0};
	action.uAction = IOAction_CloseUserConn;
	action.uIndex = (unsigned short)_uConnIndex;

	{
		std::unique_lock<std::mutex> locker(m_xSendMutex);
		m_xEventBuffer.Write((char*)&action, sizeof(IOAction));
	}
	
	awake();

	return 0;
}

IOConn* IOServer::GetUserConn(unsigned int _uConnIndex)
{
	if (_uConnIndex > m_uMaxConnUser ||
		0 == _uConnIndex) {
		LOGERROR("Invalid conn index %d", _uConnIndex);
		return NULL;
	}
	return m_pUserConnArray[_uConnIndex];
}

void IOServer::SetUserConn(unsigned int _uConnIndex, IOConn* conn)
{
	if (_uConnIndex > m_uMaxConnUser ||
		0 == _uConnIndex) {
		LOGERROR("Invalid conn index %d", _uConnIndex);
		return;
	}
	m_pUserConnArray[_uConnIndex] = conn;
}

IOConn* IOServer::GetServerConn(unsigned int _uConnIndex)
{
	if (_uConnIndex > m_uMaxConnServer ||
		0 == _uConnIndex) {
		LOGERROR("Invalid conn index %d", _uConnIndex);
		return NULL;
	}
	return m_pServerConnArray[_uConnIndex];
}

void IOServer::SetServerConn(unsigned int _uConnIndex, IOConn* conn)
{
	if (_uConnIndex > m_uMaxConnServer ||
		0 == _uConnIndex) {
		LOGERROR("Invalid conn index %d", _uConnIndex);
		return;
	}
	m_pServerConnArray[_uConnIndex] = conn;
}

void IOServer::LockSendBuffer()
{
	m_xSendMutex.lock();
}

void IOServer::UnlockSendBuffer()
{
	m_xSendMutex.unlock();
}

int IOServer::CloseServerConnection(unsigned int _uConnIndex)
{
	IOAction action = {0};
	action.uAction = IOAction_CloseServerConn;
	action.uIndex = (unsigned short)_uConnIndex;

	{
		std::unique_lock<std::mutex> locker(m_xSendMutex);
		m_xEventBuffer.Write((char*)&action, sizeof(IOAction));
	}

	awake();

	return 0;
}

void IOServer::Callback_OnAcceptUser(unsigned int _uIndex)
{
	if (NULL == m_pFuncOnAcceptUser) {
		return;
	}
	m_pFuncOnAcceptUser(_uIndex);
}

void IOServer::Callback_OnAcceptServer(unsigned int _uIndex)
{
	if (NULL == m_pFuncOnAcceptServer) {
		return;
	}
	m_pFuncOnAcceptServer(_uIndex);
}

void IOServer::Callback_OnDisconnectedUser(unsigned int _uIndex)
{
	if (NULL == m_pFuncOnDisconnectedUser) {
		return;
	}
	m_pFuncOnDisconnectedUser(_uIndex);
}

void IOServer::Callback_OnDisconnectedServer(unsigned int _uIndex)
{
	if (NULL == m_pFuncOnDisconnectedServer) {
		return;
	}
	m_pFuncOnDisconnectedServer(_uIndex);
}

void IOServer::Callback_OnRecvUser(unsigned int _uIndex, char* _pData, unsigned int _uLength)
{
	if (NULL == m_pFuncOnRecvUser) {
		return;
	}
	m_pFuncOnRecvUser(_uIndex, _pData, _uLength);
}

void IOServer::Callback_OnRecvServer(unsigned int _uIndex, char* _pData, unsigned int _uLength)
{
	if (NULL == m_pFuncOnRecvServer) {
		return;
	}
	m_pFuncOnRecvServer(_uIndex, _pData, _uLength);
}

void IOServer::onConnectionClosed(IOConn* _pConn)
{
	if (_pConn->bServerConn) {
		onServerConnectionClosed(_pConn);
	} else {
		onUserConnectionClosed(_pConn);
	}
}

void IOServer::onUserConnectionClosed(IOConn* _pConn)
{
	//	after connection is closed (recv return error)
	LOGPRINT("User connection %d closed", _pConn->uConnIndex);

	//	callback
	Callback_OnDisconnectedUser(_pConn->uConnIndex);

	//	free index
	SetUserConn(_pConn->uConnIndex, NULL);
	m_xUserIndexMgr.Push(_pConn->uConnIndex);

	//	free bufferevent
	bufferevent_free(_pConn->pEv);
	_pConn->pEv = NULL;
	delete _pConn;
	_pConn = NULL;

	--m_nConnectedUserCount;
}

void IOServer::onServerConnectionClosed(IOConn* _pConn)
{
	LOGINFO("Server connection %d closed", _pConn->uConnIndex);

	//	callback
	//	2 possibilities, 1 : connect failed and remove conn 2 : connected and remove conn
	if (_pConn->eConnState == IOConnState_Connected) {
		Callback_OnDisconnectedServer(_pConn->uConnIndex);
		_pConn->eConnState = IOConnState_Disconnected;
	}

	//	free index
	SetServerConn(_pConn->uConnIndex, NULL);
	m_xServerIndexMgr.Push(_pConn->uConnIndex);

	//	free bufferevent
	bufferevent_free(_pConn->pEv);
	_pConn->pEv = NULL;
	delete _pConn;
	_pConn = NULL;

	--m_nConnectedServerCount; 
}

// process all events in libevent thread
void IOServer::processConnEvent()
{
	IOAction action = {0};

	std::unique_lock<std::mutex> locker(m_xSendMutex);

	while(0 != m_xEventBuffer.GetReadableSize())
	{
		if (sizeof(IOAction) != m_xEventBuffer.Read((char*)&action, sizeof(IOAction))) {
			LOGERROR("Process conn event failed.");
			m_xEventBuffer.Reset();
			return;
		}

		if (IOAction_SendToUser == action.uAction) {
			IOConn* pConn = GetUserConn((unsigned int)action.uIndex);
			if (NULL == pConn) {
				LOGERROR("Failed to get conn, index:%d", action.uIndex);
			} else {
				//	write head
				if (pConn->eConnState == IOConnState_Connected)
				{
					unsigned int uNetLength = DEF_NETPROTOCOL_HEADER_LENGTH + action.uTag;
					uNetLength = htonl(uNetLength);
					bufferevent_write(pConn->pEv, &uNetLength, DEF_NETPROTOCOL_HEADER_LENGTH);
					bufferevent_write(pConn->pEv, m_xEventBuffer.GetReadableBufferPtr(), action.uTag);
				}
			}
			m_xEventBuffer.Read(NULL, action.uTag);
		} else if(IOAction_SendToServer == action.uAction) {
			IOConn* pConn = GetServerConn((unsigned int)action.uIndex);
			if(NULL == pConn) {
				LOGERROR("Failed to get conn, index:%d", action.uIndex);
			} else {
				//	write head
				if (pConn->eConnState == IOConnState_Connected)
				{
					unsigned int uNetLength = DEF_NETPROTOCOL_HEADER_LENGTH + action.uTag;
					uNetLength = htonl(uNetLength);
					bufferevent_write(pConn->pEv, &uNetLength, DEF_NETPROTOCOL_HEADER_LENGTH);
					bufferevent_write(pConn->pEv, m_xEventBuffer.GetReadableBufferPtr(), action.uTag);
				}
			}
			m_xEventBuffer.Read(NULL, action.uTag);
		} else if (IOAction_CloseUserConn == action.uAction) {
			IOConn* pConn = GetUserConn((unsigned int)action.uIndex);
			if (NULL == pConn) {
				LOGERROR("Failed to close conn, index:%d", action.uIndex);
			} else {
				onUserConnectionClosed(pConn);
				//	User close the connection, just close socket
				/*if (INVALID_SOCKET != pConn->fd) {
					evutil_closesocket(pConn->fd);
					pConn->fd = INVALID_SOCKET;
					LOGDEBUG("Close fd of user con %d", pConn->uConnIndex);
				}*/
			}
		} else if (IOAction_CloseServerConn == action.uAction) {
			IOConn* pConn = GetServerConn((unsigned int)action.uIndex);
			if (NULL == pConn) {
				LOGERROR("Failed to close conn, index:%d", action.uIndex);
			} else {
				onServerConnectionClosed(pConn);
				//	just close the socket , after recv return error, the bufferevent will be deleted
				/*if (INVALID_SOCKET != pConn->fd) {
					evutil_closesocket(pConn->fd);
					pConn->fd = INVALID_SOCKET;
					LOGDEBUG("Close fd of server con %d", pConn->uConnIndex);
				}*/
			}
		} else if (IOAction_Connect == action.uAction) {
			IOActionConnectContext ctx;
			m_xEventBuffer.Read((char*)&ctx, sizeof(IOActionConnectContext));
			processConnectAction(&ctx);
		} else {
			LOGERROR("Invalid conn event %d", action.uAction);
			m_xEventBuffer.Reset();
		}
	}

	//	reset the buffer
	m_xEventBuffer.Reset();
}

void IOServer::awake()
{
	int nRet = send(m_arraySocketPair[1], "1", 1, 0);
	if (nRet < 1) {
		LOGERROR("Write notify fail, err(%d): %s", errno, strerror(errno));
	}
}

void IOServer::processConnectAction(IOActionConnectContext* _pAction)
{
	//	get new conn index
	unsigned int uConnIndex = m_xServerIndexMgr.Pop();
	if (uConnIndex == IndexManager::s_uInvalidIndex) {
		LOGERROR("Reach max fd");
		_pAction->fnFailed(0, _pAction->pArg);
		return;
	}

	bufferevent* pBev = bufferevent_socket_new(m_pEventBase, -1, IOCONN_FLAG);
	if ( NULL == pBev ) {
		LOGERROR("Can't create bufferevent, addr %d port %d", _pAction->addr.sin_addr.s_addr, _pAction->addr.sin_port );
		_pAction->fnFailed(uConnIndex, _pAction->pArg);
		return;
	}

	int nRet = bufferevent_socket_connect(pBev, (struct sockaddr *)&_pAction->addr, sizeof(struct sockaddr));
	if (0 != nRet) {
		bufferevent_free(pBev);
		pBev = NULL;
		LOGERROR("Connect failed. addr %s port %d", _pAction->addr.sin_addr.s_addr, _pAction->addr.sin_port);
		_pAction->fnFailed(uConnIndex, _pAction->pArg);
		return;
	}

	int nFd = bufferevent_getfd(pBev);

	//	create new conn context
	IOConn* pConn = new IOConn;
	pConn->pEng = this;
	pConn->fd = nFd;
	pConn->pEv = pBev;
	pConn->uConnIndex = uConnIndex;
	pConn->bServerConn = true;
	pConn->m_fnOnConnectSuccess = _pAction->fnSuccess;
	pConn->m_fnOnConnectFailed = _pAction->fnFailed;

	bufferevent_setcb(pBev,
		&IOServer::__onConnRead,
		NULL,
		&IOServer::__onConnEvent,
		pConn);
	bufferevent_setwatermark(pBev, EV_READ, DEF_NETPROTOCOL_HEADER_LENGTH, 0);
	bufferevent_setwatermark(pBev, EV_WRITE, 0, 0);
	bufferevent_enable(pBev, EV_READ);
	SetServerConn(uConnIndex, pConn);

	//	get address
	sockaddr_in addr;
	int len = sizeof(sockaddr);
	memset(&addr, 0, len);
	getpeername (nFd, (sockaddr*)&addr, &len);
	pConn->SetAddress(&addr);
	pConn->eConnState = IOConnState_Connecting;

	//	wait for connect result
}

int IOServer::AddTimerJob(unsigned int _nJobId, unsigned int _nTriggerIntervalMS, FUNC_ONTIMER _fnOnTimer)
{
	IOTimerJob* pJob = new IOTimerJob;
	memset(pJob, 0, sizeof(IOTimerJob));
	pJob->nJobId = _nJobId;
	pJob->nTriggerIntervalMS = _nTriggerIntervalMS;
	pJob->fnOnTimer = _fnOnTimer;

	std::unique_lock<std::mutex> locker(m_xTimerMutex);
	m_xTimerJobs.push_back(pJob);

	return 0;
}

int IOServer::RemoveTimerJob(unsigned int _nJobId)
{
	std::unique_lock<std::mutex> locker(m_xTimerMutex);

	IOTimerJobList::iterator iterB = m_xTimerJobs.begin();
	for(iterB;
		iterB != m_xTimerJobs.end();
		)
	{
		IOTimerJob* pJob = *iterB;

		if (pJob->nJobId == _nJobId) {
			delete pJob;
			iterB = m_xTimerJobs.erase(iterB);
		} else {
			++iterB;
		}
	}

	return 0;
}

int IOServer::ClearTimerJob()
{
	std::unique_lock<std::mutex> locker(m_xTimerMutex);

	IOTimerJobList::iterator iterB = m_xTimerJobs.begin();
	for(iterB;
		iterB != m_xTimerJobs.end();
		++iterB
		)
	{
		IOTimerJob* pJob = *iterB;
		delete pJob;
	}

	m_xTimerJobs.clear();

	return 0;
}

void IOServer::processTimerJob()
{
	std::unique_lock<std::mutex> locker(m_xTimerMutex);
	unsigned int nNowTick = GetTickCount();

	IOTimerJobList::const_iterator iterB = m_xTimerJobs.begin();
	for(iterB;
		iterB != m_xTimerJobs.end();
		++iterB)
	{
		IOTimerJob* pJob = *iterB;

		if (nNowTick > pJob->nLastTriggerTime + pJob->nTriggerIntervalMS) {
			//	trigger
			if (pJob->fnOnTimer) {
				pJob->fnOnTimer(pJob->nJobId);
			}

			//	update
			pJob->nLastTriggerTime = nNowTick;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//	static handlers
unsigned int __stdcall IOServer::__threadEntry(void* _pArg)
{
	IOServer* pIns = (IOServer*)_pArg;
	pIns->m_nWorkingTid = (int)GetCurrentThreadId();

	//	initialize
	//evthread_use_windows_threads();
	if (pIns->m_bUseIOCP) {
		LOGINFO("Use IOCP mode.");
		evthread_use_windows_threads();
		event_config* evcfg = event_config_new();
		event_config_set_flag(evcfg, EVENT_BASE_FLAG_STARTUP_IOCP);
		pIns->m_pEventBase = event_base_new_with_config(evcfg);
		event_config_free(evcfg);
	} else {
		pIns->m_pEventBase = event_base_new();
	}

	if (NULL == pIns->m_pEventBase) {
		LOGERROR("Create event_base failed");
		return -1;
	}

	//	create socket pair
	if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, pIns->m_arraySocketPair)) {
		LOGERROR("evutil_socketpair fail");
		return -1;
	}

	evutil_make_socket_nonblocking(pIns->m_arraySocketPair[0]);
	evutil_make_socket_nonblocking(pIns->m_arraySocketPair[1]);
	pIns->m_pBvEvent = bufferevent_socket_new(pIns->m_pEventBase, pIns->m_arraySocketPair[0], 0);
	if (NULL == pIns->m_pBvEvent) {
		LOGERROR("Create bufferevent failed");
		return -1;
	}

	// set callback
	bufferevent_setcb(pIns->m_pBvEvent, &IOServer::__onThreadRead, NULL, NULL, pIns);
	bufferevent_setwatermark(pIns->m_pBvEvent, EV_READ, 1, 0);
	bufferevent_enable(pIns->m_pBvEvent, EV_READ);

	struct sockaddr_in sin = {0};
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(pIns->m_xAddr.c_str());
	sin.sin_port = htons(pIns->m_uPort);

	//	listen
	pIns->m_pConnListener = evconnlistener_new_bind(pIns->m_pEventBase,
		&IOServer::__onAcceptConn,
		pIns,
		LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,
		-1,
		(sockaddr*)&sin,
		sizeof(sin));
	if (NULL == pIns->m_pConnListener) {
		LOGPRINT("Failed to listen on host:%s port:%d", pIns->m_xAddr.c_str(), pIns->m_uPort);
		exit(IOResult_ListenFailed);
	}

	//	timer event
	struct timeval tv = { 0, 5 * 1000 };
	pIns->m_pTimerEvent = event_new(pIns->m_pEventBase, -1, EV_PERSIST, &IOServer::__onEventTimer, pIns);
	if ( NULL == pIns->m_pTimerEvent ) {
		LOGERROR("Create lib event timer failed!");
		exit(1);
	}
	evtimer_add(pIns->m_pTimerEvent, &tv);

	LOGPRINT("Thread working, Id %d", GetCurrentThreadId());

	//	event loop
	pIns->m_eStatus = IOServerStatus_Running;
	int nResult = event_base_dispatch(pIns->m_pEventBase);
	pIns->m_eStatus = IOServerStatus_Stop;
	LOGINFO("event loop quit with code %d", nResult);

	// do clear up works
	// close listener
	LOGDEBUG("free listener");
	evconnlistener_free(pIns->m_pConnListener);
	pIns->m_pConnListener = NULL;
	// we should close all connections
	LOGDEBUG("free connections");
	for (int i = 0; i < (int)pIns->m_uMaxConnUser; ++i)
	{
		IOConn* pConn = pIns->m_pUserConnArray[i];
		if (NULL != pConn)
		{
			pIns->onUserConnectionClosed(pConn);
		}
	}
	for (int i = 0; i < (int)pIns->m_uMaxConnServer; ++i)
	{
		IOConn* pConn = pIns->m_pServerConnArray[i];
		if (NULL != pConn)
		{
			pIns->onServerConnectionClosed(pConn);
		}
	}
	// close local sockets
	LOGDEBUG("free socket pair");
	bufferevent_free(pIns->m_pBvEvent);
	pIns->m_pBvEvent = NULL;
	evutil_closesocket(pIns->m_arraySocketPair[1]);
	for (int i = 0; i < sizeof(pIns->m_arraySocketPair) / sizeof(pIns->m_arraySocketPair[0]); ++i)
	{
		pIns->m_arraySocketPair[i] = INVALID_SOCKET;
	}
	// free all timer job
	LOGDEBUG("free timer");
	evtimer_del(pIns->m_pTimerEvent);
	pIns->m_pTimerEvent = NULL;
	pIns->ClearTimerJob();
	// free the event base
	LOGDEBUG("free event base");
	event_base_free(pIns->m_pEventBase);
	pIns->m_pEventBase = NULL;

	return 0;
}

void IOServer::__onAcceptConn(struct evconnlistener *pEvListener, evutil_socket_t sock, struct sockaddr *pAddr, int iLen, void *ptr)
{
	IOServer* pIns = (IOServer*)ptr;
	event_base* pEventBase = evconnlistener_get_base(pEvListener);

	//	get new conn index
	unsigned int uConnIndex = pIns->m_xUserIndexMgr.Pop();
	if (0 == uConnIndex ||
		IndexManager::s_uInvalidIndex == uConnIndex) {
		LOGPRINT("Reach max connection, close new connection.");
		evutil_closesocket(sock);
		return;
	}

#ifdef _DEBUG
	LOGPRINT("Accept conn[%d], ThreadId %d", uConnIndex, GetCurrentThreadId());
#endif

	//	register event
	bufferevent* pEv = bufferevent_socket_new(pEventBase,
		sock,
		IOCONN_FLAG
		);
	if (NULL == pEv) {
		LOGPRINT("Failed to bind bufferevent");
		evutil_closesocket(sock);
		return;
	}

	++pIns->m_nConnectedUserCount;

	//	create new conn context
	IOConn* pConn = new IOConn;
	pConn->pEng = pIns;
	pConn->fd = sock;
	pConn->pEv = pEv;
	pConn->uConnIndex = uConnIndex;

	bufferevent_setcb(pEv,
		&IOServer::__onConnRead,
		NULL,
		&IOServer::__onConnEvent,
		pConn);
	bufferevent_setwatermark(pEv, EV_READ, DEF_NETPROTOCOL_HEADER_LENGTH, 0);
	bufferevent_setwatermark(pEv, EV_WRITE, 0, 0);
	bufferevent_enable(pEv, EV_READ);
	pIns->SetUserConn(uConnIndex, pConn);

	//	get address
	sockaddr_in addr;
	int len = sizeof(sockaddr);
	memset(&addr, 0, len);
	getpeername (sock, (sockaddr*)&addr, &len);
	pConn->SetAddress(&addr);
	pConn->eConnState = IOConnState_Connected;

	//	callback
	pIns->Callback_OnAcceptUser(uConnIndex);
}

void IOServer::__onAcceptErr(struct evconnlistener *pEvListener, void *ptr)
{
	LOGPRINT("Accept error");
}

void IOServer::__onConnRead(struct bufferevent* pEv, void* pCtx)
{
	IOConn* pConn = (IOConn*)pCtx;
	evbuffer* pInput = bufferevent_get_input(pEv);
	size_t uRead = evbuffer_get_length(pInput);

	if (0 == uRead) {
		//	connection closed
		LOGPRINT("Conn %d closed", pConn->uConnIndex);
		pConn->pEng->onConnectionClosed(pConn);
		
		return;
	}

	//	read head or body
	if (0 == pConn->m_uPacketHeadLength) {
		pConn->readHead();
	} else {
		pConn->readBody();
	}
}

void IOServer::__onConnEvent(struct bufferevent* pEv, short what, void* pCtx)
{
	IOConn* pConn = (IOConn*)pCtx;
	IOServer* pEng = pConn->pEng;

	//	process event
	if (what & BEV_EVENT_CONNECTED) {
		if (pConn->bServerConn &&
			pConn->eConnState == IOConnState_Connecting) {
			pConn->eConnState = IOConnState_Connected;
			pEng->Callback_OnAcceptServer(pConn->uConnIndex);
			pConn->Callback_OnConnectSuccess();
			++pEng->m_nConnectedServerCount;
		}
		return;
	}
	
	if (what & BEV_EVENT_EOF) {
		LOGPRINT("Conn %d closed", pConn->uConnIndex);
	} else {
		LOGPRINT("Conn %d error:%d", pConn->uConnIndex, what);
	}

	// on connecting as client role
	if (pConn->bServerConn &&
		pConn->eConnState == IOConnState_Connecting) {
		pConn->Callback_OnConnectFailed();
		pConn->eConnState = IOConnState_ConnectFailed;
	}

	pEng->onConnectionClosed(pConn);
}

void IOServer::__onThreadRead(struct bufferevent* pEv, void* pCtx)
{
	IOServer* pEng = (IOServer*)pCtx;

	struct evbuffer *pInput = bufferevent_get_input(pEv);
	evbuffer_drain(pInput, evbuffer_get_length(pInput));

	pEng->processConnEvent();
}

void IOServer::__onEventTimer(evutil_socket_t, short _nEvents, void * _pCxt)
{
	IOServer* pEng = (IOServer*)_pCxt;
	pEng->processTimerJob();
}

IONS_END