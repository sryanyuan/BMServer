#ifndef _INC_MONSTEROBJECT_
#define _INC_MONSTEROBJECT_
//////////////////////////////////////////////////////////////////////////
#include "ObjectEngine.h"
#include "FreeListManager.h"
//////////////////////////////////////////////////////////////////////////
class MonsterObject : public GameObject
{
public:
	MonsterObject(/*DWORD _dwID*/);
	virtual ~MonsterObject();

public:
	virtual void DoWork(unsigned int _dwTick);
	virtual void DoAction(unsigned int _dwTick);
	virtual void UpdateStatus(unsigned int _dwCurTick);

	virtual int ReceiveDamage(GameObject* _pAttacker, bool _bMgcAtk, int _oriDC = 0, int _nDelay = 350, const ReceiveDamageInfo* _pInfo = NULL);
	virtual void ReceiveHeroDamage(GameObject* _pAttacker, int _nDamage, int _delay);

	virtual void ParseAttackMsg(AttackMsg* _pMsg);

	virtual void ProcessDelayAction();

	virtual void OnDeadRevive(int _nLeftLife){}

public:
	void AddAttackProcess(ByteBuffer& _xBuf);
	void AddAttackProcess(AttackMsg* _pMsg);
	void ClearAttackProcess();
	inline GameObject* GetTarget()					{return m_pTarget;}
	inline void ResetTarget()						{m_pTarget = NULL;}
	/*inline void SetTarget(GameObject* _pObj)		//{if(m_pTarget->GetMapID() == GetMapID()){m_pTarget = _pObj;}}
	{
		if(_pObj == NULL)
		{
			m_pTarget = NULL;
		}
		else
		{
			if(_pObj->GetMapID() == GetMapID())
			{
				m_pTarget = _pObj;
			}
		}
	}*/
	inline void SetTarget(GameObject* _pObj)		{m_pTarget = _pObj;}

	inline USHORT GetAttackLong()					{return GetObject_MaxSC();}
	inline USHORT GetViewRange()					{return GetObject_SC();}

	inline USHORT GetAttackInterval()
	{
		unsigned int dwInterval = GetObject_MaxMC();
		if(IsFrozen())
		{
			dwInterval += 600;
		}
		return dwInterval;
	}
	inline USHORT GetWalkInterval()
	{
		unsigned int dwInterval = GetObject_MC();
		if(IsFrozen())
		{
			dwInterval += 600;
		}
		return dwInterval;
	}

	inline void UpdateCostTime()					{m_dwAttackCost = GetAttackCostTime();m_dwWalkCost = GetWalkCostTime();}

	inline GameObject* GetMaster()					{return m_pMaster;}
	void SetMaster(GameObject* _pMst);

	inline bool IsMagicAttackMode()					{return TEST_FLAG_BOOL(GetObject_Weight(), WEIGHT_MASK_MGCATK);}
	inline bool CanSeeHide()						{return TEST_FLAG_BOOL(GetObject_Weight(), WEIGHT_MASK_SEEHIDE);}
	inline bool IsUnmovableMonster()				{return TEST_FLAG_BOOL(GetObject_Weight(), WEIGHT_MASK_UNMOVE);}
	inline bool CanDefFire()						{return TEST_FLAG_BOOL(GetObject_Weight(), WEIGHT_MASK_DEFFIRE);}
	inline bool IsStoneAttack()						{return TEST_FLAG_BOOL(GetObject_Weight(), WEIGHT_MASK_STONE);}
	inline bool IsPoisonAttack()					{return TEST_FLAG_BOOL(GetObject_Weight(), WEIGHT_MASK_POISON);}
	inline bool CanDefIce()							{return TEST_FLAG_BOOL(GetObject_Weight(), WEIGHT_MASK_DEFICE);}

	virtual int GetRandomAbility(ABILITY_TYPE _type);

	//	If the monster has master, it can upgrade itself
	inline void SetInitLevel(int _nLv)				{unsigned short wLo = LOBYTE(GetObject_MaxAC());SetObject_MaxAC(MAKEWORD(wLo, _nLv));}
	inline int GetInitLevel()						{return HIBYTE(GetObject_MaxAC());}
	inline int GetUpgradeLevel()					{return LOBYTE(GetObject_MaxAC());}
	inline void SetUpgradeLevel(int _nLv)			{unsigned short wHg = HIBYTE(GetObject_MaxAC());SetObject_MaxAC(MAKEWORD(_nLv, wHg));}
	inline int GetSlaveExpr()						{return GetObject_MaxMAC();}
	inline void SetSlaveExpr(int _nExpr)			{SetObject_MaxMAC(_nExpr);}
	bool AddSlaveExpr(int _nExpr);
	void SlaveLevelUp();
	inline void SetCanDropItems(bool _b)			{m_bCanDropItems = _b;}
	inline bool GetCanDropItems()					{return m_bCanDropItems;}
	inline int GetLifeTimes()						{return m_nLifeTimes;}
	inline void SetLifeTimes(int _n)				{m_nLifeTimes = _n;}
	inline bool GetCanStruck()						{return m_bCanStruck;}
	inline void SetCanStruct(bool _b)				{m_bCanStruck = _b;}

