#ifndef _INC_NETBASEWRAPPER_
#define _INC_NETBASEWRAPPER_
//////////////////////////////////////////////////////////////////////////
#include "netbase.h"
#include "SServerEngine.h"
//////////////////////////////////////////////////////////////////////////
class NetbaseWrapper : public CNetbase, public SServerEngine
{
public:
	NetbaseWrapper();
	virtual ~NetbaseWrapper();

public:
	//	implement for CNetBase
	virtual BOOL			__stdcall			CreateNetwork(DESC_NETWORK* desc,DWORD dwUserAcceptInterval,DWORD dwServerAcceptInterval);
	virtual  void 			__stdcall			BreakMainThread();
	virtual  void			__stdcall			ResumeMainThread();

	virtual  void			__stdcall			SetUserInfo(DWORD dwConnectionIndex,void* user);
	virtual  void*			__stdcall			GetUserInfo(DWORD dwConnectionIndex);

	virtual  void			__stdcall			SetServerInfo(DWORD dwConnectionIndex,void* server);
	virtual  void*			__stdcall			GetServerInfo(DWORD dwConnectionIndex);

	virtual  sockaddr_in*	__stdcall			GetServerAddress(DWORD dwConnectionIndex);
	virtual  sockaddr_in*	__stdcall			GetUserAddress(DWORD dwConnectionIndex);
	virtual  BOOL			__stdcall			GetServerAddress(DWORD dwConnectionIndex,char* pIP,WORD* pwPort);
	virtual  BOOL			__stdcall			GetUserAddress(DWORD dwConnectionIndex,char* pIP,WORD* pwPort);
	virtual  BOOL			__stdcall			SendToServer(DWORD dwConnectionIndex,char* msg,DWORD length,DWORD flag);
	virtual  BOOL			__stdcall			SendToUser(DWORD dwConnectionIndex,char* msg,DWORD length,DWORD flag);
	virtual  void			__stdcall			CompulsiveDisconnectServer(DWORD dwConnectionIndex);
	virtual  void			__stdcall			CompulsiveDisconnectUser(DWORD dwConnectionIndex);
	virtual  int			__stdcall			GetServerMaxTransferRecvSize();
	virtual  int			__stdcall			GetServerMaxTransferSendSize();
	virtual  int			__stdcall			GetUserMaxTransferRecvSize();
	virtual  int			__stdcall			GetUserMaxTransferSendSize();
	virtual  void			__stdcall			BroadcastServer(char* pMsg,DWORD len,DWORD flag);
	virtual  void			__stdcall			BroadcastUser(char* pMsg,DWORD len,DWORD flag);
	virtual  DWORD			__stdcall			GetConnectedServerNum();
	virtual  DWORD			__stdcall			GetConnectedUserNum();
	virtual  WORD			__stdcall			GetBindedPortServerSide();
	virtual  WORD			__stdcall			GetBindedPortUserSide();
	virtual  BOOL			__stdcall			ConnectToServerWithUserSide(char* szIP,WORD port,CONNECTSUCCESSFUNC,CONNECTFAILFUNC,void* pExt);
	virtual  BOOL			__stdcall			ConnectToServerWithServerSide(char* szIP,WORD port,CONNECTSUCCESSFUNC,CONNECTFAILFUNC,void* pExt);	
	virtual  BOOL			__stdcall			StartServerWithUserSide(char* ip,WORD port);
	virtual  BOOL			__stdcall			StartServerWithServerSide(char* ip,WORD port);
	virtual  HANDLE			__stdcall			GetCustomEventHandle(DWORD index);
	virtual	 BOOL			__stdcall			PauseTimer(DWORD dwCustomEventIndex);
	virtual	 BOOL			__stdcall			ResumeTimer(DWORD dwCustomEventIndex);

	virtual BOOL			__stdcall			SendToServer(DWORD dwConnectionIndex,INET_BUF* pBuf,DWORD dwNum,DWORD flag);	
	virtual BOOL			__stdcall			SendToUser(DWORD dwConnectionIndex,INET_BUF* pBuf,DWORD dwNum,DWORD flag);	
	virtual BOOL			__stdcall			SendToServer(DWORD dwConnectionIndex,PACKET_LIST* pList,DWORD flag);
	virtual BOOL			__stdcall			SendToUser(DWORD dwConnectionIndex,PACKET_LIST* pList,DWORD flag);

protected:
	SServerInitDesc m_stDesc;
};

CNetbase* CreateNetbaseWrapper();
void DestroyNetbaseWrapper(CNetbase* pNet);
//////////////////////////////////////////////////////////////////////////
#endif