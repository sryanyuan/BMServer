#ifndef _INC_MESSAGEBOARD_
#define _INC_MESSAGEBOARD_

#include <string>

struct ServerState {
	unsigned short wOnline;
	unsigned char bMode;
	unsigned int uDistinctIPCount;
	unsigned int uHeroCount;
	unsigned int uMonsCount;

	ServerState() {
		wOnline = 0;
		bMode = 0;
		uDistinctIPCount = 0;
		uHeroCount = 0;
		uMonsCount = 0;
	}
};

struct ServerBaseInfo {
	int nServerID;
	char strServerName[20];
	char szServeIP[20];

	ServerBaseInfo() {
		nServerID = 0;
		strServerName[0] = 0;
		szServeIP[0] = 0;
	}
};

class ServerShell {
public:
	// Server base information
	virtual const ServerBaseInfo* GetServerBaseInfo() = 0;
	// AddInformation to message board, it must be thread safe
	// The life time of message board must be longer than CMainServer
	virtual void AddInformation(const char* _szText) = 0;
	virtual void UpdateServerState(const ServerState* _pState) = 0;
	virtual const char* GetConfig(const char* _pszKey) = 0;
	virtual const char* GetRootPath() = 0;
};

#endif