	inline int GetLastRecvDamage()					{return m_nLastRecvDamage;}
	//inline int GetTotalRecvDamage()					{return m_nTotalRecvDamage;}

	inline int GetMonsterID()						{return GetObject_ID();}

	inline int GetStoneProb()						{return m_nStoneProb;}
	inline void SetStoneProb(int _prob)				{m_nStoneProb = _prob;}
	inline int GetStoneTime()						{return m_nStoneTime;}
	inline void SetStoneTime(int _time)				{m_nStoneTime = _time;}

	inline int GetMasterIdBeforeDie()				{return m_nMasterIdBeforeDie;}
	inline void SetMasterIdBeforeDie(int _nMasterId)	{m_nMasterIdBeforeDie = _nMasterId;}

public:
	void ProcessAttackProcess();
	void KilledByMaster();

	GameObject* SearchViewRange();

	void Wonder();
	void WalkToTarget();
	void WalkToMaster();
	void Say(const char* _pszSaying);
	virtual void AttackHero();

	int GetDefPalsyProb();

	//	Auto send packet
	//bool WalkOneStep(int _nDrt);

public:

	void DropMonsterItems();
	void DropMonsterItems(HeroObject* _pHero);

public:
	virtual unsigned int GetWalkCostTime();
	virtual unsigned int GetAttackCostTime();

public:
	//
	virtual bool CanAttack();
	virtual void OnFindTarget(){}
	virtual void OnLoseTarget(){}
	virtual bool IsOutOfView();

	virtual void SlaveLogic();
	virtual void MonsterLogic();
	virtual bool AttackTarget();

	virtual void OnMonsterDead(HeroObject* _pAttacher, bool _bKilledBySlave);

protected:
	ByteBuffer m_xAttackBuf;
	//DWORD m_dwCurrentTime;
	GameObject* m_pTarget;

	unsigned int m_dwAttackCost;
	unsigned int m_dwWalkCost;

	GameObject* m_pMaster;

	//	Attack list
	AttackMsgList m_xAttackMsgList;

	bool m_bCanDropItems;

	bool m_bSlaveKilledByMaster;

	int m_nLastRecvDamage;

	//	石化对方的几率 0-100
	int m_nStoneProb;
	//	石化时间
	int m_nStoneTime;

	//	live master id
	int m_nMasterIdBeforeDie;

	int m_nLifeTimes;

	bool m_bCanStruck;

	bool m_bDeadAsSlave;

	//int m_nTotalRecvDamage;
};

//	触龙神 赤月恶魔
class GroundMonster : public MonsterObject
{
public:
	GroundMonster();
	virtual ~GroundMonster();

public:
	virtual void DoAction(unsigned int _dwTick);
	virtual bool CanAttack();
	virtual void OnFindTarget();
	virtual void OnLoseTarget();
	virtual bool IsOutOfView();
	virtual bool AttackTarget();

protected:
	bool m_bInGround;
	unsigned int m_dwLastAppearTime;
	unsigned int m_dwLastGroundTime;
};

//	神兽
class ShenShouMonster : public MonsterObject
{
public:
	ShenShouMonster();
	virtual ~ShenShouMonster();

public:
	virtual bool AttackTarget();

	virtual int GetRandomAbility(ABILITY_TYPE _type);
};

//	白虎
class BaiHuMonster : public MonsterObject
{
public:
	BaiHuMonster();
	virtual ~BaiHuMonster();

public:
	virtual int GetRandomAbility(ABILITY_TYPE _type);
};

//	上古神兽
class SuperShenShouMonster : public ShenShouMonster
{
/*
public:
	SuperShenShouMonster();
	virtual ~SuperShenShouMonster();*/

public:
	virtual int GetRandomAbility(ABILITY_TYPE _type);
};

//	骷髅
class KulouMonster : public MonsterObject
{
public:
	KulouMonster();
	virtual ~KulouMonster();

public:
	virtual void DoAction(unsigned int _dwTick);
	virtual int GetRandomAbility(ABILITY_TYPE _type);

private:
	unsigned int m_dwLastShowTime;
};

