#ifndef _INC_OBJECTSTATUS_
#define _INC_OBJECTSTATUS_
//////////////////////////////////////////////////////////////////////////
#include "ObjectEngine.h"
#include <stack>
//////////////////////////////////////////////////////////////////////////
//	ÈËÎï×´Ì¬

struct ObjectStatusItem
{
	int nStatusID;
	int nExpireTime;
	int nExtraParam;
	int nLastUpdateTime;
};

typedef std::map<int, ObjectStatusItem*> ObjectStatusMap;

extern std::stack<ObjectStatusItem*> g_xObjectStatusFreeList;

class ObjectStatusControl
{
public:
	ObjectStatusControl(GameObject* _pAttach);
public:
	~ObjectStatusControl();

public:
	void Update();
	void CalcObjectStatus(ObjectStatusItem* _pStatusItem);
	void Clear();
	bool CheckStatusExist(int _nStatusID);
	void RemoveStatus(int _nStatusID);
	ObjectStatusItem* GetStatus(int _nStatusID);
	bool InsertStatus(int _nStatusID, int _nExpireTime, int _nExtraParam);

public:
	bool IsFreezon();
	bool IsStone();
	bool IsLvDu();
	bool IsHongDu();
	bool IsShield();
	bool IsEnergyShield();
	bool IsHide();
	bool IsDecDC();

protected:
	ObjectStatusMap m_xObjectStatus;
	GameObject* m_pAttach;
};

//////////////////////////////////////////////////////////////////////////
//	Ä§·¨ÑÓ³Ù
class MagicDelayManager
{
protected:
	MagicDelayManager();

public:
	~MagicDelayManager();

public:
	static MagicDelayManager* GetInstance()
	{
		static MagicDelayManager* s_pIns = NULL;
		if(NULL == s_pIns)
		{
			s_pIns = new MagicDelayManager;
		}
		return s_pIns;
	}

public:

};

//////////////////////////////////////////////////////////////////////////
//	Ò©Æ·ÑÓ³Ù
struct DrugDelayItem
{
	int nDrugAttribID;
	DWORD dwExpireTime;
};

typedef std::stack<DrugDelayItem*> DrugDelayFreeList;
typedef std::map<int, DrugDelayItem*> DrugUseHistoryList;


extern DrugDelayFreeList g_xDrugDelayFreeList;

class DrugDelayManager
{
protected:
	DrugDelayManager();

public:
	~DrugDelayManager();

public:
	static DrugDelayManager* GetInstance()
	{
		static DrugDelayManager* s_pIns = NULL;
		if(NULL == s_pIns)
		{
			s_pIns = new DrugDelayManager;
		}
		return s_pIns;
	}

public:
	bool CanUse(int _nDrugItemAttribID);
	bool PushDrugStatus(int _nAttribID, int _nDelayTime);
	void Clear();

private:
	DrugUseHistoryList m_xDrugUseHistoryList;
};

//////////////////////////////////////////////////////////////////////////
#endif