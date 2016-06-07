#ifndef CUser_H_
#define CUser_H_

#include <Windows.h>
#include <map>
#include "../../CommonModule/GDefine.h"

class CMainServer;
struct PacketBase;
/************************************************************************/
/* 服务器连接玩家
/************************************************************************/
class CUser
{
	friend class CMainServer;

public:
	CUser(DWORD _dwIndex);
	virtual ~CUser();

public:
	virtual void OnConnect(){}
	virtual void OnDisconnect(){}
	virtual void OnMessage(PacketBase* _pPacket);

public:
	inline DWORD GetUserIndex()						{return m_dwIndex;}

private:
	DWORD m_dwIndex;
};

typedef std::map<DWORD, CUser*> PlayerMap;

/************************************************************************/
/* 游戏玩家
/************************************************************************/
class CGameUser : public CUser
{
public:
	CGameUser(DWORD _dwIndex);
	~CGameUser();

public:
	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual void OnMessage(PacketBase* _pPacket);
};

#endif