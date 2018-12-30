// SServerEngine.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
#include "SServerEngine.h"
#include "NetbaseWrapper.h"
#include "Logger.h"

#ifndef _LIB

int nLibEventThreadStatus = 0;
SServerEngine eng;
NetbaseWrapper svr;

void printThreadId()
{
	DWORD dwTid = GetCurrentThreadId();
	printf("Thread id %d \n", dwTid);
}

void __stdcall onAcceptUser(unsigned int _index)
{
	LOGINFO("user %d connected", _index);
}

void __stdcall onAcceptServer(unsigned int _index)
{
	LOGINFO("server %d connected", _index);
}

void __stdcall onDisconnectedUser(unsigned int _index)
{
	LOGINFO("user %d disconnected", _index);
}

void __stdcall onDisconnectedServer(unsigned int _index)
{
	LOGINFO("server%d disconnected", _index);
}

void __stdcall onConnectSuccess(unsigned int _index, void* _arg)
{
	LOGINFO("connect success %d", _index);
}

void __stdcall onConnectFailed(unsigned int _index, void* _arg)
{
	LOGINFO("connect failed %d", _index);
}

void __stdcall onRecvUser(unsigned int _index, char* _data, unsigned int _len)
{
	static char tx[1024];
	strncpy(tx, _data, _len);
	printThreadId();
	LOGINFO("%d recv user %s length %d", _index, tx, _len);

	if('Q' == _data[0])
	{
		eng.CloseUserConnection(_index);
	}
	else if('T' == _data[0])
	{
		eng.SyncSendPacketToUser(_index, tx, _len);
	}
	else if('C' == _data[0])
	{
		eng.SyncConnect("127.0.0.1", 8000, onConnectSuccess, onConnectFailed, NULL);
	}
	else if('D' == _data[0])
	{
		eng.SyncConnect("127.0.0.1", 2223, onConnectSuccess, onConnectFailed, NULL);
	}
	else if('S' == _data[0])
	{
		eng.Stop();
	}
	/*if('Q' == _data[0])
	{
		svr.CompulsiveDisconnectUser(_index);
	}
	else if('T' == _data[0])
	{
		svr.SendToServer(_index, tx, _len, 0);
	}
	else if('C' == _data[0])
	{
		svr.ConnectToServerWithServerSide("127.0.0.1", 2222, CONNECTSUCCESSFUNC(onConnectSuccess), CONNECTFAILFUNC(onConnectFailed), NULL);
	}
	else if('D' == _data[0])
	{
		svr.ConnectToServerWithServerSide("127.0.0.1", 2223, CONNECTSUCCESSFUNC(onConnectSuccess), CONNECTFAILFUNC(onConnectFailed), NULL);
	}*/
}

void __stdcall onRecvServer(unsigned int _index, char* _data, unsigned int _len)
{
	static char tx[1024];
	strncpy(tx, _data, _len);
	printThreadId();
	LOGINFO("%d recv server %s length %d", _index, tx, _len);

	if('Q' == _data[0])
	{
		eng.CloseUserConnection(_index);
	}
	else if('T' == _data[0])
	{
		eng.SendPacketToUser(_index, tx, _len);
	}
	else if('C' == _data[0])
	{
		eng.Connect("127.0.0.1", 8000, onConnectSuccess, onConnectFailed, NULL);
	}
}

void __stdcall onTimer(unsigned int _id)
{
	static int s_nLastTimerTime = 0;

	if (s_nLastTimerTime == 0)
	{
		s_nLastTimerTime = GetTickCount();
	}

	if (GetTickCount() - s_nLastTimerTime > 1000 + 1000)
	{
		LOGERROR("timer timeout ?");
	}
	s_nLastTimerTime = GetTickCount();
}

int main(int argc, char* argv[])
{
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0202, &wsa_data);
#endif
	printThreadId();

	SServerInitDesc desc;
	desc.uMaxConnUser = 5;
	desc.bUseIOCP = false;
	desc.pFuncOnAcceptUser = onAcceptUser;
	desc.pFuncOnDisconnctedUser = onDisconnectedUser;
	desc.pFuncOnDisconnctedServer = onDisconnectedServer;
	desc.pFuncOnRecvUser = onRecvUser;
	desc.pFuncOnRecvServer = onRecvServer;

	eng.Init(&desc);
	int nRet = eng.Start("127.0.0.1", 4444);

	eng.AddTimerJob(1, 1000, onTimer);
	
	if(nRet != kSServerResult_Ok)
	{
		LOGPRINT("Start server failed.Error:%d", nRet);
		exit(1);
	}
	/*DESC_NETWORK desc = {0};
	desc.dwMaxUserNum = 5;
	desc.OnAcceptServer = (ACCEPTFUNC)onAcceptServer;
	desc.OnAcceptUser = (ACCEPTFUNC)onAcceptUser;
	desc.OnDisconnectServer = (DISCONNECTFUNC)onDisconnectedServer;
	desc.OnDisconnectUser = (DISCONNECTFUNC)onDisconnectedUser;
	desc.OnRecvFromServerTCP = (RECVFUNC)onRecvServer;
	desc.OnRecvFromUserTCP = (RECVFUNC)onRecvUser;

	svr.CreateNetwork(&desc, 0, 0);
	svr.StartServerWithServerSide("127.0.0.1", 4444);*/

	for(;;)
	{
		Sleep(10);
	}

	return 0;
}

#endif