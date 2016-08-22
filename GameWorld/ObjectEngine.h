#ifndef _INC_OBJECTENGINE_
#define _INC_OBJECTENGINE_
//////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include "../../CommonModule/ByteBuffer.h"
#include "../../CommonModule/GamePacket.h"
#include "../../CommonModule/ObjectData.h"
#include "../../CommonModule/MagicEffectID.h"
#include "../../CommonModule/ShareData.h"
#include "../CMainServer/CUser.h"
#include <stdlib.h>
#include <string>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include "../../CommonModule/QuestContext.h"
#include "ObjectValid.h"
#include "../../CommonModule/ShareDefine.h"
#include "../../CommonModule/ConsoleHelper.h"
#include "../../CommonModule/RandGenerator.h"
#include "../../CommonModule/StoveManager.h"
#include "FreeListManager.h"
//////////////////////////////////////////////////////////////////////////
extern ByteBuffer g_xThreadBuffer;
extern ConsoleHelper g_xConsole;
class GameWorld;
class GameScene;
class MonsterObject;
class HeroObject;
class ObjectValid;
class ObjectStatusControl;
class GameInstanceScene;

unsigned int SendBuffer(unsigned int _nIdx, ByteBuffer* _pBuf);
unsigned int SendBufferToServer(unsigned int _nIdx, ByteBuffer* _pBuf);
void MirLog(const char* _pLog);
void ConsolePrint(const char* _pszText);

extern const int g_nMoveOft[16];
//////////////////////////////////////////////////////////////////////////
#define OBJECT_BUFFER_SIZE	1*1024
#define OBJECT_MSGQUEUE_SIZE	1*1024
#define PLAYER_BAG_SIZE		(6 + 36)
#define HP_SUPPLY_NUMBER	4
#define MP_SUPPLY_NUMBER	4

#define STONE_PROPERTY		16
#define STONE_TIME			4000

#define TIME_KICK			50

#define PRISON_MAP_ID		27

#define TAG_CRCVERIFY	

#ifdef _DEBUG
#define DESTORY_GAMEWORLD		
#else
#define DESTORY_GAMEWORLD		memset(GameWorld::GetInstancePtr(), rand() % 256, sizeof(GameWorld));
#endif

#ifdef _DEBUG
#define DESTORY_GAMESCENE		
#else
#define DESTORY_GAMESCENE		memset(GameSceneManager::GetInstance(), rand() % 256, sizeof(GameWorld));
#endif

#define IMPLEMENT_RANDOM_ABILITY(NAME)\
	WORD GetRandom##NAME()\
	{\
		WORD wMin = m_stAbli.w##NAME;\
		WORD wMax = m_stAbli.wMax##NAME;\
		if(wMin > wMax)\
		{\
			std::swap(wMin, wMax);\
		}\
		int nVar = (int)(wMin + rand() % (wMax - wMin));\
		if(nVar < 0)\
		{\
			return 0;\
		}\
		return (WORD)nVar;\
	}

template<typename T>
T GetRandomIn(T _x, T _y)
{
	T minvar = _x;
	T maxvar = _y;

	if(_x == 0 &&
		_y == 0)
	{
		return 0;
	}
	if(_x == _y)
	{
		return _x;
	}

	if(minvar > maxvar)
	{
		std::swap(minvar, maxvar);
		//return 0;
	}

	T var = (T)(minvar + rand() % (maxvar - minvar + 1));
	return var;
}

#define MAKE_POSITION_DWORD(obj)	MAKELONG(obj->GetUserData()->wCoordX, obj->GetUserData()->wCoordY)


#define DO_PACKET(PACKET_NAME)		DoPacket(*static_cast<PACKET_NAME*>(_pPkt))
#define BEGIN_HANDLE_PACKET(pkt)	switch(pkt->uOp){
#define HANDLE_PACKET(op,pkt)		case op:{DO_PACKET(pkt);}break;
#define END_HANDLE_PACKET()			}

#define DO_PROCESS_PACKET(pkt)		pkt pt;m_xMsgQueue >> pt;ProcessPacket(&pt);
#define BEGIN_PROCESS_PACKET(pkt)	switch(pkt->uOp){
#define PROCESS_PACKET(op,pkt)		case op:{DO_PROCESS_PACKET(pkt);}break;
#define PROCESS_DEFAULT()			default:{m_xMsgQueue.SetHeadOffset(pHeader->uLen);LOG(ERROR) << "Unknown opcode[" << pHeader->uOp << "]";}break;
#define END_PROCESS_PACKET()		}

//	with buffer
#define DO_PROCESS_PACKET_WITH_BUF(pkt, buf)					pkt pt;buf >> pt;ProcessPacket(&pt);
#define PROCESS_PACKET_WITH_BUF(op, pkg, buf)		case op:{DO_PROCESS_PACKET_WITH_BUF(pkg, buf);}break;

//#define DO_PROCES_MAGIC(pkt)		switch(pkg.uParam1){
//#define PROCESS_MAGIC(ID,)

#define PVP_FACTOR_NORMAL					4
#define PVP_FACTOR_MAGIC					5
//////////////////////////////////////////////////////////////////////////

// enum USER_STATE
// {
// 	US_NOTHING,
// 	US_LOGIN,
// 	US_PLAY,
// };
//////////////////////////////////////////////////////////////////////////
struct MagicInfo
{
	WORD wID;
	WORD wLevel[7];
	BYTE bJob;
	BYTE bMultiple;
	WORD wIncrease;
	BYTE bBaseCost;
	DWORD dwDelay;
};

struct UserMagic
{
	const MagicInfo* pInfo;
	BYTE bLevel;
};

typedef std::vector<UserMagic> USERMAGICVECTOR;

