#ifndef _INC_GAMESCENEMANAGER_
#define _INC_GAMESCENEMANAGER_
//////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <map>
#include "../../BackMirClient/Common/MirMap.h"
//#include "../../CommonModule/ScriptEngine.h"
#include "LuaServerEngine.h"
#include "../../CommonModule/ByteBuffer.h"
#include "../../CommonModule/SaveFile.h"
#include "../../CommonModule/GDefine.h"
#include "../../CommonModule/SimpleIni.h"
#include "SceneEvent.h"
#include "FreeListManager.h"
//////////////////////////////////////////////////////////////////////////
#define MAX_SCENE_NUMBER		60/*60个地图场景*/
#define EXECUTE_SCRIPT_INTERVAL	2*1000

#define OBJECT_MASK				0x20000000

#define CELL_NONE				0
#define CELL_MOVEOBJECT			1
#define CELL_ITEMS				2
#define CELL_MAGIC				3
#define CELL_DOOR				4


#define MAPPEDOBJECT_PLAYER			(1 << 0)
#define MAPPEDOBJECT_MONSTER		(1 << 1)
#define MAPPEDOBJECT_SLAVE			(1 << 2)
#define MAPPEDOBJECT_STATUE_ALIVE	(1 << 3)
#define MAPPEDOBJECT_ALL			(MAPPEDOBJECT_PLAYER | MAPPEDOBJECT_MONSTER | MAPPEDOBJECT_SLAVE)
#define MAPPEDOBJECT_ALL_ALIVE		(MAPPEDOBJECT_PLAYER | MAPPEDOBJECT_MONSTER | MAPPEDOBJECT_SLAVE | MAPPEDOBJECT_STATUE_ALIVE)
//////////////////////////////////////////////////////////////////////////
enum MapPkType
{
	//	安全地图
	kMapPkType_None,
	//	Pk地图
	kMapPkType_All,
};
//////////////////////////////////////////////////////////////////////////
struct CellData
{
	void* pData;
	BYTE bType;
	DWORD dwInfo;
};

typedef std::list<CellData*> CELLDATALIST;

struct MapCellInfo
{
	//	0x80 for block
	BYTE bFlag;
	CELLDATALIST* pCellObjects;
};

struct StaticMagic
{
	WORD wMgcID;
	WORD wMinDC;
	WORD wMaxDC;
	DWORD dwFire;
	DWORD dwEffectActive;
	DWORD dwEffectActiveCount;
	short sPosX;
	short sPoxY;
	DWORD dwEnableTime;
	DWORD dwExpire;
	WORD wMgcLevel;
	GameObject* pFire;
};

struct DoorEvent
{
	WORD wMapID;
	WORD wOX;
	WORD wOY;
	WORD wPosX;
	WORD wPoxY;
	DWORD dwTime;
};

struct MonsterGenerateInfo
{
	DWORD dwPos;
	BYTE bNumber;
	BYTE bOft;
	BYTE bMonsGenerateType;
	DWORD dwInterval;
	DWORD dwMonsID;
	DWORD dwLastExecuteTime;
};

typedef std::list<StaticMagic*> STATICMAGICLIST;
typedef std::list<DoorEvent*>	DOOREVENTLIST;
typedef std::list<DWORD>		DOORPOSITIONLIST;
typedef std::list<MonsterGenerateInfo*> MONSTERGENERATEINFOLIST;
typedef std::list<GameObject*> GameObjectList;
//////////////////////////////////////////////////////////////////////////
class HeroObject;
class GameObject;
class GameSceneManager;
class ByteBuffer;
struct GroundItem;
class NPCObject;
class MonsterObject;
//////////////////////////////////////////////////////////////////////////
extern const int g_nSearchPoint[98];
//////////////////////////////////////////////////////////////////////////

