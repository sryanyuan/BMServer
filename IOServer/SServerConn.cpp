#include "SServerConn.h"
#include "Logger.h"
#include "SServerEngine.h"
//////////////////////////////////////////////////////////////////////////
SServerConn::SServerConn()
{
	pEng = NULL;
	pEv = NULL;
	uConnIndex = 0;
	fd = 0;
	m_uPacketHeadLength = 0;
	m_xReadBuffer.AllocBuffer(5 * 1024);
	memset(&m_stAddress, 0, sizeof(sockaddr_in));
	bServerConn = false;
	eConnState = kSServerConnState_None;

	m_fnOnConnectSuccess = NULL;
	m_fnOnConnectFailed = NULL;
	m_pConnectResultArg = NULL;
}

SServerConn::~SServerConn()
{

}


void SServerConn::readHead()
{
	evbuffer* pInput = bufferevent_get_input(pEv);
	size_t uLen = evbuffer_get_length(pInput);

	if(uLen < DEF_NETPROTOCOL_HEADER_LENGTH)
	{
		LOGERROR("Head length invalid %d", uLen);
		pEng->onConnectionClosed(this);
		return;
	}

	unsigned int uHeadLength = 0;
	evbuffer_copyout(pInput, &uHeadLength, DEF_NETPROTOCOL_HEADER_LENGTH);
	evbuffer_drain(pInput, DEF_NETPROTOCOL_HEADER_LENGTH);
	m_uPacketHeadLength = (unsigned int)ntohl(uHeadLength);

	//	next we should read body
	if(m_uPacketHeadLength <= DEF_NETPROTOCOL_HEADER_LENGTH)
	{
		LOGERROR("Head length content invalid %d", m_uPacketHeadLength);
		pEng->onConnectionClosed(this);
		return;
	}

	// check the packet length limit
	size_t uMaxPacketLength = pEng->GetMaxPacketLength();
	if (0 != uMaxPacketLength &&
		m_uPacketHeadLength > uMaxPacketLength)
	{
		LOGERROR("Packet length of conn %d out of limit, conn %d");
		pEng->onConnectionClosed(this);
		return;
	}

	//	is data full to read body?
	if(uLen >= m_uPacketHeadLength)
	{
		readBody();
	}
	else
	{
		//	wait read body
		bufferevent_setwatermark(pEv, EV_READ, m_uPacketHeadLength - DEF_NETPROTOCOL_HEADER_LENGTH, 0);
	}
}

void SServerConn::readBody()
{
	//	make sure we had read packet length
	if(m_uPacketHeadLength <= DEF_NETPROTOCOL_HEADER_LENGTH)
	{
		LOGERROR("Invalid body length %d", m_uPacketHeadLength);
		pEng->onConnectionClosed(this);
		return;
	}

	unsigned int uBodyLength = m_uPacketHeadLength - DEF_NETPROTOCOL_HEADER_LENGTH;

	evbuffer* pInput = bufferevent_get_input(pEv);
	size_t uLen = evbuffer_get_length(pInput);

	// not receive full packet
	if(uLen < uBodyLength)
	{
		// not receive full body , continue recv ...
		LOGERROR("Invalid read body length %d", uLen);
		pEng->CloseUserConnection(uConnIndex);
		return;
	}

	//	read the buffer to read buffer
	if(m_xReadBuffer.GetAvailableSize() < uBodyLength)
	{
		m_xReadBuffer.ReallocBuffer();
	}

	evbuffer_copyout(pInput, m_xReadBuffer.GetFreeBufferPtr(), uBodyLength);
	evbuffer_drain(pInput, uBodyLength);
	m_xReadBuffer.Rewind();

	//	callback
	if(bServerConn)
	{
		pEng->Callback_OnRecvServer(uConnIndex, m_xReadBuffer.GetReadableBufferPtr(), uBodyLength);
	}
	else
	{
		pEng->Callback_OnRecvUser(uConnIndex, m_xReadBuffer.GetReadableBufferPtr(), uBodyLength);
	}
	m_xReadBuffer.Reset();

	//	continue read head
	m_uPacketHeadLength = 0;

	//	check next packet
	uLen -= uBodyLength;
	if(uLen >= DEF_NETPROTOCOL_HEADER_LENGTH)
	{
		readHead();
	}
	else
	{
		//	wait read head
		bufferevent_setwatermark(pEv, EV_READ, DEF_NETPROTOCOL_HEADER_LENGTH, 0);
	}
}

void SServerConn::SetAddress(const sockaddr_in* _pAddr)
{
	memcpy(&m_stAddress, _pAddr, sizeof(sockaddr_in));
}

sockaddr_in* SServerConn::GetAddress()
{
	return &m_stAddress;
}

bool SServerConn::GetAddress(char* _pBuffer, unsigned short* _pPort)
{
	*_pPort = 0;
	memset(_pBuffer, 0, 15);

	char* paddr = inet_ntoa(m_stAddress.sin_addr);
	if(NULL == paddr)
	{
		return false;
	}

	memcpy(_pBuffer, paddr, lstrlen(paddr) + 1);
	*_pPort = ntohs(m_stAddress.sin_port);
	return true;
}

void SServerConn::Callback_OnConnectSuccess()
{
	if(NULL == m_fnOnConnectSuccess)
	{
		return;
	}
	m_fnOnConnectSuccess(uConnIndex, m_pConnectResultArg);
}

void SServerConn::Callback_OnConnectFailed()
{
	if(NULL == m_fnOnConnectFailed)
	{
		return;
	}
	m_fnOnConnectFailed(uConnIndex, m_pConnectResultArg);
}