//////////////////////////////////////////////////////////////////////////
class LockObject
{
public:
	LockObject()
	{
		InitializeCriticalSection(&m_stCS);
	}
	virtual ~LockObject()
	{
		DeleteCriticalSection(&m_stCS);
	}

public:
	void Lock()
	{
		EnterCriticalSection(&m_stCS);
	}
	void Unlock()
	{
		LeaveCriticalSection(&m_stCS);
	}

protected:
	CRITICAL_SECTION m_stCS;
};
//////////////////////////////////////////////////////////////////////////
struct ReceiveDamageInfo
{
	bool bFarPalsy;
	bool bIgnoreAC;
	int nStoneTime;
	bool bIgnoreMagicShield;
	int nMagicId;
	char bMagicLevel;
};
//////////////////////////////////////////////////////////////////////////

class GameObject  : public LockObject/*, public CUser*/
{
	friend class GameWorld;

public:
	GameObject(/*DWORD _dwID*/);
	virtual ~GameObject();

public:
	//	For all objects to update themselves,implement method for executing normally
	virtual void DoWork(unsigned int _dwTick) {}
	//	Process the packet that have read from buffer
	virtual void ProcessPacket(PacketHeader* _pPkt);

	//	Handle the message queue
	virtual bool DoMsgQueue(unsigned int _dwTick);

	//	dispatch the packet to ProcessPacket
	virtual bool DispatchPacket(ByteBuffer& _refBuf, const PacketHeader *_pHeader);
	bool DispatchPacket(ByteBuffer& _refBuf);

	//	Receive damage(unhandled damage)
	//	Attacked by monster
	virtual int ReceiveDamage(GameObject* _pAttacker, bool _bMgcAtk, int _oriDC = 0, int _nDelay = 500, const ReceiveDamageInfo* _pInfo = NULL){return 0;}
	//	Attacked by hero
	virtual void ReceiveHeroDamage(GameObject* _pAttacker, int _nDamage, int _delay){}

	//	Add delayed process,call it on main loop thread,don't call it on main thread
	//	Now can only call in main loop thread
	void AddProcess(const PacketHeader* _pPkt);
	bool AddProcess(ByteBuffer& _xBuf);

	inline ObjectValid* GetValidCheck()				{return m_pValid;}
	inline void SetID(unsigned int _uId)			{m_uID = _uId;}
	inline unsigned int GetID()						{return m_uID;}
	inline SERVER_OBJECT_TYPE	GetType()					{return m_eType;}
	inline ItemAttrib* GetAttrib()					{return &m_stData.stAttrib;}
	inline WORD GetMapID()							{return m_stData.wMapID;}
	inline void SetMapID(WORD _wMapID)				{m_stData.wMapID = _wMapID;}
	inline UserData* GetUserData()					{return &m_stData;}
	inline int GetAttribID()						{return GetObject_ID();}
	inline int GetLevel()							{return GetObject_Level();}
	inline bool CanStoneRestore()					{return m_bStoneRestore;}
	inline void SetStoneRestore(bool _b)			{m_bStoneRestore = _b;}
	inline int GetState()							{return (int)m_stData.eGameState;}

	inline void SetValidPositon(WORD _wX, WORD _wY)		{m_wLastValidPositionX = _wX;m_wLastValidPositionY = _wY;}
	inline WORD GetValidPositionX()						{return m_wLastValidPositionX;}
	inline WORD GetValidPositionY()						{return m_wLastValidPositionY;}
	inline void SetDefectTime(DWORD _dwTime)							{m_dwDefectTime = _dwTime;}

	inline WORD GetCoordX()							{return m_stData.wCoordX;}
	inline WORD GetCoordY()							{return m_stData.wCoordY;}

	GameScene* GetLocateScene();

	inline StateController* GetStateController()	{return &m_xStates;}

//	IMPLEMENT_RANDOM_ABILITY(DC);
//	IMPLEMENT_RANDOM_ABILITY(AC);
//	IMPLEMENT_RANDOM_ABILITY(SC);
//	IMPLEMENT_RANDOM_ABILITY(MC);
//	IMPLEMENT_RANDOM_ABILITY(MAC);
	inline DWORD GetLastAttackTime()				{return m_dwLastAttackTime;}
	inline void SetLastAttackTime(DWORD _dwTime)	{m_dwLastAttackTime = _dwTime;}

	//	just export to lua
	inline int GetCoordXInt()						{return (int)m_stData.wCoordX;}
	inline int GetCoordYInt()						{return (int)m_stData.wCoordY;}
	inline int GetTotalRecvDamage()					{return m_nTotalRecvDamage;}
	inline int GetHP()								{return GetObject_HP();}
	inline int GetMaxHP()							{return GetObject_MaxHP();}
	inline int GetExpr()							{return GetObject_Expr();}

	const char* GetName();

	//	delay action list
	void AddDelayAction(DelayActionBase* _pAction);
	virtual void ProcessDelayAction(){}
	void ClearDelayAction();

public:
	bool IncHP(DWORD _dwHP);
	bool DecHP(DWORD _dwHP);
	bool IncMP(DWORD _dwMP);
	bool DecMP(DWORD _dwMP);
	void SyncHP(HeroObject* _pHero);
	void SyncMP(HeroObject* _pHero);

