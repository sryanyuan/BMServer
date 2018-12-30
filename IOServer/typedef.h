#pragma once

#ifndef _TYPEDEF_HEAD_NETWORK
#define _TYPEDEF_HEAD_NETWORK

#include <winsock2.h>
#include <windows.h>

typedef void (__stdcall *ACCEPTFUNC)(DWORD);
typedef void (__stdcall *RECVFUNC)(DWORD dwConnectionIndex,char* pMsg,DWORD dwLength);
			  
typedef void (__stdcall *VOIDFUNC)(void);
typedef	void (__stdcall *EVENTCALLBACK)(DWORD);
typedef void (__stdcall *CONNECTSUCCESSFUNC)(DWORD dwConnectionIndex,void* pVoid);
typedef void(__stdcall *CONNECTFAILFUNC)(DWORD dwConnectionIndex, void* pVoid);
typedef void (__stdcall *DISCONNECTFUNC)(DWORD dwConnectionIndex);
typedef void (__stdcall *EVENTFUNC)(DWORD dwEventIndex );

#define	NETDDSC_ENCRYPTION			0x00000001
#define NETDDSC_DEBUG_LOG			0x00000010


enum FLAG_SEND
{
	
	FLAG_SEND_ENCRYPTION		=	0x00000001,
	FLAG_SEND_NOT_ENCRYPTION	=	0x00000000
};

struct CUSTOM_EVENT
{   
    DWORD		dwPeriodicTime;
	EVENTFUNC	pEventFunc;
	
	
};

struct INET_BUF
{
    u_long      len;     /* the length of the buffer */
    char FAR *  buf;     /* the pointer to the buffer */
};

struct PACKET_LIST
{
	char*			pMsg;
	DWORD			dwLen;
	PACKET_LIST*	pNext;
	DWORD			dwFlag;
};

struct DESC_NETWORK
{
    DWORD			dwMaxUserNum;
    DWORD			dwMaxServerNum;
	RECVFUNC		OnRecvFromUserTCP;
	RECVFUNC		OnRecvFromServerTCP;
	ACCEPTFUNC		OnAcceptUser;
	ACCEPTFUNC		OnAcceptServer;
	DISCONNECTFUNC	OnDisconnectUser;
	DISCONNECTFUNC	OnDisconnectServer;
	
    DWORD			dwServerMaxTransferSize;
    DWORD			dwUserMaxTransferSize;
    DWORD			dwServerBufferSizePerConnection;
    DWORD			dwUserBufferSizePerConnection;
    DWORD			dwMainMsgQueMaxBufferSize;
	DWORD			dwConnectNumAtSameTime;
    DWORD			dwFlag;
    DWORD			dwCustomDefineEventNum;
	
    CUSTOM_EVENT*	pEvent;
};
   
#endif//#define _TYPEDEF_HEAD_NETWORK