class GameScene
{
	friend class GameSceneManager;

public:
	GameScene();
	virtual ~GameScene();

public:
	virtual bool Initialize(DWORD _dwMapID);
	virtual void Release();

public:
	virtual void Update(DWORD _dwTick);

public:
	bool InsertPlayer(GameObject* _pObj);
	bool RemovePlayer(DWORD _dwID, bool _bDelete = true);
	GameObject* GetPlayer(DWORD _dwID);
	GameObject* GetPlayerByName(const char* _pszName);
	GameObject* GetPlayerWithoutLock(DWORD _dwID);
	bool InsertNPC(GameObject* _pNPC);
	bool RemoveNPC(DWORD _dwID, bool _bDelete = true);
	bool InsertItem(GroundItem* _pItem, bool _bCopy = true);
	bool RemoveItem(DWORD _dwItemUniqueID);
	GroundItem* GetItem(DWORD _dwTag);
	GameObject* GetMonster(int _nAttribID);
	int MoveSomeMonsterTo(int _nAttribID,int _nSum, int _x, int _y);

	DWORD BroadcastPacket(ByteBuffer* _pBuf, DWORD _dwIgnoreID = 0);
	DWORD BroadcastPacketRange(ByteBuffer* _pBuf, RECT& rcRange, DWORD _dwIgnoreID = 0);

	//	For broadcasting in a range
	//DWORD BroadcastPacketRange(ByteBuffer* _pBuf, DWORD _dwIgnoreID = 0);
	bool GetSceneData(HeroObject* _pObj);
	bool GetRandomPosition(DWORD* _pOut);
	bool GetDropPosition(WORD _wX, WORD _wY, DWORD* _pOut);
	//DWORD GetMapData(int _x, int _y);
	MapCellInfo* GetMapData(int _x, int _y);
	void UpdateObjectBlockInfo();
	bool GetAroundCanMove(int _nX, int _nY, int* _pX, int* _pY);

	bool IsUserNameExist(const char* _pszName);

	bool CanMove(int _x, int _y);
	bool CanThrough(int _x, int _y);
	bool CanDrop(int _x, int _y);
	bool CanCross(int _sx, int _sy, int _dx, int _dy);

	bool IsInCity(WORD _wPosX, WORD _wPosY);
	inline WORD GetCityCenterX()			{return LOWORD(m_dwCityCenter);}
	inline WORD GetCityCenterY()			{return HIWORD(m_dwCityCenter);}
	inline WORD GetMapID()					{return m_dwMapID;}
	inline int GetMapIDInt()				{return m_dwMapID;}
	inline DWORD GetMapUID()				{return m_dwMapUID;}

	inline int GetTimeLimitID()				{return m_nTimeLimitID;}
	inline void SetTimeLimitID(int _id)		{m_nTimeLimitID = _id;}
	inline int GetTimeLimitSec()			{return m_nTimeLimitSec;}
	inline void SetTimeLimitSec(int _sec)	{m_nTimeLimitSec = _sec;}

	inline MapPkType GetMapPkType()			{return m_eMapPkType;}
	inline void SetMapPkType(MapPkType _eT)	{m_eMapPkType = _eT;}

	bool LoadScript();
	bool ReloadScript();
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

	bool PushStaticMagic(const StaticMagic* _pMgc);
	void HandleStaticMagic();
	void ParseStaticMagic(const StaticMagic* _pMgc);
	void RemoveStaticMagicByFire(GameObject* _pFire);
	void RestoreStaticMagicByFire(GameObject* _pFire);

	void CheckGroundItems();
	inline const MapInfo& GetMapInfo()		{return m_stMapInfo;}

	void UpdateDoorEvent();