	virtual void EncryptObject();
	inline void SetEncrypt()			{m_stData.stAttrib.extra |= EXTRA_MASK_ENCRYPT;}
	inline bool IsEncrypt()				{return TEST_FLAG_BOOL(m_stData.stAttrib.extra, EXTRA_MASK_ENCRYPT);}

public:
	//	Get Attrib
	SETGET_OBJECT_ATBE(id, ID);
	SETGET_OBJECT_ATBE(lucky, Lucky);
	SETGET_OBJECT_ATBE(curse, Curse);
	SETGET_OBJECT_ATBE(hide, Hide);
	SETGET_OBJECT_ATBE(accuracy, Accuracy);
	SETGET_OBJECT_ATBE(atkSpeed, AtkSpeed);
	SETGET_OBJECT_ATBE(atkPalsy, AtkPalsy);
	SETGET_OBJECT_ATBE(atkPois, AtkPois);
	SETGET_OBJECT_ATBE(moveSpeed, MoveSpeed);
	SETGET_OBJECT_ATBE(weight, Weight);
	SETGET_OBJECT_ATBE(reqType, ReqType);
	SETGET_OBJECT_ATBE(reqValue, ReqValue);
	SETGET_OBJECT_ATBE(sex, Sex);
	SETGET_OBJECT_ATBE(type, Type);
	SETGET_OBJECT_ATBE(maxDC, MaxDC);
	SETGET_OBJECT_ATBE(DC, DC);
	SETGET_OBJECT_ATBE(maxAC, MaxAC);
	SETGET_OBJECT_ATBE(AC, AC);
	SETGET_OBJECT_ATBE(maxMAC, MaxMAC);
	SETGET_OBJECT_ATBE(MAC, MAC);
	SETGET_OBJECT_ATBE(maxSC, MaxSC);
	SETGET_OBJECT_ATBE(SC, SC);
	SETGET_OBJECT_ATBE(maxMC, MaxMC);
	SETGET_OBJECT_ATBE(MC, MC);
	SETGET_OBJECT_ATBE(maxHP, MaxHP);
	SETGET_OBJECT_ATBE(HP, HP);
	SETGET_OBJECT_ATBE(maxMP, MaxMP);
	SETGET_OBJECT_ATBE(MP, MP);
	SETGET_OBJECT_ATBE(maxEXPR, MaxExpr);
	SETGET_OBJECT_ATBE(EXPR, Expr);
	SETGET_OBJECT_ATBE(level, Level);
	SETGET_OBJECT_ATBE(tex, Tex);

public:
	virtual int GetRandomAbility(ABILITY_TYPE _type);
	bool IsMissed(GameObject* _pTarget);

public:
	//	Basic action
	bool WalkOneStep(int _nDrt);
	bool RunOneStep(int _nDrt);
	bool TurnTo(int _nDrt);
	bool FlyTo(int _x, int _y);
	bool FlyToMap(int _x, int _y, int _nMapID);
	bool FlyToInstanceMap(int _x, int _y, GameInstanceScene* _pInsScene);

	//	Valid positon
	void ForceGotoValidPosition();

public:
	void ResetSupply();
	bool AddDrugState(int _total, int _type);
	bool AddHealState(int _total);
	bool AddEnergyShieldState(int _total, int _step);
	virtual void UpdateStatus(DWORD _dwCurTick);
	virtual void SetEffStatus(DWORD _effMask, DWORD _time, DWORD _param);
	void ResetEffStatus(DWORD _effMask);

	inline bool IsHide()								{return TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_HIDE);}
	inline bool IsFrozen()								{return TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_ICE);}

	inline bool IsElite()								{return TEST_FLAG_BOOL(GetObject_MaxExpr(), MAXEXPR_MASK_ELITE);}
	inline bool IsLeader()								{return TEST_FLAG_BOOL(GetObject_MaxExpr(), MAXEXPR_MASK_LEADER);}
	inline bool IsBoss()								{return TEST_FLAG_BOOL(GetObject_MaxExpr(), MAXEXPR_MASK_BOSS);}

	inline DWORD GetHumEffFlag()						{return m_dwHumEffFlag;}
	inline DWORD GetHumEffTime(int _nIndex)
	{
		if(_nIndex < 0 ||
			_nIndex >= MMASK_SERVER_TOTAL)
		{
			return 0;
		}

		return m_dwHumEffTime[_nIndex];
	}

protected:
	//	唯一ID
	unsigned int m_uID;

	DWORD m_dwLastExeTime;
	DWORD m_dwTotalExeTime;

	SERVER_OBJECT_TYPE m_eType;
	//ObjectAbility m_stAbli;

	//	time counter
	DWORD m_dwLastMoveTime;
	DWORD m_dwLastAttackTime;
	DWORD m_dwLastDeathTime;

	UserData m_stData;

	//	the object message queue, stored serialized packet
	ByteBuffer m_xMsgQueue;

	//	Timer counter
	DWORD m_dwLastWalkTime;
	DWORD m_dwWalkInterval;
	//DWORD m_dwLastAttackTime;
	DWORD m_dwLastStruckTime;
	DWORD m_dwLastDeadTime;
	DWORD m_dwLastSearchTime;

	DWORD m_dwLastRunTime;
	DWORD m_dwRunInterval;

	DWORD m_dwCurrentTime;


	WORD m_wLastValidPositionX;
	WORD m_wLastValidPositionY;

	//	magic effect,such as poison
	DWORD m_dwHumEffFlag;
	DWORD m_dwHumEffTime[MMASK_SERVER_TOTAL];

	//	drug
	//	supply
	DWORD m_dwSupply[HP_SUPPLY_NUMBER + MP_SUPPLY_NUMBER];
	DWORD m_dwHealSupply[HP_SUPPLY_NUMBER];
	DWORD m_dwEnergyShieldSupply[HP_SUPPLY_NUMBER];
	DWORD m_dwLastIncHPTime;
	DWORD m_dwLastHealIncHPTime;
	DWORD m_dwLastIncMPTime;
	//	defence of the MEFF_CHARMAC skill
	DWORD m_dwACInrease;
	//	poison per second damage
	DWORD m_dwPoison;
	DWORD m_dwLastUpdatePoisonTime;
	//	Auto add hp mp
	DWORD m_dwLastAddHpMpTime;
	//	time to defect master
	DWORD m_dwDefectTime;

	//	IsValid?
	ObjectValid* m_pValid;
	DWORD m_dwLastCheckValidTime;

	//
	bool m_bStoneRestore;
	//	All status
	ObjectStatusControl* m_pStatusCtrl;

	//	State control
	StateController m_xStates;

	DWORD m_dwInvalidMsgQueueTimes;
	bool m_bNetDataValid;

	int m_nTotalRecvDamage;
	//	中毒是否生效
	bool m_bCanPosion;
	//	恢复体力的时间间隔
	DWORD m_dwHPRecoverInterval;

	DelayActionList m_xDelayActions;
};

