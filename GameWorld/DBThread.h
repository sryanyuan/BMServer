#ifndef _INC_DBTHREAD_
#define _INC_DBTHREAD_
//////////////////////////////////////////////////////////////////////////
#include <sqlite3.h>
#include <Windows.h>
#include "Struct.h"
//#include "../../CommonModule/ScriptEngine.h"
#include "LuaServerEngine.h"
#include <list>
#include "../../CommonModule/ObjectData.h"
//////////////////////////////////////////////////////////////////////////
struct SqlElement
{
	sqlite3* pSql;
	char szName[20];
};
//////////////////////////////////////////////////////////////////////////
struct ItemAttrib;
class HeroObject;
//////////////////////////////////////////////////////////////////////////
#define ITEM_ID_MAX	2000
#define DBCALLBACK	__cdecl
#define ITEMATTRIB_COL	35
#define MAGICATTRIB_COL 13

/*
#define ITEM_MAKE_MASK	0x7A
#define INVALID_ITEM_MAKE_MASK	0x18*/

#define DBTHREAD_PARAM_MAX	7
//////////////////////////////////////////////////////////////////////////
/*
int GetItemMakeMask(BYTE _bRandom);
int GetMakeMaskValue(BYTE _bMask);*/
//////////////////////////////////////////////////////////////////////////
struct DBOperationParam
{
	DWORD dwOperation;
	DWORD dwParam[DBTHREAD_PARAM_MAX];

	DBOperationParam()
	{
		dwOperation = 0;
		memset(dwParam, 0, sizeof(dwParam));
	}

	void SetOperation(int _nOp)
	{
		dwOperation = _nOp;
	}
	void SetParam(int _idx, int _value)
	{
		if(_idx < 0 ||
			_idx >= DBTHREAD_PARAM_MAX)
		{
			return;
		}

		dwParam[_idx] = _value;
	}
};

struct ItemVerify
{
	ItemAttrib* pUserItem;
	bool bValid;
};
//////////////////////////////////////////////////////////////////////////
#define PROCESS_POOL_SIZE	(5*1024)
#define MAX_DATABASE_NUM	5
#define MAX_TRANSACTION_NUM	50
//////////////////////////////////////////////////////////////////////////
typedef std::list<DBOperationParam*> DBOPERATIONS;
extern USHORT g_nItemPrice[ITEM_ID_MAX];

typedef int (*DBFUNC)(void*,int,char**,char**);
//////////////////////////////////////////////////////////////////////////

class ItemIDList : public std::list<int>
{
public:
	virtual ~ItemIDList(){}

public:
	void Push(int _value)
	{
		push_back(_value);
	}
};
//////////////////////////////////////////////////////////////////////////
class DBThread
{
public:
	~DBThread();
	static DBThread* GetInstance(bool _bDestroy = false)
	{
		static DBThread* pIns = NULL;
		if (_bDestroy) {
			delete pIns;
			pIns = NULL;
			return pIns;
		}
		if(NULL == pIns)
		{
			pIns = new DBThread;
		}
		return pIns;
	}

protected:
	DBThread();

public:
	unsigned int Run();
	void Stop();
	void Join();

	bool ConnectDB(const char* _pszDBName);
	void DisconnectDB();

	void AddProcess(const DelayedDBProcess* _pProc);
	void AsynExecute(DBOperationParam* _pParam);

	bool LoadItemsPrice();
	bool LoadMagicAttrib();
	bool ExtendOperation(const char* _pszExpr, DBFUNC _pFunc, void* _pParam);
	bool VerifyHeroItem(HeroObject* _pHero);
	bool VerifyHeroItemSimple(HeroObject* _pHero);
	void CheckUserItem(ItemAttrib* _pItem, ItemVerify* _pVerify);

	void LoadAndAddPlayerItemAttrib(ItemAttrib* _pItem, WORD _dwItemID, DWORD _dwUse, HeroObject* _pPlayer);

	void UpgradeItems(ItemAttrib* _pItem, int _nProb);
	bool UpgradeAttrib(ItemAttrib* _pItem, int _index, int _value);

	bool LoadScript();

	//	for lua
	DBOperationParam* GetNewParam()
	{
		DBOperationParam* pParam = new DBOperationParam;
		return pParam;
	}
	LuaServerEngine* GetLuaEngine()
	{
		return &m_xScript;
	}
	lua_State* GetLuaState()
	{
		//return m_xScript.GetScript();
		return m_xScript.GetVM();
	}
	GroundItem* NewGroundItem()
	{
		//GroundItem* pItem = new GroundItem;
		//ZeroMemory(pItem, sizeof(GroundItem));
		//return pItem;
		return NULL;
	}
	void DeleteGroundItem(GroundItem* _pItem)
	{
		//delete (_pItem);
	}
	int GroundItemToInt(GroundItem* _pItem)
	{
		//return (int)_pItem;
		return 0;
	}

public:
	static unsigned int __stdcall WorkThread(void* _pData);
	static int DBCALLBACK DBItemAttribCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName);
	static int DBCALLBACK DBMonsAttribCallback(void* _pParam,int _nCount, char** _pValue, char** _pName);
	static int DBCALLBACK DBDropItemCallback(void* _pParam,int _nCount, char** _pValue, char** _pName);
	static int DBCALLBACK DBDropItemExCallback(void* _pParam,int _nCount, char** _pValue, char** _pName);
	static int DBCALLBACK DBDropItemExCallback_Lua(void* _pParam,int _nCount, char** _pValue, char** _pName);
	static int DBCALLBACK DBDropItemExCallback_Lua2(void* _pParam,int _nCount, char** _pValue, char** _pName);
	static int DBCALLBACK DBItemPriceCallback(void* _pParam,int _nCount, char** _pValue, char** _pName);
	static int DBCALLBACK DBMagicAttribCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName);

	static int DBCALLBACK DBItemVerifyCallback(void* _pParam,int _nCount, char** _pValue, char** _pName);
	static int DBCALLBACK DBItemVerifySimpleCallback(void* _pParam,int _nCount, char** _pValue, char** _pName);

protected:
	void DoWork();
	void HandleProcess(const DelayedDBProcess* _pProc);

	void LockProcess()
	{
		EnterCriticalSection(&m_stCsProcess);
	}
	void UnLockProcess()
	{
		LeaveCriticalSection(&m_stCsProcess);
	}
	void ProcessGameDelay();
	inline void PushGameDelay(DBOperationParam* _pParam)		{m_xGameDelayProcess.push_back(_pParam);}

private:
	void DoQuery();

private:
	bool m_bTerminate;
	SqlElement m_stSql[MAX_DATABASE_NUM];

	//	For thread
	unsigned int m_uThreadID;
	HANDLE m_hThread;
	//	Working process buffer
	ByteBuffer m_xProcess;
	DWORD m_dwUpdateCounter;
	DWORD m_dwDeleteCounter;
	DWORD m_dwInsertCounter;
	ByteBuffer m_xTransactionDelete;
	ByteBuffer m_xTransactionUpdate;
	ByteBuffer m_xTransactionInsert;
	ByteBuffer m_xTransactionQuery;
	//	critical section
	CRITICAL_SECTION m_stCsProcess;
	//	operation list
	DBOPERATIONS m_xQueryOperations;
	//	into gamewolrd delay process
	DBOPERATIONS m_xGameDelayProcess;

	int m_nWorldDropMulti;

	//	db script
	LuaServerEngine m_xScript;
};
//////////////////////////////////////////////////////////////////////////
#endif