	//////////////////////////////////////////////////////////////////////////
	//	For script
	void BroadcastChatMessage(std::string& _xMsg, unsigned int _dwExtra);
	void BroadcastChatMessage(const char* _pszMsg, unsigned int _dwExtra);
	void BroadcastSceneSystemMessage(const char* _pszMsg);
	void BroadcastSystemNotify(const char* _pszMsg);
	int GetPlayerSum();
	int GetSlaveSum();
	int GetMonsterSum(unsigned int _uID);
	void CreateNPC(unsigned int _uID, unsigned short _uX, unsigned short _uY);
	//	Normal,elite and leader monster
	int CreateMonster(unsigned int _uID, unsigned short _uX, unsigned short _uY);
	//	Just elite monster
	int CreateEliteMonster(unsigned int _uID, unsigned short _uX, unsigned short _uY);
	//	Just leader monster
	int CreateLeaderMonster(unsigned int _uID, unsigned short _uX, unsigned short _uY);
	//	Just normal monster
	int CreateNormalMonster(unsigned int _uID, unsigned short _uX, unsigned short _uY);
	//	Random position
	int CreateMonster(unsigned int _uID);
	int CreateNormalMonster(unsigned int _uID);
	int CreateEliteMonster(unsigned int _uID);
	int CreateLeaderMonster(unsigned int _uID);
	//	Generate monster task
	void CreateMonster(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval);
	void CreateEliteMonster(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval);
	void CreateLeaderMonster(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval);
	void CreateMonsterLater(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval);
	void CreateEliteMonsterLater(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval);
	void CreateLeaderMonsterLater(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval);

	void OnMonsterCreated(unsigned int _uID, MonsterObject* _pMonster);
	void CreateGroundItem(unsigned _uID, unsigned short _uX, unsigned short _uY);
	void CreateGroundItem(unsigned _uID, unsigned short _uX, unsigned short _uY, int _nExtraProb);
	void InsertGroundItem(GroundItem* _pItem);
	void CreateDoorEvent(unsigned int _uMapID, unsigned int _ux, unsigned int _uy, unsigned int _nextx, unsigned int _nexty);
	void CreateDoorEvent(unsigned int _uMapID, unsigned int _ux, unsigned int _uy, unsigned int _nextx, unsigned int _nexty, unsigned int _utime);
	void CreateDoorEventWithNotification(unsigned int _uMapID, unsigned int _ux, unsigned int _uy, unsigned int _nextx, unsigned int _nexty, unsigned int _utime);
	NPCObject* GetNPCByID(unsigned int _uID);
	GameObject* GetNPCByHandleID(unsigned int _uID);
	GameObject* GetOnePlayerInRange(RECT& _rc, bool _bCanSeeHide = false);
	GameObject* MonsterGetOneTargetInRange(const RECT& _rc, bool _bCanSeeHide = false);
	GameObject* SlaveGetOneTargetInRange(const RECT& _rc, bool _bCanSeeHide = false);
	void SetCityRange(int _x, int _y, int _width, int _height, int _centerx, int _centery);
	inline void SetAutoReset()		{m_bAutoReset = true;}
	MonsterObject* NewMonsterByID(int _nAttribID);

	//	NOT:因为DROPITEMLOADED后 假如有很多爆的物品 则GameWorld占了GameWorld的锁，而又调用了CreateGroundItem，导致了DBThread
	//	也占了DBThread的锁，而DBThread又要GameWorld的锁，gameworld也要dbthread锁 造成了死锁，故加入链表，等释放锁后再处理
	//	NewGroundItem ，仅仅记录信息 id posx posy
	void NewGroundItem(int _id, int _x, int _y);
	void HandleNewItem();

	bool IsItemInGround(int _nAttribID, int _x, int _y);
	void RemoveGroundItem(int _nAttribID, int _x, int _y);


	inline int GetPlayerCount()						{return m_nCurPlayers;}
	inline int GetMonsterCount()					{return m_nCurMonsters;}
	inline bool IsAutoReset()						{return m_bAutoReset;}
	inline void SetCannotStay()						{m_bSaveAndStay = false;}
	inline bool CanSaveAndStay()					{return m_bSaveAndStay;}
	inline void SetCannotUseScroll()				{m_bCanUseScroll = false;}
	inline bool CanUseScroll()						{return m_bCanUseScroll;}
	inline bool CanUseMove()						{return m_bCanUseMove;}
	inline void SetCanUseMove(bool _b)				{m_bCanUseMove = _b;}

