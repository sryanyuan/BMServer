#ifndef _INC_LUASERVERENGINE_
#define _INC_LUASERVERENGINE_
//////////////////////////////////////////////////////////////////////////
#include "../../CommonModule/LuaBaseEngine.h"
#include <string>
using std::string;
//////////////////////////////////////////////////////////////////////////
TOLUA_API int tolua_BackMirServer_open (lua_State* tolua_S);
//////////////////////////////////////////////////////////////////////////
#define LOADMODE_PATH		0
#define LOADMODE_ZIP		1
#define LOADMODE_NPK		2
//////////////////////////////////////////////////////////////////////////
class NPCObject;
class HeroObject;
class GameScene;

enum LuaEvent
{
	kLuaEvent_None,
	//	世界更新 1s调用一次
	kLuaEvent_WorldUpdate,
	//	定时任务触发
	kLuaEvent_WorldScheduleActive,
	//	NPC点击
	kLuaEvent_WorldNPCActive,
	//	世界启动
	kLuaEvent_WorldStartRunning,
	//	进入场景
	kLuaEvent_ScenePlayerEnter,
	//	连接上了登陆服务器
	kLuaEvent_WorldLoginServerConnected,
	// 离开场景
	kLuaEvent_ScenePlayerLeave,
};

struct LuaEvent_ScenePlayerEnter
{
	GameScene* pScene;
	HeroObject* pHero;
};

struct LuaEvent_ScenePlayerLeave
{
	GameScene* pPrevScene;
	GameScene* pNewScene;
	HeroObject* pHero;
};

struct LuaEvent_WorldScheduleActive
{
	int nScheduleId;

	LuaEvent_WorldScheduleActive()
	{
		nScheduleId = 0;
	}
};

struct LuaEvent_WorldNPCActive
{
	int nButtonId;
	NPCObject* pNPC;
	HeroObject* pHero;

	LuaEvent_WorldNPCActive()
	{
		nButtonId = 0;
		pNPC = NULL;
		pHero = NULL;
	}
};
//////////////////////////////////////////////////////////////////////////
class LuaServerEngine : public LuaBaseEngine
{
public:
	LuaServerEngine();
	virtual ~LuaServerEngine(){}

public:
	bool ExecuteZip(const char* _pszFileName, const char* _pszSubFile);
	bool ExecuteFile(const char* _pszFileName);

	//bool LoadModule(const char* _pszModuleFile);

	virtual int OnDispatchEvent(const LuaDispatchEvent* _pEvent, LuaDispatchInfo* _pInfo);
	virtual void OnDispatchEventResult(const LuaDispatchEvent* _pEvent, bool bResult);
	
public:
	inline int GetUserTag()
	{
		return m_nUserTag;
	}
	inline void SetUserTag(int _nTag)
	{
		m_nUserTag = _nTag;
	}
	inline void SetModulePath(const char* _pszPath, int _nMode = LOADMODE_PATH)
	{
		m_xModulePath = _pszPath;
		m_nLoadMode = _nMode;
	}
	inline const string& GetModulePath()
	{
		return m_xModulePath;
	}

public:
	virtual void Output(const char* _pszLog);

protected:
	int m_nUserTag;
	string m_xModulePath;
	int m_nLoadMode;
};
//////////////////////////////////////////////////////////////////////////
#endif