//	沃玛教主
class WoMaJiaoZhuMonster : public MonsterObject
{
public:
	WoMaJiaoZhuMonster();
	virtual ~WoMaJiaoZhuMonster();

public:
	virtual void MonsterLogic();
	virtual bool AttackTarget();

protected:
	unsigned int m_dwLastSummonTime;
	unsigned int m_dwLastFlyTime;
	bool m_bUpdateAttrib;
};

//	祖玛教主
class ZuMaJiaoZhuMonster : public MonsterObject
{
public:
	ZuMaJiaoZhuMonster();
	virtual ~ZuMaJiaoZhuMonster();

public:
	virtual void MonsterLogic();

protected:
	unsigned int m_dwLastSummonTime;
	unsigned int m_dwLastFlyTime;
	bool m_bUpdateAttrib;
};

//	电僵尸类
class Self16DrtMonster : public MonsterObject
{
public:
	virtual bool CanAttack();
};

//	赤月恶魔
class ChiYueEMoMonster : public MonsterObject
{
public:
	virtual bool CanAttack();
	virtual bool AttackTarget();
};

//	魔龙之柱
class MoLongStoneMonster : public MonsterObject
{
public:
	MoLongStoneMonster();

public:
	virtual bool CanAttack();
	virtual bool AttackTarget();
	//	解决魔龙教主在视野外死亡 并且尸体消失后 m_pBoss没有设为NULL导致服务器崩溃的问题
	virtual void DoAction(unsigned int _dwTick);

public:
	inline void SetBoss(MonsterObject* _pMonster)
	{
		m_pBoss = _pMonster;
	}

private:
	MonsterObject* m_pBoss;
	unsigned int m_dwLastSearchBossTime;
};

//	MoLong king
class MoLongKingMonster : public MonsterObject
{
public:
	MoLongKingMonster();

public:
	virtual void MonsterLogic();
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();

private:
	unsigned int m_dwLastSummonTime;
	unsigned int m_dwFlyTime;
	bool m_bUpdateAttrib;
};

//	SuHuMonster
class SuHuMonster : public MonsterObject
{
public:
	SuHuMonster();

public:
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();
	virtual bool CanAttack();

private:
	unsigned int m_dwLastSpellTime;
};

//	FlyStatueMonster
class FlyStatueMonster : public MonsterObject
{
public:
	FlyStatueMonster();

public:
	virtual void DoAction(unsigned int _dwTick);
	virtual bool CanAttack();
	virtual bool AttackTarget();

private:
	unsigned int m_dwLastSpellTime;
	unsigned int m_dwLastActualAttackTime;
	int m_nAttackMode;
};

//	FlameSummonerMonster
class FlameSummonerMonster : public MonsterObject
{
public:
	FlameSummonerMonster();
	virtual ~FlameSummonerMonster(){}

public:
	virtual void DoAction(unsigned int _dwTick);
	virtual bool CanAttack();
	virtual bool AttackTarget();

private:
	unsigned int m_dwLastSpellTime1;
	unsigned int m_dwLastSpellTime2;
	int m_nAttackMode;
	unsigned int m_dwLastActualAttackTime;
};

//	FlameDCMonster
class FlameDCMonster : public MonsterObject
{
public:
	FlameDCMonster();
	virtual ~FlameDCMonster(){}

public:
	virtual void DoAction(unsigned int _dwTick);
	virtual bool CanAttack();
	virtual bool AttackTarget();

private:
	unsigned int m_dwLastSpellTime;
	int m_nAttackMode;
};

//	FlameMCMonster
class FlameMCMonster : public MonsterObject
{
public:
	FlameMCMonster();
	virtual ~FlameMCMonster(){}

public:
	virtual void DoAction(unsigned int _dwTick);
	virtual bool CanAttack();
	virtual bool AttackTarget();

private:
	unsigned int m_dwLastSpellTime;
	int m_nAttackMode;
};

//	Shadow spider
class ShadowSpiderMonster : public MonsterObject
{
public:
	ShadowSpiderMonster();

public:
	virtual bool CanAttack();
	virtual bool AttackTarget();

private:
	unsigned int m_dwLastSummonTime;
};

//	explode spider
class ExplodeSpiderMonster : public MonsterObject
{
public:
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();
};

//	Fire dragon king
class FireDragonKing : public MonsterObject
{
public:
	FireDragonKing();
	virtual ~FireDragonKing();

public:
	virtual bool AttackTarget();

};