//////////////////////////////////////////////////////////////////////////
class NPCObject : public GameObject
{
public:
	NPCObject(/*DWORD _dwID*/);
	virtual ~NPCObject();

public:
	virtual void DoWork(unsigned int _dwTick);
	//	handle packet
	virtual void ProcessPacket(PacketHeader* _pPkt);

public:
	//	export to lua script
	inline void ClearAllSellItem()			{memset(m_sSellItems, 0, sizeof(m_sSellItems));m_nCurItems = 0;}
	void AddSellItem(int _id);
	bool IsItemExist(int _id);
	inline USHORT* GetItems()				{return m_sSellItems;}
	inline int GetItemSum()					{return m_nCurItems;}

	inline void SetWorldNPC(bool _b)		{m_bWorldNPC = _b;}
	inline bool IsWorldNPC()				{return m_bWorldNPC;}

public:
	void DoPacket(const PkgPlayerClickNPCReq& req);

protected:
	USHORT m_sSellItems[72];
	int m_nCurItems;
	bool m_bWorldNPC;
};
//////////////////////////////////////////////////////////////////////////
/*
class SortAttribIDByTag
{
public:
	bool operator()(const ItemAttrib* _pLeft, const ItemAttrib* _pRight) const
	{
		return _pLeft->id < _pRight->id;
	}
};*/
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

typedef std::vector<int> GiftItemIDVector;
typedef std::map<int, DWORD> DrugUseTimeMap;

#define MAX_SLAVE_SUM	3
#define MAX_STORE_NUMBER	36
#define MAX_BIGSTORE_NUMBER	80
class HeroObject : public GameObject/*, public CUser*/
{
public:
	//	constructor and destructor
	HeroObject(DWORD _dwID);
	virtual ~HeroObject();

public:
	//	For all objects to update themselves,implement method for normal execute
	virtual void DoWork(unsigned int _dwTick);
	//	process ai
	virtual void DoAction(unsigned int _dwTick);
	//	send all delayed packet
	virtual void DoSendDelayedPackets();
	//	handle packet
	virtual void ProcessPacket(PacketHeader* _pPkt);

	virtual int ReceiveDamage(GameObject* _pAttacker, bool _bMgcAtk, int _oriDC = 0, int _nDelay = 500, const ReceiveDamageInfo* _pInfo = NULL);

	//	executed by main thread, do not call it on main loop thread
	unsigned int PushMessage(const void* _pData, unsigned int _uLen);
	unsigned int PushMessage(ByteBuffer* _xBuf);
	//	append delayed packet
	unsigned int AppendPacket(const PacketHeader* _pPkt);
	unsigned int AppendBuffer(ByteBuffer* _pBuf);
	//	send
	virtual unsigned int SendMsgToUser(const void* _pData, unsigned int _uLen);

	//	事件回调
	virtual void OnPlayerLevelUp(int _nCurrentLevel);
	virtual void OnPlayerDressdItem(int _nTag);
	virtual void OnPlayerLogin();

	virtual void SetEffStatus(DWORD _effMask, DWORD _time, DWORD _param);

	virtual void ProcessDelayAction();


	bool SendPlayerBuffer(ByteBuffer& _xBuf);
	template <typename T>
	bool SendPacket(T pct)
	{
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << pct;
		return SendPlayerBuffer(g_xThreadBuffer);
	}
	//	user index
	inline DWORD GetUserIndex()		{return m_dwIndex;}
	bool AcceptLogin(bool _bNew);
	void VerifyLogin();
	bool WriteHumDataToBuffer(std::vector<char>& _refCharVector, bool _bHideIdentify = false, bool _bSaveCannotSaveItem = false);
	bool WriteHumBigStoreDataToBuffer(std::vector<char>& _refCharVector, bool _bHideIdentify = false, bool _bSaveCannotSaveItem = false);
	//void SendHumData(bool _bSendToClient, bool _bSendToLogin);
	void SendHumDataV2(bool _bSendToClient, bool _bSendToLogin);

	void CheckDoorEvent();

	virtual int GetRandomAbility(ABILITY_TYPE _type);
	virtual void UpdateStatus(DWORD _dwCurTick);

	virtual void EncryptObject();

	inline void SetVersion(USHORT _uVersion)				{m_uVersion = _uVersion;}
	inline USHORT GetVersion()								{return m_uVersion;}

	void OnItemDataLoaded(ItemAttrib* _pItem);

	inline DWORD GetLastRecvDataTime()						{return m_dwLastRecvDataTime;}
	inline void SetLastRecvDataTime(DWORD _dwTime)			{m_dwLastRecvDataTime = _dwTime;}

	void GetStatusInfo(PkgPlayerGStatusNtf& ntf);
	void SendStatusInfo();
	void BroadcastStatusInfo();

	inline void SetUnuse(bool _b)							{m_bUnUse = _b;}
	inline bool GetUnuse()									{return m_bUnUse;}

	inline bool IsSmallQuit()								{return m_bSmallQuit;}
	inline void SetSmallQuit(bool _b)						{m_bSmallQuit = _b;}

	inline int GetDonateMoney()								{return m_nDonateMoney;}
	inline void SetDonateMoney(int _nMoney)					{m_nDonateMoney = _nMoney;}
	inline int GetDonateLeft()								{return m_nDonateLeft;}
	inline void SetDonateLeft(int _nMoney)					{m_nDonateLeft = _nMoney;}
	inline int GetDonateLevel()
	{
		if(m_nDonateMoney == 0)
		{
			return 0;
		}
		else if(m_nDonateMoney < 100)
		{
			return 1;
		}
		else if(m_nDonateMoney < 500)
		{
			return 2;
		}
		else if(m_nDonateMoney < 2000)
		{
			return 3;
		}
		else if(m_nDonateMoney < 5000)
		{
			return 4;
		}

		return 5;
	}
	void SyncDonateLeft();

