#ifndef _INC_FREELISTMANAGER_
#define _INC_FREELISTMANAGER_
//////////////////////////////////////////////////////////////////////////
#include <list>
#include <stack>
#include <Windows.h>
//////////////////////////////////////////////////////////////////////////
struct SceneDelayMsg
{
	USHORT uOp;
	DWORD dwDelayTime;
	UINT uParam[6];
};

typedef std::list<SceneDelayMsg*> SceneDelayMsgList;
typedef std::stack<SceneDelayMsg*> SceneDelayMsgStack;

#define ATTACKMSG_MASK_CRITICAL	0x00000001

struct AttackMsg
{
	WORD wDamage;
	DWORD dwInfo;
	DWORD dwAttacker;
	BYTE bType;
	DWORD dwMasks;
	DWORD dwActiveTime;
};

typedef std::list<AttackMsg*> AttackMsgList;
typedef std::stack<AttackMsg*> AttackMsgStack;

enum DelayActionType
{
	kDelayAction_None = 0,
	kDelayAction_Attacked,
	kDelayAction_Struck,
	kDelayAction_Heal,
	kDelayAction_MakeSlave,
	kDelayAction_StaticMagic,
};

class DelayActionBase
{
public:
	DelayActionBase()
	{
		uOp = kDelayAction_None;
		dwActiveTime = 0;
	}
	virtual void Reset()
	{
		dwActiveTime = 0;
	}

public:
	USHORT uOp;
	DWORD dwActiveTime;
};

class DelayActionAttacked : public DelayActionBase
{
public:
	DelayActionAttacked()
	{
		uOp = kDelayAction_Attacked;
		nDamage = 0;
		nMgcId = 0;
		dwAttackerId = 0;
		dwAttackMask = 0;
	}
	virtual void Reset()
	{
		nDamage = 0;
		nMgcId = 0;
		dwAttackerId = 0;
		dwAttackMask = 0;

		__super::Reset();
	}

public:
	int nDamage;
	short nMgcId;
	DWORD dwAttackerId;
	DWORD dwAttackMask;
};

class DelayActionStruck : public DelayActionBase
{
public:
	DelayActionStruck()
	{
		uOp = kDelayAction_Struck;
	}
};

class DelayActionHeal : public DelayActionBase
{
public:
	DelayActionHeal()
	{
		uOp = kDelayAction_Heal;
		nHealCnt = 0;
	}
	virtual void Reset()
	{
		nHealCnt = 0;
		__super::Reset();
	}

public:
	int nHealCnt;
};

class DelayActionMakeSlave : public DelayActionBase
{
public:
	DelayActionMakeSlave()
	{
		uOp = kDelayAction_MakeSlave;
		nSlaveId = 0;
	}
	virtual void Reset()
	{
		nSlaveId = 0;
		__super::Reset();
	}

public:
	int nSlaveId;
};

class DelayActionStaticMagic : public DelayActionBase
{
public:
	DelayActionStaticMagic()
	{
		uOp = kDelayAction_StaticMagic;

		nMagicId = nMagicLevel = 0;
		wMaxDC = wMinDC = 0;
		nPosX = nPosY = 0;
		nContinueTime = 0;
		nMapId = 0;
	}
	virtual void Reset()
	{
		nMagicId = nMagicLevel = 0;
		wMaxDC = wMinDC = 0;
		nPosX = nPosY = 0;
		nContinueTime = 0;
		nMapId = 0;

		__super::Reset();
	}

public:
	int nMapId;
	int nMagicId;
	int nMagicLevel;
	WORD wMaxDC;
	WORD wMinDC;
	short nPosX;
	short nPosY;
	int nContinueTime;
};

typedef std::list<DelayActionBase*> DelayActionList;
typedef std::stack<DelayActionBase*> DelayActionStack;

//////////////////////////////////////////////////////////////////////////
class FreeListManager
{
public:
	~FreeListManager();
protected:
	FreeListManager();

public:
	static FreeListManager* GetInstance(bool _bDestroy = false)
	{
		static FreeListManager* s_pIns = NULL;
		if (_bDestroy) {
			delete s_pIns;
			s_pIns = NULL;
			return s_pIns;
		}
		if(NULL == s_pIns)
		{
			s_pIns = new FreeListManager;
		}
		return s_pIns;
	}

public:
	SceneDelayMsg* GetFreeSceneDelayMsg();
	void PushFreeSceneDelayMsg(SceneDelayMsg* _pMsg);

	AttackMsg* GetFreeAttackMsg();
	void PushFreeAttackMsg(AttackMsg* _pMsg);

	DelayActionBase* GetFreeDelayAction(DelayActionType _eType);
	void PushFreeDelayAction(DelayActionBase* _pAction);

	void Clear();

protected:
	SceneDelayMsgStack m_xSceneDelayMsgStack;
	AttackMsgStack m_xAttackMsgStack;
};
//////////////////////////////////////////////////////////////////////////
#endif