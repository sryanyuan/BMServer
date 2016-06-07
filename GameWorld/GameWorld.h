#ifndef _INC_GAMEWORLD_
#define _INC_GAMEWORLD_
//////////////////////////////////////////////////////////////////////////
#include "ObjectEngine.h"
#include "Struct.h"
#include <map>
#include <Windows.h>
#include "../../CommonModule/ByteBuffer.h"
#include "../../CommonModule/cron/CronSchedule.h"
#include "DBThread.h"
//#include "../../CommonModule/ScriptEngine.h"
#include "LuaServerEngine.h"
#include "GameDbBuffer.h"
#include "WorldEventDispatcher.h"
//////////////////////////////////////////////////////////////////////////
//	For encrypt
#ifdef _THEMIDA_

#include "../Themida/ThemidaSDK.h"

#define PROTECT_START_VM	VM_START
#define PROTECT_END_VM		VM_END

#else

	#ifdef _SHIELDEN_

		#include "../Shielden/C/SESDK.h"

		#define PROTECT_START_VM	SE_PROTECT_START_VIRTUALIZATION
		#define PROTECT_END_VM		SE_PROTECT_END

	#else
		#define PROTECT_START_VM	
		#define PROTECT_END_VM
	#endif

#endif
//////////////////////////////////////////////////////////////////////////
#define THREAD_CALL __stdcall
//////////////////////////////////////////////////////////////////////////
typedef std::map<int, GameObject*> ObjectMap;
typedef std::map<unsigned int, unsigned int> Connection2ID;
//////////////////////////////////////////////////////////////////////////
#define MAX_COMMAND		80960

#define ALERT_MSGBOX(T)		::MessageBox(NULL, T, "ALERT", MB_ICONERROR | MB_TASKMODAL)

#define ITEMTAG_INQUERY	0xFFFFFFFF

#define MAX_CONNECTIONS	500

#define STOP_EXETOOLONG		3
#define STOP_HEROSPDERR		1
#define STOP_HEROATTRIBERR	2
#define STOP_HPZERONOTDEAD	4
#define STOP_MONSNOTDEAD	5
//////////////////////////////////////////////////////////////////////////
extern HeroObject* g_pxHeros[MAX_CONNECTIONS + 1];
extern std::vector<MagicInfo> g_xMagicInfoTable;
extern std::map<int, int> g_xShowDonateTimeMap;
//////////////////////////////////////////////////////////////////////////
enum WORLD_STATE
{
	WS_STOP,
	WS_WORKING,
	WS_PAUSE,
};

enum BUFFER_OPERATE
{
	BO_NEWOBJECT = 0,
	BO_DELOBJECT,
};

struct BlackListItem
{
	std::string xName;
	BYTE bJob;
	int nLevel;
	BYTE bSex;
};

typedef std::list<BlackListItem> BlackList;
//////////////////////////////////////////////////////////////////////////
class GameWorld
{
public:
	GameWorld();
	~GameWorld();

public:
	static GameWorld* GetInstancePtr();
	static GameWorld& GetInstance()
	{
		return *GetInstancePtr();
	}
	static void DestroyInstance();

private:
	static unsigned int THREAD_CALL WorkThread(void* _pData);
	static int __cronActive(int _nJobId, int _nArg);

public:
	//	world control
	void Initialize(const char* _pszDestFile = NULL);
	unsigned int Run();
	void Stop()
	{
		if(m_eState == WS_STOP)
		{
			return;
		}
		m_bTerminate = true;
	}
	void Stop(int _ret)
	{
		return;
		if(m_eState == WS_STOP)
		{
			return;
		}
		m_nRetCode = _ret;
		m_bTerminate = true;
	}
	void Terminate(int _nRet)
	{
		if(m_eState == WS_STOP)
		{
			return;
		}
		m_nRetCode = _nRet;
		m_bTerminate = true;
	}
	void Pause()
	{
		if(m_eState == WS_WORKING)
		{
			m_bPause = true;
		}
	}
	void Resume()
	{
		if(m_eState == WS_PAUSE)
		{
			m_bPause = false;
		}
	}
	inline WORLD_STATE GetWorldState()
	{
		return m_eState;
	}

	//	get the id
	inline unsigned int GenerateObjectID()
	{
		static unsigned int s_uID = 0;
		return ++s_uID;
	}
	inline unsigned int GenerateItemTag()
	{
		static unsigned int s_uID = 0;
		return ++s_uID;
	}
	inline unsigned int GenerateMapID()
	{
		static unsigned int s_uID = 0;
		return ++s_uID;
	}