	//	Instance
	inline bool IsInstance()						{return m_bIsInstance;}
	inline void SetInstance(bool _bIns)				{m_bIsInstance = _bIns;}

	//	All treasure map relative
	inline bool IsTreasureMap()						{return m_bIsTreasureMap;}
	inline void SetTreasureMap(bool _b)				{m_bIsTreasureMap = _b;}
	void DeleteAllMonster();
	void DeleteAllItem();
	inline void ResetGiveReward()					{m_bGiveReward = false;m_bKilledMonster = false;}
	inline void SetKilledMonster()					{m_bKilledMonster = true;}

	inline float GetMonsterDefenceMulti()			{return m_fSceneMonsterDefenceMulti;}
	inline void SetMonsterDefenceMulti(float _fMulti)	{m_fSceneMonsterDefenceMulti = _fMulti;}
	inline float GetMonsterAttackMulti()			{return m_fSceneMonsterAttackMulti;}
	inline void SetMonsterAttackMulti(float _fMulti)	{m_fSceneMonsterAttackMulti = _fMulti;}

	inline DWORD GetTimeCount()						{return GetTickCount();}

	void EraseTarget(GameObject* _pObj);

	DoorEvent* GetDoorEvent(int _nX, int _nY);

	void ChallengeResetScene(int _nClgID);
	void ChallengeSuccess(int _nItemID);
	int GetChallengeID()							{return m_nChallengeID;}
	void AllHeroFlyToHome();

	void ProcessWaitInsert();
	void ProcessWaitRemove();
	void ProcessWaitDelete();

public:
	void PushDelayBuf(DelaySendInfo& _inf);
	void PushDelayBuf(SceneDelayMsg* _pMsg);

protected:
	void FlushDelayBuf();
	void ParseDelayInfo(DelaySendInfo& _inf);
	void ParseDelayInfo(SceneDelayMsg* _pMsg);

public:
	void ClearGroundItem(DWORD _dwExpire);
	void AutoGenerateMonster();
	void AllMonsterHPToFull();
public:
	//	Not:Don't use those
	bool RemoveMappedObject(int _x, int _y, void* _pData);
	bool AddMappedObject(int _x, int _y,BYTE _type, void* _pData);
	bool MoveMappedObject(int _sx, int _sy, int _dx, int _dy, void* _pData);
	void GetMapObjects(int _x, int _y, int _oft, GameObjectList& _xObjList);
	int GetMappedObjects(int _x, int _y, int _ofx, int _ofy, GameObjectList& _refList, int _nTypeFlag = MAPPEDOBJECT_ALL);

	inline void AddWaitInsert(GameObject* _pObj)	{m_xWaitInsertPlayers.push_back(_pObj);}
	inline void AddWaitRemove(GameObject* _pObj)	{m_xWaitRemovePlayers.push_back(_pObj);}
	inline void AddWaitDelete(GameObject* _pObj)	{m_xWaitDeletePlayers.push_back(_pObj);}

public:
	//	for scene event register, lua
	void RegisterCallback(SceneEvent _eType, const char* _pszFuncName, int _nInterval);

protected:
	//DWORD* m_pMapData;
	MapCellInfo* m_pCellData;
	MapInfo m_stMapInfo;
	std::map<DWORD, GameObject*> m_xPlayers;

	std::list<GameObject*> m_xWaitRemovePlayers;
	std::list<GameObject*> m_xWaitInsertPlayers;
	std::list<GameObject*> m_xWaitDeletePlayers;