	//	是否有该礼品
	bool IsGiftIDExist(int _nGiftID);
	GiftItemIDVector& GetGiftItemIDs()
	{
		return m_xGiftItemIDs;
	}
	bool ReceiveGift(int _nGiftID);

	//	同步玩家数据
	void SendPlayerDataTo(HeroObject* _pRecvHero);

	//	暴击相关
	bool CanCriticalAttack();
	float GetCriticalAttackLimit();

	//	read/write extend json bytes
	void WriteExtendJson(std::string& _refData);
	void ReadExtendJson(std::string& _refData);

	//	read/write extend bytes
	void GetHeroExtendAttrib(ExtendAttribVector& _refVec, bool _bIsSelf = true);
	void WriteSaveExtendField(ByteBuffer& _refBuffer);
	bool ReadSaveExtendField(ByteBuffer& _refBuffer);

	//	event
	void OnHeroKilledMonster(MonsterObject* _pMons);

	//	获得生活技能
	void UpdateLifeSkillLevel(LifeSkillType _eType, int _nLevel);

	//	PVP相关
	inline HeroPkType GetPkType()					{return m_ePkType;}
	inline void SetPkType(HeroPkType _eT)			{m_ePkType = _eT;}
	bool CanPk();
	bool CanPkPlayer(HeroObject* _pHero);

public:
	ItemAttrib* GetItemByIndex(DWORD _dwIndex);
	ItemAttrib* GetItemByTag(DWORD _dwTag);
	ItemAttrib* Lua_GetItemByTag(int _nTag)				{return GetItemByTag(_nTag);}
	ItemAttrib* Lua_GetItemByAttribID(int _nId);
	ItemAttrib* GetEmptyItem();
	bool AddBagItem(const ItemAttrib* _pItem);
	bool AddBagItem(DWORD _dwIndex, const ItemAttrib* _pItem);
	int GetBagEmptySum();
	int GetAssistEmptySum();
	int GetAllEmptySum();
	BYTE CheckItemCanDress(ItemAttrib* _pItem);
	inline ItemAttrib* GetEquip(PLAYER_ITEM_TYPE _type)	{return &m_stEquip[_type];}
	inline QuestContext* GetQuest()						{return &m_xQuest;}
	int CountItem(int _nAttribID);
	void ClearItem(int _nAttribID, int _number);
	bool AddItem(int _nAttribID);
	int AddItemReturnTag(int _nAttribID);
	bool AddItemNoBind(int _nAttribID);
	int AddItemNoBindReturnTag(int _nAttribID);
	bool AddBatchItem(int _nAttribID, int _nSum);
	bool AddItem_GM(int _nAttribID);
	bool RemoveItem(int _nTag);
	int ItemTagToAttribID(int _nTag);
	void SyncItemAttrib(int _nTag);
	bool AddCostItem(int _nAttribID);
	int GetBagStoreBodyItemCount(int _nAttribID);
	void SyncQuestData(int _nStage);
	HeroObject* GetTeamMate(int _idx);

	void ClearAllItem();
	void FlyToHome();
	void FlyToPrison();
	void SetValidState()								{m_pValid->UpdateAllAttrib();}

	inline int GetHeroJob()								{return m_stData.bJob;}
	inline int GetHeroSex()								{return /*m_stData.stAttrib.sex;*/GetObject_Sex();}
	inline GameScene* GetHomeScene();
	inline DWORD GetHomeMapID()							{return m_dwLastCityMap;}

	inline bool IsNewPlayer()							{return m_bIsNewHero;}
	inline void SetNewPlayer()							{m_bIsNewHero = true;}

	inline DWORD GetLoginMinutes()						{return m_dwLastLoginMinutes;}
	void SetLoginMinutes(DWORD _dwMin)					{m_dwLastLoginMinutes = _dwMin;}
	inline DWORD GetKilledMonsterCounter()				{return m_dwKilledMonsterCounter;}
	void SetKilledMonsterCounter(DWORD _dwCnt)			{m_dwKilledMonsterCounter = _dwCnt;}

	inline DWORD GetLSIndex()							{return m_dwLSIndex;}
	inline void SetLSIndex(DWORD _dwIdx)				{m_dwLSIndex = _dwIdx;}

	inline DWORD GetUID()								{return m_dwUID;}
	inline void SetUID(DWORD _dwUID)					{m_dwUID = _dwUID;}

	bool HaveSlave();

	inline bool IsLastAttackCritical()					{return m_bLastAttackCritical;}

	inline int GetHeroWanLi()
	{
		if(GetObject_Level() > MAX_LEVEL ||
			m_stData.bJob > 2)
		{
			return 0;
		}
		else
		{
			//return g_nWanLiTable[GetObject_Level() - 1][m_stData.bJob];
			return GetGlobalWanLi(GetObject_Level(), m_stData.bJob);
		}
	}

	static bool IsEquipItem(DWORD _dwType);
	static bool IsAttackItem(DWORD _dwType);
	static bool IsDefenceItem(DWORD _dwType);
	static bool IsJewelryItem(DWORD _dwType);
	static bool IsCostItem(DWORD _dwType);

	bool ShowQuestDlg(NPCObject* _pnpc, int _questid, int _step);
	void HideQuestDlg();
	bool ShowShopDlg(NPCObject* _pnpc, int _type);

	//	显示对话框的
	void ResetInteractiveDialog();
	bool AddInteractiveDialogItem(int _nX, int _nY, int _nType, int _nId, const char* _pszText);
	bool AddInteractiveDialogItem_CloseButton(int _nX, int _nY, const char* _pszText);
	bool AddInteractiveDialogItem_Button(int _nX, int _nY, int _nId, const char* _pszText);
	bool AddInteractiveDialogItem_String(int _nX, int _nY, const char* _pszText);
	void ShowInteractiveDialog(NPCObject* _pNpc);
	void HideInteractiveDialog();
	//	简化
	void ResetIDlg()
	{
		ResetInteractiveDialog();
	}
	bool AddIDlg_CloseButton(int _nX, int _nY, const char* _pszText)
	{
		return AddInteractiveDialogItem_CloseButton(_nX, _nY, _pszText);
	}
	bool AddIDlg_Button(int _nX, int _nY, int _nId, const char* _pszText)
	{
		return AddInteractiveDialogItem_Button(_nX, _nY, _nId, _pszText);
	}
	bool AddIDlg_String(int _nX, int _nY, const char* _pszText)
	{
		return AddInteractiveDialogItem_String(_nX, _nY, _pszText);
	}
	void ShowIDlg(NPCObject* _pNpc)
	{
		ShowInteractiveDialog(_pNpc);
	}
	void HideIDlg()
	{
		HideInteractiveDialog();
	}