	//	sync object
	inline void LockProcess()
	{
		EnterCriticalSection(&m_stCsProcess);
	}
	inline void UnlockProcess()
	{
		LeaveCriticalSection(&m_stCsProcess);
	}

	//	Only for main thread
	void OnMessage(unsigned int _uId, const void* _pData, unsigned int _uLen);
	void OnMessage(unsigned int _dwIndex, unsigned int _uId, ByteBuffer* _xBuffer);
	void AddDelayedProcess(const DelayedProcess* _pDp);
	//	can't call on main thread
	GameObject* GetPlayer(unsigned int _uId);
	GameObject* GetNPC(unsigned int _uId);

	//	Broadcast msg
	unsigned int Broadcast(ByteBuffer* _pBuf);
	unsigned int BroadcastExcept(GameObject* _pObj, ByteBuffer* _pBuf);
	unsigned int BroadcastRange(ByteBuffer* _pBuf, DWORD _dwSrcX, DWORD _dwSrcY, DWORD _dwOftX = 10, DWORD _dwOftY = 10);
	unsigned int BroadcastRangeExcept(GameObject* _pObj, ByteBuffer* _pBuf, DWORD _dwSrcX, DWORD _dwSrcY, DWORD _dwOftX = 10, DWORD _dwOftY = 10);

	//	script function
	/*inline ScriptEngine* GetScript()
	{
		return &m_xScript;
	}*/
	inline LuaServerEngine* GetLuaEngine()
	{
		return &m_xScript;
	}
	inline lua_State* GetLuaState()
	{
		return m_xScript.GetVM();
	}
	inline void DispatchLuaEvent(int _nEvtId, void* _pEvt)
	{
		m_xScript.DispatchEvent(_nEvtId, _pEvt);
	}
	bool LoadScript();
	bool ReloadScript();
	bool LoadBlackList();

	inline const char* GetRankListData()
	{
		return m_pRankListData;
	}

	void PostRunMessage(const MSG* _pMsg)
	{
		if(m_hThread != NULL &&
			m_dwThreadID != 0)
		{
			PostThreadMessage(m_dwThreadID, _pMsg->message, _pMsg->wParam, _pMsg->lParam);
		}
	}

	void SetSchedule(int _nEventId, const char* _pszCronExpr);
	void ResetSchedule(int _nEventId);

public:
	//	working process
	void DoWork_Objects(DWORD _dwTick);
	void DoWork_System(DWORD _dwTick);
	void DoWork_DelayedProcess(DWORD _dwTick);
	unsigned int Thread_ProcessMessage(const MSG* _pMsg);

private:
	bool NewPlayer(GameObject* _pObj);
	bool RemovePlayer(unsigned int _uId);
	bool NewNPC(unsigned int _uId);
	void RemoveNPC(unsigned int _uId);

private:
	void DoDelayGame(const DelayedProcess& _dp);
	void DoDelayDatabase(const DelayedProcess& _dp);
	void DoDelaySystem(const DelayedProcess& _dp);

public:
	bool LoadMagicInfo();
	void UpgradeItems(ItemAttrib* _pItem);
	void UpgradeItems(ItemAttrib* _pItem, int _nProb);
	bool UpgradeAttrib(ItemAttrib* _pItem, int _index, int _value);
	void UpgradeItemsWithAddition(ItemAttrib* _pItem, int _nAddition);
	void SetItemHideAttrib(ItemAttrib* _pItem);
	void SetMaterialItemQuality(ItemAttrib* _pItem);

public:
	inline int GetOnlinePlayerCount()
	{
		return m_nOnlinePlayers;
	}
	inline int GetExprMultiple()	{return m_nExprMultiple;}
	inline int GetDropMultiple()	{return m_nDropMultiple;}
	inline void SetExprMultiple(int _nMult)			{m_nExprMultiple = _nMult;}
	inline void SetDropMultiple(int _nMult)			{m_nDropMultiple = _nMult;}
	inline void EnableAutoReset()					{m_bEnableAutoReset = true;}
	inline void DisableAutoReset()					{m_bEnableAutoReset = false;}
	inline bool GetAutoReset()						{return m_bEnableAutoReset;}