	std::map<DWORD, GameObject*> m_xNPCs;
	std::map<DWORD, GroundItem*> m_xItems;
	CRITICAL_SECTION m_csPlayer;

	LuaServerEngine m_xScript;
	DWORD m_dwExecuteScriptInterval;
	DWORD m_dwMapID;

	DWORD m_dwLoopTime;
	//DWORD m_dwLastUpdateObjectBlockTime;

	//	Delay send packet
	ByteBuffer m_xDelaySendBuf;
	ByteBuffer m_xWaitDelay;

	std::list<int> m_xNewGroundItem;

	//	static magic
	STATICMAGICLIST m_xStaticMagic;
	DWORD m_dwLastUpdateMagicTime;
	//	door event
	DOOREVENTLIST m_xDoorEvts;
	DOORPOSITIONLIST m_xDoorPos;
	//	monster generate controls
	MONSTERGENERATEINFOLIST m_xMonsterGenerator;
	DWORD m_dwLastGenerateMonsterTime;
	//	clean items
	DWORD m_dwLastCleanItemTime;

	//	City map info
	RECT m_stCityRect;
	DWORD m_dwCityCenter;

	//	Log out and return to full HP?
	bool m_bAutoReset;
	int m_nCurPlayers;
	int m_nCurMonsters;
	bool m_bSaveAndStay;

	//	A instance scene?
	bool m_bIsInstance;

	//	A treasure map?
	bool m_bIsTreasureMap;
	bool m_bGiveReward;
	bool m_bKilledMonster;
	DWORD m_dwTreasureTipTime;

	bool m_bCanUseScroll;

	bool m_bCanUseMove;

	//	Unique map id
	DWORD m_dwMapUID;

//#ifdef _FREELIST_
	//	Msg queue
	SceneDelayMsgList m_xSceneDelayMsgList;
//#endif
	SceneEventExecutor m_xSceneEventExecutor;

	float m_fSceneMonsterDefenceMulti;
	float m_fSceneMonsterAttackMulti;

	//	挑战地图的ID
	int m_nChallengeID;

	//	限制人物在地图内时间
	int m_nTimeLimitID;
	int m_nTimeLimitSec;

	//	Pk模式
	MapPkType m_eMapPkType;
};

//////////////////////////////////////////////////////////////////////////
#define UPDATEPLAYERCOUNT_INTERVAL	(8 * 1000)
//////////////////////////////////////////////////////////////////////////
class GameInstanceScene;
typedef std::list<GameInstanceScene*> INSTANCEMAPLIST;

class GameSceneManager
{
protected:
	GameSceneManager();
public:
	~GameSceneManager();
	static GameSceneManager* GetInstance();
	static void DestroyInstance();

public:
	bool CreateAllScene();
	void ReleaseAllScene();

	void ReloadScript();

	bool InsertPlayer(GameObject* _pObj);
	bool InsertNPC(GameObject* _pObj);
	bool RemovePlayer(DWORD _dwID);

	GameObject* GetPlayer(WORD _wMapID, DWORD _dwID);
	GameObject* GetPlayer(DWORD _dwID);
	void GetPlayerByUid(int _nUid, GameObjectList& _refList);
	GameObject* GetPlayerByName(const char* _pszName)
	{
		GameObject* pObj = NULL;

		for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
		{
			if(m_pScenes[i])
			{
				pObj = m_pScenes[i]->GetPlayerByName(_pszName);
				if(pObj)
				{
					return pObj;
				}
			}
			else
			{
				break;
			}
		}

		return NULL;
	}

	inline int CountPlayer()
	{
		int nPlayerCounter = 0;

		for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
		{
			if(m_pScenes[i])
			{
				nPlayerCounter += m_pScenes[i]->GetPlayerCount();
			}
			else
			{
				break;
			}
		}

		return nPlayerCounter;
	}