//	Ice soldier
class IceSoldierMonster : public MonsterObject
{
public:
	IceSoldierMonster();
	virtual ~IceSoldierMonster();

public:
	virtual bool CanAttack();
	virtual bool AttackTarget();
	virtual void DoAction(unsigned int _dwTick);
	virtual void AttackHero();

private:
	bool ExtAttack();

protected:
	int m_nAttackMode;
	unsigned int m_dwLastSpellTime;
};

//	Ice guard
class IceGuardMonster : public MonsterObject
{
public:
	IceGuardMonster();
	virtual ~IceGuardMonster();

public:
	virtual unsigned int GetAttackCostTime();
	virtual bool CanAttack();
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();
	virtual void AttackHero();

private:
	bool ExtAttack();

protected:
	int m_nAttackMode;
	unsigned int m_dwLastSpellTime;
};

//	Ice King of war
class IceKingOfWarMonster : public MonsterObject
{
public:
	IceKingOfWarMonster();
	virtual ~IceKingOfWarMonster();

public:
	virtual bool CanAttack();
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();
	virtual void AttackHero();

private:
	bool ExtAttack();
	bool ExtAttack2();

protected:
	int m_nAttackMode;
	unsigned int m_dwLastSpellTime;
	unsigned int m_dwLastSpellTime2;
	unsigned int m_dwLastFlyTime;
};

//	Ice king
class IceKingMonster : public MonsterObject
{
public:
	IceKingMonster();
	virtual ~IceKingMonster();

public:
	virtual bool CanAttack();
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();
	virtual void AttackHero();
	virtual void OnFindTarget();

private:
	bool ExtAttack();
	bool ExtAttack2();

protected:
	int m_nAttackMode;
	unsigned int m_dwLastSpellTime;
	unsigned int m_dwLastSpellTime2;
	unsigned int m_dwLastFlyTime;
};

//	Ice Savage
class IceSavageMonster : public MonsterObject
{
public:
	IceSavageMonster();
	virtual ~IceSavageMonster();

public:
	virtual bool CanAttack();
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();
	virtual void AttackHero();

private:
	bool ExtAttack();
	bool ExtAttack2();

protected:
	int m_nAttackMode;
	unsigned int m_dwLastSpellTime;
	unsigned int m_dwLastSpellTime2;
};

//	Ice Defender
class IceDefenderMonster : public MonsterObject
{
public:
	IceDefenderMonster();
	virtual ~IceDefenderMonster();

public:
	virtual bool CanAttack();
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();
	virtual void AttackHero();

private:
	bool ExtAttack();

protected:
	int m_nAttackMode;
	unsigned int m_dwLastSpellTime;
};

//	Yama Watcher Monster
class YamaWatcherMonster : public MonsterObject
{
public:
	YamaWatcherMonster();
	virtual ~YamaWatcherMonster();

public:
	virtual void MonsterLogic();
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();
	virtual bool CanAttack();
	virtual int GetRandomAbility(ABILITY_TYPE _type);

private:
	bool ExtAttack();
	bool ExtAttack2();

private:
	int m_nAttackMode;
	unsigned int m_dwLastFlyTime;
};

//	Blue ghast monster
class BlueGhasterMonster : public MonsterObject
{
public:
	BlueGhasterMonster();
	virtual ~BlueGhasterMonster(){}

public:
	virtual void OnMonsterDead(HeroObject* _pAttacher, bool _bKilledBySlave);
};

//	manworm
class ManWormMonster : public MonsterObject
{
public:
	ManWormMonster();
	virtual ~ManWormMonster();

public:
	virtual bool CanAttack();
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();
	virtual void AttackHero();

private:
	bool ExtAttack();

protected:
	int m_nAttackMode;
	unsigned int m_dwLastSpellTime;
};

//	BehemothDevourerMonster
class BehemothDevourerMonster : public MonsterObject
{
public:
	BehemothDevourerMonster();
	virtual ~BehemothDevourerMonster(){}

public:
	virtual bool CanAttack();
	virtual void DoAction(unsigned int _dwTick);
	virtual bool AttackTarget();
	virtual void AttackHero();

private:
	bool ExtAttack();
	bool ExtAttack2();
	bool ExtAttack3();
	bool CanAttackExt();
	bool CanAttackExt3();

protected:
	int m_nAttackMode;
	unsigned int m_dwLastActualAttackTime;
	unsigned int m_dwLastSpellTime;
	unsigned int m_dwLastSpeAtk2;
	unsigned int m_dwLastSpeAtk3;
};

//	BowMan
class BowManMonster : public MonsterObject
{
public:
	virtual int GetRandomAbility(ABILITY_TYPE _type);
};
//////////////////////////////////////////////////////////////////////////
#endif