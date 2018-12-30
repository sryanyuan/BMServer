#include "NetbaseWrapper.h"
//////////////////////////////////////////////////////////////////////////
NetbaseWrapper::NetbaseWrapper()
{

}

NetbaseWrapper::~NetbaseWrapper()
{
	Stop();
}

BOOL __stdcall NetbaseWrapper::CreateNetwork(DESC_NETWORK* desc,DWORD dwUserAcceptInterval,DWORD dwServerAcceptInterval)
{
	memset(&m_stDesc, 0, sizeof(m_stDesc));
	m_stDesc.pFuncOnAcceptUser = (FUNC_ONACCEPT)desc->OnAcceptUser;
	m_stDesc.pFuncOnDisconnctedUser = (FUNC_ONDISCONNECTED)desc->OnDisconnectUser;
	m_stDesc.pFuncOnRecvUser = (FUNC_ONRECV)desc->OnRecvFromUserTCP;
	m_stDesc.pFuncOnAcceptServer = (FUNC_ONACCEPT)desc->OnAcceptServer;
	m_stDesc.pFuncOnDisconnctedServer = (FUNC_ONDISCONNECTED)desc->OnDisconnectServer;
	m_stDesc.pFuncOnRecvServer = (FUNC_ONRECV)desc->OnRecvFromServerTCP;
	m_stDesc.uMaxConnUser = desc->dwMaxUserNum;

	if(desc->dwFlag & 0x00000001)
	{
		m_stDesc.bUseIOCP = true;
	}

	Init(&m_stDesc);

	//	add timer events
	if(NULL != desc->pEvent)
	{
		for(DWORD i = 0; i < desc->dwCustomDefineEventNum; ++i)
		{
			AddTimerJob(i, int(desc->pEvent[i].dwPeriodicTime), FUNC_ONTIMER(desc->pEvent[i].pEventFunc));
		}
	}

	return TRUE;
}

void __stdcall NetbaseWrapper::BreakMainThread()
{

}

void __stdcall NetbaseWrapper::ResumeMainThread()
{

}

void __stdcall NetbaseWrapper::SetUserInfo(DWORD dwConnectionIndex,void* user)
{

}

void* __stdcall NetbaseWrapper::GetUserInfo(DWORD dwConnectionIndex)
{
	return NULL;
}

void __stdcall NetbaseWrapper::SetServerInfo(DWORD dwConnectionIndex,void* server)
{

}

void* __stdcall NetbaseWrapper::GetServerInfo(DWORD dwConnectionIndex)
{
	return NULL;
}

sockaddr_in* __stdcall NetbaseWrapper::GetServerAddress(DWORD dwConnectionIndex)
{
	SServerConn* pConn = GetServerConn(dwConnectionIndex);
	if(NULL == pConn)
	{
		return NULL;
	}

	return pConn->GetAddress();
}

sockaddr_in* __stdcall NetbaseWrapper::GetUserAddress(DWORD dwConnectionIndex)
{
	SServerConn* pConn = GetUserConn(dwConnectionIndex);
	if(NULL == pConn)
	{
		return NULL;
	}

	return pConn->GetAddress();
}

