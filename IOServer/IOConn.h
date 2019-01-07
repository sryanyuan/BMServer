#ifndef _INC_SSERVERCONN_
#define _INC_SSERVERCONN_
//////////////////////////////////////////////////////////////////////////
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <string>
#include "IOBuffer.h"
#include "Def.h"
#include "namespace.h"
//////////////////////////////////////////////////////////////////////////
struct sockaddr_in;

IONS_START

class IOServer;
//////////////////////////////////////////////////////////////////////////
enum IOConnStateType
{
	IOConnState_None,
	IOConnState_Connecting,
	IOConnState_Connected,
	IOConnState_ConnectFailed,
	IOConnState_Disconnected
};
//////////////////////////////////////////////////////////////////////////

struct AddressInfo {
	std::string strIP;
	unsigned short uPort;

	AddressInfo() {
		uPort = 0;
	}

	void SetAddress(const sockaddr_in *_pAddr);
};
//////////////////////////////////////////////////////////////////////////
#define IOCONN_FLAG	BEV_OPT_CLOSE_ON_FREE
//////////////////////////////////////////////////////////////////////////
class IOConn
{
	friend class IOServer;

public:
	IOConn();
	~IOConn();

public:
	void SetAddress(const sockaddr_in* _pAddr);
	const AddressInfo* GetAddress();
	bool GetAddress(char* _pBuffer, unsigned short* _pPort);

	void Callback_OnConnectSuccess();
	void Callback_OnConnectFailed();

private:
	void readHead();
	void readBody();

private:
	IOServer* pEng;
	bufferevent* pEv;
	unsigned int uConnIndex;
	evutil_socket_t fd;
	bool bServerConn;
	IOConnStateType eConnState;

	unsigned int m_uPacketHeadLength;

	IOBuffer m_xReadBuffer;

	AddressInfo m_stAddress;

	//	for connect to server
	FUNC_ONCONNECTFAILED m_fnOnConnectFailed;
	FUNC_ONCONNECTSUCCESS m_fnOnConnectSuccess;
	void* m_pConnectResultArg;
};
//////////////////////////////////////////////////////////////////////////
IONS_END

#endif