	//	使用道具
	bool UseDrugItem(ItemAttrib* _pItem);
	bool UseScrollItem(ItemAttrib* _pItem);
	bool UseCostItem(ItemAttrib* _pItem);
	bool UseBaleItem(ItemAttrib* _pItem);
	bool UseTreasureMap(ItemAttrib* _pItem);
	bool UseSummonItem(ItemAttrib* _pItem);
	bool UseMoneyItem(ItemAttrib* _pItem);
	bool UseLotteryItem(ItemAttrib* _pItem);
	bool UseHairStyleCard(ItemAttrib* _pItem);
	bool UseWingStyleCard(ItemAttrib* _pItem);
	bool UsePackItem(ItemAttrib* _pItem);
	bool UseHairStyleResetCard(ItemAttrib* _pItem);
	bool UseWingStyleResetCard(ItemAttrib* _pItem);
	bool UseChatColorCard(ItemAttrib* _pItem);
	bool UseFireworkItem(ItemAttrib* _pItem);
	bool UseClothStyleCard(ItemAttrib* _pItem);
	bool UseClothStyleResetCard(ItemAttrib* _pItem);
	bool UseWeaponStyleCard(ItemAttrib* _pItem);
	bool UseWeaponStyleResetCard(ItemAttrib* _pItem);
	bool UseNameFrameCard(ItemAttrib* _pItem);
	bool UseNameFrameResetCard(ItemAttrib* _pItem);
	bool UseChatFrameCard(ItemAttrib* _pItem);
	bool UseChatFrameResetCard(ItemAttrib* _pItem);
	//bool UseLandScroll(ItemAttrib* _pItem);

	inline int GetMoney()
	{
		if(IsEncrypt())
		{
			return DecryptValue(ATB_MONEY_MASK, m_nMoney);
		}
		else
		{
			return m_nMoney;
		}
	}
	inline void SetMoney(int _nMoney)
	{
		if(IsEncrypt())
		{
			if(_nMoney < 0)
			{
				m_nMoney = EncryptValue(ATB_MONEY_MASK, 0);
			}
			else
			{
				m_nMoney = EncryptValue(ATB_MONEY_MASK, _nMoney);
			}
		}
		else
		{
			if(_nMoney < 0)
			{
				m_nMoney = 0;
			}
			else
			{
				m_nMoney = _nMoney;
			}
		}
	}
	void AddMoney(int _nMoney);
	void MinusMoney(int _nMoney);

	bool SwitchScene(DWORD _dwMapID, WORD _wPosX, WORD _wPosY);
	void GainExp(int _expr);

	void RefleshAttrib();
	void UpdateSuitAttrib();
	void UpdateStoveAttrib();
	void GetHeroAttrib(ItemAttrib* _pItem);

	inline void SetCityMap(WORD _wmap)							{m_dwLastCityMap = _wmap;}
	inline ItemAttrib* GetStoreBag()							{return m_stStore;}
	bool AddStoreItem(ItemAttrib* _pItem);
	bool IsStoreItemExist(DWORD _dwTag);
	ItemAttrib* GetStoreItem(DWORD _dwTag);
	ItemAttrib* GetStoreItemByIndex(DWORD _dwIndex)				{return &m_stStore[_dwIndex];}

	bool AddBigStoreItem(ItemAttrib* _pItem);
	ItemAttrib* GetBigStoreItem(DWORD _dwTag);

	USHORT GetHeroWalkInterval();
	USHORT GetHeroRunInterval();
	USHORT GetHeroSpellInterval();
	USHORT GetHeroAttackInterval();

	bool CanStruck(int _nDamage);

	void SendQuickMessage(int _nMsgCode, int _nParam = 0);

	int GetChallengeItemID();
	bool UseChallengeItem(int _nClgID);

	//	login server sync
	void LoginSvr_UpdatePlayerRank();

	void Lua_SetQuestStep(int _nQuestId, int _nStep);
	int Lua_GetQuestStep(int _nQuestId);
	void Lua_SetQuestCounter(int _nQuestId, int _nCounter);
	int Lua_GetQuestCounter(int _nQuestId);

	void SetItemBind(ItemAttrib* _pItem, bool _bBind);

	//	转移鉴定属性
	int TransferIdentifyAttrib(int _nItemTag0, int _nItemTag1, int _nCount);
	int TransferIdentifyAttribFailed(int _nItemTag0, int _nItemTag1);

public:
	//	Magic
	bool DoSpell(const PkgUserActionReq& req);
	bool HandleSpell(const PkgUserActionReq& req);
	bool DoSpeHit(MonsterObject* _pMonster, HeroObject* _pHero, DWORD* _pHitStyle);

	bool CanAllowLongHit();
	bool CanAllowWideHit();
	bool CanAllowPowerHit();
	bool CanAllowFireHit();
	bool CanAllowSuperFireHit();

	bool ReadBook(ItemAttrib* _pItem);