BOOL __stdcall NetbaseWrapper::GetServerAddress(DWORD dwConnectionIndex,char* pIP,WORD* pwPort)
{
	SServerConn* pConn = GetServerConn(dwConnectionIndex);
	if(NULL == pConn)
	{
		return FALSE;
	}

	if(pConn->GetAddress(pIP, (unsigned short*)pwPort))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL __stdcall NetbaseWrapper::GetUserAddress(DWORD dwConnectionIndex,char* pIP,WORD* pwPort)
{
	SServerConn* pConn = GetUserConn(dwConnectionIndex);
	if(NULL == pConn)
	{
		return FALSE;
	}

	if(pConn->GetAddress(pIP, (unsigned short*)pwPort))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL __stdcall NetbaseWrapper::SendToServer(DWORD dwConnectionIndex,char* msg,DWORD length,DWORD flag)
{
	if(0 == SyncSendPacketToServer(dwConnectionIndex, msg, length))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL __stdcall NetbaseWrapper::SendToUser(DWORD dwConnectionIndex,char* msg,DWORD length,DWORD flag)
{
	if(0 == SyncSendPacketToUser(dwConnectionIndex, msg, length))
	{
		return TRUE;
	}
	return FALSE;
}

void __stdcall NetbaseWrapper::CompulsiveDisconnectServer(DWORD dwConnectionIndex)
{
	CloseServerConnection(dwConnectionIndex);
}

void __stdcall NetbaseWrapper::CompulsiveDisconnectUser(DWORD dwConnectionIndex)
{
	CloseUserConnection(dwConnectionIndex);
}

int __stdcall NetbaseWrapper::GetServerMaxTransferRecvSize()
{
	return 0;
}

int __stdcall NetbaseWrapper::GetServerMaxTransferSendSize()
{
	return 0;
}

int __stdcall NetbaseWrapper::GetUserMaxTransferRecvSize()
{
	return 0;
}

int __stdcall NetbaseWrapper::GetUserMaxTransferSendSize()
{
	return 0;
}

void __stdcall NetbaseWrapper::BroadcastServer(char* pMsg,DWORD len,DWORD flag)
{
	
}

void __stdcall NetbaseWrapper::BroadcastUser(char* pMsg,DWORD len,DWORD flag)
{

}

DWORD __stdcall NetbaseWrapper::GetConnectedServerNum()
{
	return (DWORD)GetConnectedServerCount();
}

DWORD __stdcall NetbaseWrapper::GetConnectedUserNum()
{
	return (DWORD)GetConnectedUserCount();
}

WORD __stdcall NetbaseWrapper::GetBindedPortServerSide()
{
	return 0;
}

WORD __stdcall NetbaseWrapper::GetBindedPortUserSide()
{
	return 0;
}

BOOL __stdcall NetbaseWrapper::ConnectToServerWithUserSide(char* szIP,WORD port,CONNECTSUCCESSFUNC,CONNECTFAILFUNC,void* pExt)
{
	return FALSE;
}

BOOL __stdcall NetbaseWrapper::ConnectToServerWithServerSide(char* szIP,WORD port,CONNECTSUCCESSFUNC fnSuccess,CONNECTFAILFUNC fnFailed,void* pExt)
{
	if(0 == SyncConnect(szIP, (unsigned short)port, FUNC_ONCONNECTSUCCESS(fnSuccess), FUNC_ONCONNECTFAILED(fnFailed), pExt))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL __stdcall NetbaseWrapper::StartServerWithUserSide(char* ip,WORD port)
{
	if(0 == Start(ip, (unsigned short)port))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL __stdcall NetbaseWrapper::StartServerWithServerSide(char* ip,WORD port)
{
	/*if(0 == Start(ip, (unsigned short)port))
	{
		return TRUE;
	}*/
	return FALSE;
}

HANDLE __stdcall NetbaseWrapper::GetCustomEventHandle(DWORD index)
{
	return NULL;
}

BOOL __stdcall NetbaseWrapper::PauseTimer(DWORD dwCustomEventIndex)
{
	return FALSE;
}

BOOL __stdcall NetbaseWrapper::ResumeTimer(DWORD dwCustomEventIndex)
{
	return FALSE;
}

BOOL __stdcall NetbaseWrapper::SendToServer(DWORD dwConnectionIndex,INET_BUF* pBuf,DWORD dwNum,DWORD flag)
{
	return FALSE;
}

BOOL __stdcall NetbaseWrapper::SendToUser(DWORD dwConnectionIndex,INET_BUF* pBuf,DWORD dwNum,DWORD flag)
{
	return FALSE;
}

BOOL __stdcall NetbaseWrapper::SendToServer(DWORD dwConnectionIndex,PACKET_LIST* pList,DWORD flag)
{
	return FALSE;
}

BOOL __stdcall NetbaseWrapper::SendToUser(DWORD dwConnectionIndex,PACKET_LIST* pList,DWORD flag)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// new
CNetbase* CreateNetbaseWrapper()
{
	return new NetbaseWrapper;
}

void DestroyNetbaseWrapper(CNetbase* pNet)
{
	delete pNet;
}