	inline int CountMonster()
	{
		int nMonsterCounter = 0;

		for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
		{
			if(m_pScenes[i])
			{
				nMonsterCounter += m_pScenes[i]->GetMonsterCount();
			}
			else
			{
				break;
			}
		}

		return nMonsterCounter;
	}

	inline int GetMonsterSum(int _id)
	{
		int nMonsterCounter = 0;

		for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
		{
			if(m_pScenes[i])
			{
				nMonsterCounter += m_pScenes[i]->GetMonsterSum(_id);
			}
			else
			{
				break;
			}
		}

		return nMonsterCounter;
	}

	void AllMonsterHPToFull()
	{
		for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
		{
			if(m_pScenes[i])
			{
				m_pScenes[i]->AllMonsterHPToFull();
			}
			else
			{
				break;
			}
		}
	}

	void BroadcastPacketAllScene(ByteBuffer* _xBuf, DWORD _dwIgnore = 0)
	{
		for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
		{
			if(m_pScenes[i])
			{
				m_pScenes[i]->BroadcastPacket(_xBuf, _dwIgnore);
			}
			else
			{
				break;
			}
		}
	}

	void SendRawSystemMessageAllScene(const char* _pszMsg)
	{
		for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
		{
			if(m_pScenes[i])
			{
				m_pScenes[i]->BroadcastChatMessage(_pszMsg, 1);
				//m_pScenes[i]->BroadcastSceneSystemMessage(_pszMsg);
			}
			else
			{
				break;
			}
		}
	}

	void SendSystemMessageAllScene(const char* _pszMsg)
	{
		for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
		{
			if(m_pScenes[i])
			{
				//m_pScenes[i]->BroadcastChatMessage(_pszMsg, 1);
				m_pScenes[i]->BroadcastSceneSystemMessage(_pszMsg);
			}
			else
			{
				break;
			}
		}
	}

	void SendSystemNotifyAllScene(const char* _pszMsg)
	{
		for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
		{
			if(m_pScenes[i])
			{
				//m_pScenes[i]->BroadcastChatMessage(_pszMsg, 1);
				m_pScenes[i]->BroadcastSystemNotify(_pszMsg);
			}
			else
			{
				break;
			}
		}
	}

	GameInstanceScene* GetFreeInstanceScene(int _id);
	
public:
	inline GameScene* GetScene(DWORD _dwMapID)
	{
		if(_dwMapID >= MAX_SCENE_NUMBER)
		{
			return NULL;
		}
		return m_pScenes[_dwMapID];
	}
	inline GameScene* GetSceneInt(int _nMapID)
	{
		return GetScene(_nMapID);
	}
	void Update(DWORD _dwTick);
	inline const char* GetRunMap(int _id)
	{
		if(_id < 0 ||
			_id >= m_xRunMapData.size())
		{
			return NULL;
		}
		return m_xRunMapData[_id].c_str();
	}
	const char* GetMapChName(int _id);
	inline const char* GetInstanceMap(int _id)
	{
		INSTANCEMAPDATA::const_iterator fndIter = m_xInsMapData.find(_id);
		if(fndIter != m_xInsMapData.end())
		{
			return fndIter->second.c_str();
		}

		return NULL;
	}
	inline bool IsUserNameExist(const char* _pszName)
	{
		bool bExist = false;

		for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
		{
			if(m_pScenes[i])
			{
				if(m_pScenes[i]->IsUserNameExist(_pszName))
				{
					bExist = true;
					break;
				}
			}
			else
			{
				break;
			}
		}

		return bExist;
	}

protected:
	GameScene* m_pScenes[MAX_SCENE_NUMBER];
	DWORD m_dwLoadedMap;
	RUNMAPDATA m_xRunMapData;

	INSTANCEMAPLIST m_xInsMaps;
	INSTANCEMAPDATA m_xInsMapData;

	DWORD m_dwLastNotifyPlayerCount;
	CSimpleIniA m_xIniMapName;
};

//////////////////////////////////////////////////////////////////////////
#endif