	const UserMagic* GetUserMagic(DWORD _dwMgcID);
	int GetMagicLevel(DWORD _dwMgcID)
	{
		const UserMagic* pMgc = GetUserMagic(_dwMgcID);
		if(NULL == pMgc)
		{
			return 0;
		}
		else
		{
			return pMgc->bLevel;
		}
	}
	const UserMagic* GetUserMagicByIndex(int _index)
	{
		return &m_xMagics[_index];
	}
	bool AddUserMagic(DWORD _dwMgcID);
	bool UpgradeUserMagic(DWORD _dwMgcID);
	int GetMagicDamage(const UserMagic* _pMgc, GameObject* _pAttacked);
	int GetMagicDamageNoDefence(const UserMagic* _pMgc);
	int GetMagicCost(const UserMagic* _pMgc);
	int GetMagicMaxDC(const UserMagic* _pMgc);
	int GetMagicMinDC(const UserMagic* _pMgc);

	int GetMagicACDefence(int _damage);
	int GetMagicMACDefence(int _damage);
	int GetSheildDefence(int _damage);
	void ProcessSheild(int _defence);

	bool IsEffectExist(DWORD _dwMgcID);
	//bool ReceiveDamage(int _nDmg);

private:
	bool LearnMagic(DWORD _dwMgcID, BYTE _bBookLevel);


public:
	void SendChatMessage(std::string& _xMsg, unsigned int _dwExtra);
	void SendChatMessage(const char* _pszMsg, unsigned int _dwExtra);
	void SendSystemMessage(const char* _pszMsg);

public:
	inline void SetAssistItemsSum(int _sum)				{m_nAssistItemSum = _sum;}
	inline GameObject* GetSlave(int _index)
	{
		if(_index < 0 ||
			_index >= MAX_SLAVE_SUM)
		{
			return NULL;
		}
		return m_pSlaves[_index];
	}
	bool MakeSlave(int _slaveid);
	void KillAllSlave();
	void SetSlaveTarget(GameObject* _pTarget);
	bool ClearSlave(GameObject* _pSlave);
	inline int GetSlaveCount()
	{
		int nCounter = 0;
		for(int i = 0; i < MAX_SLAVE_SUM; ++i)
		{
			if(m_pSlaves[i] != NULL)
			{
				++nCounter;
			}
		}
		return nCounter;
	}

	inline int GetTeamID()			{return m_nTeamID;}
	inline void SetTeamID(int _id)	{m_nTeamID = _id;}
	bool IsTeamLeader();
	bool IsTeamMateAround(int _nOffset);
	void TeamMateFlyToInstanceMap(int _x, int _y, GameInstanceScene* _pInsScene);
	void TeamMateFlyToMap(int _x, int _y, int _nMapID);
	void GainExpEx(int _nExp);
	void GainExpExDelay(int _nExp);

	inline bool CanActiveDoubleDrop()
	{
		if(m_dwDoubleDropExpireTime != 0)
		{
			if(m_dwDoubleDropExpireTime > GetTickCount())
			{
				return true;
			}
		}
		return false;
	}
	//	export to lua
	inline int Lua_GetActiveDropParam()
	{
		if(CanActiveDoubleDrop())
		{
			return (int)MAKELONG(2, 0);
		}
		return 0;
	}
	inline void ActiveDoubleDrop(DWORD _dwLastTime)
	{
		m_dwDoubleDropExpireTime = GetTickCount() + _dwLastTime;
	}
	inline DWORD GetDoubleLastTime()
	{
		if(m_dwDoubleDropExpireTime == 0)
		{
			return 0;
		}
		if(m_dwDoubleDropExpireTime > GetTickCount())
		{
			return m_dwDoubleDropExpireTime - GetTickCount();
		}
		return 0;
	}
	inline bool IsGmHide()				{return m_bGmHide;}

	void IncWeaponGrow();
	void IncWeaponGrowWithExp(DWORD _dwExp);
	//void GainExpExCheck(int _nExp);

	//	wrap for drug use time
	DWORD GetLastDrugUseTime(int _nItemID);
	void SetLastDrugUseTime(int _nItemID, DWORD _dwTime);
	bool CanUseCoolDownDrug(int _nItemID, DWORD _dwCoolDownTime);

	void CheckServerNetDelay();
	inline DWORD GetServerNetDelay()		{return m_dwServerNetDelaySec;}

	//	time limit map
	void SetEnterTimeLimitScene(GameScene* _pScene);

	//	last use magic id
	inline int GetLastUseMagicID()			{return m_nLastUseMagicID;}
	inline void SetLastUseMagicID(int _id)	{m_nLastUseMagicID = _id;}

	inline bool IsNeedTransAni()			{return m_bNeedTransAni;}
	inline void SetNeedTransAni(bool _bSet)	{m_bNeedTransAni = _bSet;}