	inline bool IsGenElitMons()						{return m_bGenElitMons;}
	inline void SetGenElitMons(bool _b)				{m_bGenElitMons = _b;}

	inline bool IsEnableOfflineSell()				{return m_bEnableOfflineSell;}
	inline void SetEnableOfflineSell(bool _b)		{m_bEnableOfflineSell = _b;}

	inline bool IsEnableWorldNotify()				{return m_bEnableWorldNotify;}
	inline void SetEnableWorldNotify(bool _b)		{m_bEnableWorldNotify = _b;}

	inline void SetFinnalExprMulti(int _nMulti)		{m_nFinnalExprMulti = _nMulti;}
	inline int GetFinnalExprMulti()					{return m_nFinnalExprMulti;}

	bool IsInBlackList(HeroObject* _pHero);

	inline std::string& GetExpFireworkUserName()	{return m_xExpFireworkUserName;}
	inline std::string& GetBurstFireworkUserName()	{return m_xBurstFireworkUserName;}
	inline std::string& GetMagicDropFireworkUserName()	{return m_xMagicDropFireworkUserName;}
	inline void SetExpFireworkUserName(const char* _pszUserName){
		m_xExpFireworkUserName = _pszUserName;
	}
	inline void SetBurstFireworkUserName(const char* _pszUserName){
		m_xBurstFireworkUserName = _pszUserName;
	}
	inline void SetMagicDropFireworkUserName(const char* _pszUserName){
		m_xMagicDropFireworkUserName = _pszUserName;
	}
	inline DWORD GetExpFireworkTime()				{return m_dwLastExpFireworkTime;}
	inline void SetExpFireworkTime(DWORD _dwTime)	{m_dwLastExpFireworkTime = _dwTime;}
	inline DWORD GetBurstFireworkTime()				{return m_dwLastBurstFireworkTime;}
	inline void SetBurstFireworkTime(DWORD _dwTime)	{m_dwLastBurstFireworkTime = _dwTime;}
	inline DWORD GetMagicDropFireworkTime()			{return m_dwLastMagicDropFireworkTime;}
	inline void SetMagicDropFireworkTime(DWORD _dwTime)	{m_dwLastMagicDropFireworkTime = _dwTime;}

	inline int GetDifficultyLevel()					{return m_nDifficultyLevel;}
	inline void SetDifficultyLevel(int _nLevel)		{m_nDifficultyLevel = _nLevel;}

private:
	bool m_bTerminate;
	bool m_bPause;
	WORLD_STATE m_eState;

	//	Monsters and NPC
	ObjectMap m_xNPC;
	//	Players
	ObjectMap m_xPlayers;

	//	For working thread
	DWORD m_dwThreadID;
	HANDLE m_hThread;
	DWORD m_dwWorkTotalTime;

	//	delayed command buffer
	ByteBuffer m_xProcessD;

	//	translate connection index to object id
	//Connection2ID m_xTranslateTable;

	//	Database thread
	DBThread* m_xDatabase;
	//	World script
	LuaServerEngine m_xScript;

	static int m_nRetCode;

	//	Expr multiple and Drop multiple
	int m_nDropMultiple;
	int m_nExprMultiple;
	//	Special map support resetting?
	bool m_bEnableAutoReset;

	BlackList m_xBlackList;
	//	Last receive watcher msg time
	DWORD m_dwLastRecWatcherMsgTime;

	bool m_bGenElitMons;
	bool m_bEnableOfflineSell;
	bool m_bEnableWorldNotify;

	int m_nFinnalExprMulti;

	//	排行榜的字符串指针 new出来 需要delete
	char* m_pRankListData;

	//	放烟花产生的状态
	DWORD m_dwLastExpFireworkTime;
	std::string m_xExpFireworkUserName;

	DWORD m_dwLastBurstFireworkTime;
	std::string m_xBurstFireworkUserName;

	DWORD m_dwLastMagicDropFireworkTime;
	std::string m_xMagicDropFireworkUserName;

	//	更新脚本引擎的时间
	DWORD m_dwLastUpdateScriptEngineTime;

	//	游戏难度
	int m_nDifficultyLevel;

protected:
	CRITICAL_SECTION m_stCsProcess;

public:
	//	For interlock use
	int m_nOnlinePlayers;

	//	cron调度器
	CronJobScheduler m_xCronScheduler;
};
//////////////////////////////////////////////////////////////////////////
#endif