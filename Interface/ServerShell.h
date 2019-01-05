#ifndef _INC_MESSAGEBOARD_
#define _INC_MESSAGEBOARD_

class ServerShell {
public:
	// AddInformation to message board, it must be thread safe
	// The life time of message board must be longer than CMainServer
	virtual void AddInformation(const char* _szText) = 0;
	virtual void UpdateDistinctIPCount(int _nCnt) = 0;
	virtual void UpdateObjectCount(int _nHero, int _nMons) = 0;
};

#endif