	inline BYTE GetDifficultyLevel()		{return m_byteDifficultyLevel;}
	inline void SetDifficultyLevel(BYTE _bLevel)	{m_byteDifficultyLevel = _bLevel;}

public:
	//	sync functions
	void SyncRandSeedNormalAtk();

public:
	void DoPacket(const PkgUserActionReq& req);
	void DoPacket(const PkgPlayerChangeEquipReq& req);
	void DoPacket(const PkgPlayerSayReq& req);
	void DoPacket(const PkgPlayerDropItemReq& req);
	void DoPacket(const PkgPlayerPickUpItemReq& req);
	void DoPacket(const PkgPlayerUndressItemReq& req);
	void DoPacket(const PkgPlayerDressItemReq& req);
	void DoPacket(const PkgPlayerUseItemReq& req);
	void DoPacket(const PkgGameLoadedAck& ack);
	void DoPacket(const PkgPlayerAttackReq& req);
	void DoPacket(const PkgPlayerSyncAssistReq& req);
	void DoPacket(const PkgPlayerShopOpReq& req);
	void DoPacket(const PkgPlayerUserDataReq& req);
	void DoPacket(const PkgPlayerMonsInfoReq& req);
	void DoPacket(const PkgPlayerReviveReq& req);
	void DoPacket(const PkgPlayerSlaveStopReq& req);
	void DoPacket(const PkgPlayerCubeItemsReq& req);
	void DoPacket(const PkgPlayerCallSlaveReq& req);
	void DoPacket(const PkgPlayerKillSlaveReq& req);
	void DoPacket(const PkgPlayerSpeOperateReq& req);
	void DoPacket(const PkgPlayerMergyCostItemReq& req);
	void DoPacket(const PkgPlayerNetDelayReq& req);
	void DoPacket(const PkgGmNotificationReq& req);
	void DoPacket(const PkgSystemNotifyReq& req);
	void DoPacket(const PkgPlayerDecomposeReq& req);
	void DoPacket(const PkgPlayerForgeItemReq& req);
	void DoPacket(const PkgPlayerSplitItemReq& req);
	void DoPacket(const PkgPlayerOffSellItemReq& req);
	void DoPacket(const PkgPlayerOffBuyItemReq& req);
	void DoPacket(const PkgPlayerOffGetListReq& req);
	void DoPacket(const PkgPlayerOffCheckSoldReq& req);
	void DoPacket(const PkgPlayerPrivateChatReq& req);
	void DoPacket(const PkgPlayerOffTakeBackReq& req);
	void DoPacket(const PkgPlayerIdentifyItemReq& req);
	void DoPacket(const PkgPlayerUnbindItemReq& req);
	void DoPacket(const PkgPlayerServerDelayAck& ack);
	void DoPacket(const PkgPlayerRankListReq& req);
	void DoPacket(const PkgPlayerGetOlShopListReq& req);
	void DoPacket(const PkgPlayerBuyOlShopItemReq& req);
	void DoPacket(const PkgLoginCheckBuyShopItemAck& ack);
	void DoPacket(const PkgLoginConsumeDonateAck& ack);
	void DoPacket(const PkgPlayerSmeltMaterialsReq& req);
	void DoPacket(const PkgPlayerHandMakeItemReq& req);
	void DoPacket(const PkgPlayerOpenPotentialReq& req);
	void DoPacket(const PkgPlayerChargeReq& req);
	void DoPacket(const PkgPlayerWorldSayReq& req);
	void DoPacket(const PkgPlayerLoginExtDataReq& req);
	void DoPacket(const PkgPlayerDifficultyLevelReq& req);
	void DoPacket(const PkgPlayerQuitSelChrReq& req);

	//	Transfer to NPC or Monster object to handle it
	void DoPacket(const PkgPlayerClickNPCReq& req);

protected:
	//ByteBuffer m_xSendBuffer;
	//	player's bag
	std::vector<ItemAttrib> m_xBag;
	int m_nMoney;
	ItemAttrib m_stEquip[PLAYER_ITEM_TOTAL];
	ItemAttrib m_stStore[MAX_STORE_NUMBER];
	ItemAttrib m_stBigStore[MAX_BIGSTORE_NUMBER];
	//	Can process packet
	bool m_bClientLoaded;

	//	Quest info
	QuestContext m_xQuest;
	//	Assist item sum
	int m_nAssistItemSum;
	//	
	DWORD m_dwIndex;

	//	the sheild defence
	DWORD m_dwDefence;
	USERMAGICVECTOR m_xMagics;

	//	Last city map
	WORD m_dwLastCityMap;
	//	Slaves
	GameObject* m_pSlaves[MAX_SLAVE_SUM];

	DWORD m_dwLastSpellTime;
	DWORD m_dwTimeOut;

	bool m_bIsNewHero;
	DWORD m_dwLastReviveTime;
	//	Save file version
	USHORT m_uVersion;

	DWORD m_dwLastCheckTime;
	int m_nGainedExp;

	//	move ring use time
	DWORD m_dwLastUseMoveRingTime;

	//	Team id
	int m_nTeamID;

	//	Last recv packet time
	DWORD m_dwLastRecvDataTime;
	//	Login time(In Minute)
	DWORD m_dwLastLoginMinutes;
	//	Killed monsters
	DWORD m_dwKilledMonsterCounter;

	DWORD m_dwDoubleDropExpireTime;

	DWORD m_dwReviveRingUseTime;

	DWORD m_dwLSIndex;

	DWORD m_dwUID;

	bool m_bGmHide;

	DWORD m_dwLastPrivateChatTime;
	DWORD m_dwLastTeamChatTime;

	DWORD m_dwLastReqOffSellItems;
	DWORD m_dwLastReqOffTakeBack;

	//	jingang expire
	DWORD m_dwJinGangExpireTime;

	//	unuse
	bool m_bUnUse;

	//	abnormal speed times
	int m_nAbnormalSpeedTimes;

	//	login time
	DWORD m_dwLoginTimeTick;
	bool m_bCheckSoldItems;

	//	auto save counter
	unsigned char m_cAutoSaveCounter;

	//	donate money
	int m_nDonateMoney;
	//	donate left
	int m_nDonateLeft;

	//	gift id
	GiftItemIDVector m_xGiftItemIDs;

	//	drug use time record
	DrugUseTimeMap m_xDrugUseTime;

	//	server net delay time
	DWORD m_dwServerNetDelaySec;
	DWORD m_dwLastCheckServerNetDelay;
	int m_nServerNetDelaySeq;

	//	rand generator
	RandGenerator m_xNormalAttackRand;

	//	extend json attribute
	ExtendHeroAttrib m_stExtAttrib;

	//	last dc mc sc random critical?
	bool m_bLastAttackCritical;

	//	interactive dialog items
	static PkgPlayerInteractiveDialogContentNot s_xInteractiveDialogItemPkg;

	//	enter time limit map time
	int m_nEnterTimeLimitMapTime;
	int m_nTimeLimitID;

	//	last use magic id
	int m_nLastUseMagicID;

	//	need trans animation>?
	bool m_bNeedTransAni;

	//	difficulty level
	BYTE m_byteDifficultyLevel;

	//	Pk模式
	HeroPkType m_ePkType;

	//	小退了?
	bool m_bSmallQuit;

//public:
	//int m_nConnectionIndex;
};

typedef std::list<HeroObject*> HeroObjectList;
//////////////////////////////////////////////////////////////////////////
#endif