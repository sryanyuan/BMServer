#include "DBThread.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <process.h>
#include <Shlwapi.h>
#include "../../CommonModule/ObjectData.h"
#include "../GameWorld/GameWorld.h"
#include "../GameWorld/GameSceneManager.h"
#include "../GameWorld/ExceptionHandler.h"
#include "DBDropDownContext.h"
#include "../Helper.h"
//////////////////////////////////////////////////////////////////////////
const int g_nOpQueryItemAttrib = DO_QUERY_ITEMATTRIB;
USHORT g_nItemPrice[ITEM_ID_MAX];

static bool s_bHeroValid = true;
static bool s_bHeroValidThread2 = true;
static USHORT s_uHeroVersion = 0;
//////////////////////////////////////////////////////////////////////////
DBThread::DBThread() : m_xProcess(PROCESS_POOL_SIZE)
{
	m_bTerminate = false;
	m_uThreadID = 0;
	m_hThread = NULL;
	m_dwInsertCounter =
	m_dwInsertCounter =
	m_dwUpdateCounter = 0;
	InitializeCriticalSection(&m_stCsProcess);

	memset(g_nItemPrice, 0, sizeof(g_nItemPrice));

	for(int i = 0; i < MAX_DATABASE_NUM; ++i)
	{
		m_stSql[i].pSql = NULL;
	}

	m_nWorldDropMulti = 1;
}

DBThread::~DBThread()
{
	Stop();
	DeleteCriticalSection(&m_stCsProcess);
}
//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* unsigned int Run()
/************************************************************************/
unsigned int DBThread::Run()
{
	if(m_hThread != NULL)
	{
		//	the thread has run
		return 0;
	}

	//	set script load path
	char szBuf[MAX_PATH];
#ifdef _DEBUG
	sprintf(szBuf, "%s\\Help\\",
		GetRootPath());
	m_xScript.SetModulePath(szBuf, LOADMODE_PATH);
	m_xScript.SetLuaLoadPath(szBuf);
#else
	sprintf(szBuf, "%s\\Help\\dog.idx",
		GetRootPath());
	m_xScript.SetModulePath(szBuf, LOADMODE_ZIP);
	m_xScript.SetLuaLoadPath(szBuf);
#endif

	if(!LoadScript())
	{
		return 0;
	}

	m_hThread = (HANDLE)_beginthreadex(NULL,
		0,
		&DBThread::WorkThread,
		this,
		0,
		&m_uThreadID);

	if(m_uThreadID == 0)
	{
		LOG(ERROR) << "Can not start dbthread!";
		return 0;
	}
	else
	{
		LOG(INFO) << "DBThread start!";
		return 1;
	}

	return 1;
}
//////////////////////////////////////////////////////////////////////////
bool DBThread::LoadScript()
{
	m_xScript.Destroy();
	m_xScript.Create();
	tolua_BackMirServer_open(m_xScript.GetVM());
	m_xScript.PushEngine();

	if(m_xScript.DoModule("server/db/db"))
	{
#ifdef _DEBUG
		LOG(INFO) << "载入db.lua成功";
#endif
		return true;
	}
	return false;
}
/************************************************************************/
/* void Stop()
/************************************************************************/
void DBThread::Stop()
{
	m_bTerminate = true;
}

/************************************************************************/
/* void DoQuery
/************************************************************************/
void DBThread::DoQuery()
{
	RECORD_FUNCNAME_DB;
	//DelayedDBProcess stDp;
	DBOperationParam* pParam = NULL;
	char* pszErr = 0;
	int nRet = 0;
	char szExpr[MAX_PATH];

	//	query
	//if(m_xTransactionQuery.GetLength())
	if(!m_xQueryOperations.empty())
	{
		sqlite3* sql = m_stSql[SQL_ITEMATTRIB].pSql;
		if(sql)
		{
			nRet = sqlite3_exec(sql, "Begin transaction", 0, 0, &pszErr);
			if(nRet != SQLITE_OK)
			{
				LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
				sqlite3_free(pszErr);
			}
			else
			{
				//while(m_xTransactionQuery.GetLength())
				while(!m_xQueryOperations.empty())
				{
// 					try
// 					{
// 						m_xTransactionQuery >> stDp;
// 					}
// 					BUFFER_EXCEPTION_CATCH_RETURN_VOID;
					pParam = m_xQueryOperations.front();
					m_xQueryOperations.pop_front();

					//if(LOWORD(stDp.uParam0) == DO_QUERY_ITEMATTRIB)
					if(pParam->dwOperation == DO_QUERY_ITEMATTRIB)
					{
						sprintf(szExpr, "Select * from Items where id = %d",
							HIWORD(pParam->dwParam[1]));
						nRet = sqlite3_exec(sql, szExpr, &DBThread::DBItemAttribCallBack, (void*)pParam, &pszErr);
					}
					else if(pParam->dwOperation == DO_QUERY_MONSATTRIB)
					{
						sprintf(szExpr, "Select * from Monsters where id = %d",
							pParam->dwParam[1]);
						nRet = sqlite3_exec(sql, szExpr, &DBThread::DBMonsAttribCallback, (void*)pParam, &pszErr);
					}
					else if(pParam->dwOperation == DO_QUERY_DROPITEM)
					{
						sprintf(szExpr, "Select * from DropItem where id = %d",
							pParam->dwParam[0]);
						nRet = sqlite3_exec(sql, szExpr, &DBThread::DBDropItemCallback, (void*)pParam, &pszErr);
					}
					else if(pParam->dwOperation == DO_QUERY_DROPITEMEX)
					{
						sprintf(szExpr, "Select * from DropItem where id = %d",
							pParam->dwParam[0]);
						nRet = sqlite3_exec(sql, szExpr, &DBThread::DBDropItemExCallback_Lua2, (void*)pParam, &pszErr);
					}
					else if(pParam->dwOperation == DO_QUERY_RELOADSCRIPT)
					{
						if(LoadScript())
						{
							LOG(INFO) << "reload script db success!" ;
						}
						//	direct remove
						SAFE_DELETE(pParam);
					}
					if(nRet != SQLITE_OK)
					{
						LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
						sqlite3_free(pszErr);
						break;
					}
				}

				if(nRet == SQLITE_OK)
				{
					nRet = sqlite3_exec(sql, "Commit transaction", NULL, 0, &pszErr);
					if(nRet != SQLITE_OK)
					{
						LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
						sqlite3_free(pszErr);
					}
				}
			}
		}
		//m_xTransactionQuery.Reset();
	}
}

/************************************************************************/
/* void DoWork()
/************************************************************************/
void DBThread::DoWork()
{
	RECORD_FUNCNAME_DB;

	m_nWorldDropMulti = GameWorld::GetInstance().GetDropMultiple();
	if(m_nWorldDropMulti < 1 ||
		m_nWorldDropMulti >= 5)
	{
		m_nWorldDropMulti = 1;
	}

	ProcessGameDelay();

	LockProcess();

	//	query
	DoQuery();

	UnLockProcess();
}
//////////////////////////////////////////////////////////////////////////
bool DBThread::LoadItemsPrice()
{	
	DBOperationParam* pParam = NULL;
	char* pszErr = 0;
	int nRet = 0;
	char szExpr[MAX_PATH];
	bool bRet = false;

	sqlite3* sql = m_stSql[SQL_ITEMATTRIB].pSql;
	if(sql)
	{
		nRet = sqlite3_exec(sql, "Begin transaction", 0, 0, &pszErr);
		if(nRet != SQLITE_OK)
		{
			LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
			sqlite3_free(pszErr);
		}
		else
		{
			sprintf(szExpr, "Select * from Items");
			nRet = sqlite3_exec(sql, szExpr, &DBThread::DBItemPriceCallback, (void*)pParam, &pszErr);

			if(nRet == SQLITE_OK)
			{
				nRet = sqlite3_exec(sql, "Commit transaction", NULL, 0, &pszErr);
				if(nRet != SQLITE_OK)
				{
					LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
					sqlite3_free(pszErr);
				}
				else
				{
					bRet = true;
				}
			}
		}
	}
	return bRet;
}
//////////////////////////////////////////////////////////////////////////
bool DBThread::ExtendOperation(const char* _pszExpr, DBFUNC _pFunc, void* _pParam)
{
	char* pszErr = NULL;
	bool bRet = false;
	int nRet = 0;

	if(NULL == _pszExpr ||
		NULL == _pFunc)
	{
		return false;
	}

	sqlite3* sql = m_stSql[SQL_ITEMATTRIB].pSql;
	if(sql)
	{
		nRet = sqlite3_exec(sql, _pszExpr, _pFunc, _pParam, &pszErr);
		if(SQLITE_OK == nRet)
		{
			//	Nothing
			bRet = true;
		}
		else
		{
			LOG(ERROR) << "sqlite3 can execute this expression";
			sqlite3_free(pszErr);
			pszErr = NULL;
		}
	}
	return bRet;
}
//////////////////////////////////////////////////////////////////////////
bool DBThread::LoadMagicAttrib()
{
	DBOperationParam* pParam = NULL;
	char* pszErr = 0;
	int nRet = 0;
	char szExpr[MAX_PATH];
	bool bRet = false;

	sqlite3* sql = m_stSql[SQL_ITEMATTRIB].pSql;
	if(sql)
	{
		nRet = sqlite3_exec(sql, "Begin transaction", 0, 0, &pszErr);
		if(nRet != SQLITE_OK)
		{
			LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
			sqlite3_free(pszErr);
		}
		else
		{
			sprintf(szExpr, "Select * from Magics");
			nRet = sqlite3_exec(sql, szExpr, &DBThread::DBMagicAttribCallBack, (void*)pParam, &pszErr);

			if(nRet == SQLITE_OK)
			{
				nRet = sqlite3_exec(sql, "Commit transaction", NULL, 0, &pszErr);
				if(nRet != SQLITE_OK)
				{
					LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
					sqlite3_free(pszErr);
				}
				else
				{
					bRet = true;
				}
			}
		}
	}
	return bRet;
}
//////////////////////////////////////////////////////////////////////////
bool DBThread::VerifyHeroItem(HeroObject* _pHero)
{
	//	Body item
	ItemAttrib* pItem = NULL;
	bool bValid = true;
	int nRet = SQLITE_OK;
	sqlite3* sql = m_stSql[SQL_ITEMATTRIB].pSql;
	char szExpr[MAX_PATH];
	char* pszErr = 0;

	if(NULL == sql)
	{
		return false;
	}

	s_bHeroValid = true;
	s_uHeroVersion = _pHero->GetVersion();
	bool bBeginTransaction = false;

	/*USHORT uVersion = _pHero->GetVersion();
	int nItemTotal = 0;
	if(uVersion == BACKMIR_VERSION116)
	{
		nItemTotal = PLAYER_ITEM_TOTAL116;
	}
	else if(uVersion == BACKMIR_VERSION117)
	{
		//nItemTotal = ;
	}
	else
	{
		return false;
	}*/

	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		pItem = _pHero->GetEquip((PLAYER_ITEM_TYPE)i);
		if(NULL == pItem)
		{
			continue;
		}

		if(HeroObject::IsEquipItem(pItem->type))
		{
			//	Check it
			if(!bBeginTransaction)
			{
				bBeginTransaction = true;

				nRet = sqlite3_exec(sql, "Begin transaction", 0, 0, &pszErr);
				if(nRet != SQLITE_OK)
				{
					LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
					sqlite3_free(pszErr);
					return false;
				}
			}

			sprintf(szExpr, "Select * from Items where id = %d",
				pItem->id);
			nRet = sqlite3_exec(sql, szExpr, &DBThread::DBItemVerifyCallback, (void*)pItem, &pszErr);
			if(nRet != SQLITE_OK)
			{
				LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
				sqlite3_free(pszErr);
				return false;
			}
		}
	}

	//	Bag Item
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		pItem = _pHero->GetItemByIndex(i);
		if(NULL == pItem)
		{
			continue;
		}

		if(HeroObject::IsEquipItem(pItem->type))
		{
			sprintf(szExpr, "Select * from Items where id = %d",
				pItem->id);
			nRet = sqlite3_exec(sql, szExpr, &DBThread::DBItemVerifyCallback, (void*)pItem, &pszErr);
			if(nRet != SQLITE_OK)
			{
				LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
				sqlite3_free(pszErr);
				return false;
			}
		}
	}

	//	Storage item
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		pItem = _pHero->GetStoreItemByIndex(i);
		if(NULL == pItem)
		{
			continue;
		}
		if(HeroObject::IsEquipItem(pItem->type))
		{
			sprintf(szExpr, "Select * from Items where id = %d",
				pItem->id);
			nRet = sqlite3_exec(sql, szExpr, &DBThread::DBItemVerifyCallback, (void*)pItem, &pszErr);
			if(nRet != SQLITE_OK)
			{
				LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
				sqlite3_free(pszErr);
				return false;
			}
		}
	}

	if(bBeginTransaction)
	{
		nRet = sqlite3_exec(sql, "Commit transaction", NULL, 0, &pszErr);
		if(nRet != SQLITE_OK)
		{
			LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
			sqlite3_free(pszErr);
			return false;
		}
	}

	return s_bHeroValid;
}

bool DBThread::VerifyHeroItemSimple(HeroObject* _pHero)
{
	//	Body item
	ItemAttrib* pItem = NULL;
	bool bValid = true;
	int nRet = SQLITE_OK;
	sqlite3* sql = m_stSql[SQL_ITEMATTRIB].pSql;
	char szExpr[MAX_PATH];
	char* pszErr = 0;

	if(NULL == sql)
	{
		return false;
	}

	s_bHeroValidThread2 = true;
	//s_uHeroVersion = _pHero->GetVersion();
	bool bBeginTransaction = false;

	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		pItem = _pHero->GetEquip((PLAYER_ITEM_TYPE)i);
		if(NULL == pItem)
		{
			continue;
		}

		if(HeroObject::IsEquipItem(GETITEMATB(pItem, Type)))
		{
			//	Check it
			if(!bBeginTransaction)
			{
				bBeginTransaction = true;

				nRet = sqlite3_exec(sql, "Begin transaction", 0, 0, &pszErr);
				if(nRet != SQLITE_OK)
				{
					LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
					sqlite3_free(pszErr);
					return false;
				}
			}

			sprintf(szExpr, "Select * from Items where id = %d",
				GETITEMATB(pItem, ID));
			nRet = sqlite3_exec(sql, szExpr, &DBThread::DBItemVerifySimpleCallback, (void*)pItem, &pszErr);
			if(nRet != SQLITE_OK)
			{
				LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
				sqlite3_free(pszErr);
				return false;
			}
		}
	}

	if(bBeginTransaction)
	{
		nRet = sqlite3_exec(sql, "Commit transaction", NULL, 0, &pszErr);
		if(nRet != SQLITE_OK)
		{
			LOG(ERROR) << "sqlite3 transaction executing error!" << pszErr;
			sqlite3_free(pszErr);
			return false;
		}
	}

	return s_bHeroValidThread2;
}

void DBThread::CheckUserItem(ItemAttrib* _pItem, ItemVerify* _pVerify)
{

}
//////////////////////////////////////////////////////////////////////////
void DBThread::ProcessGameDelay()
{
	RECORD_FUNCNAME_DB;

	if(m_xGameDelayProcess.empty())
	{
		return;
	}

	DBOPERATIONS::const_iterator begiter = m_xGameDelayProcess.begin();
	DBOPERATIONS::const_iterator enditer = m_xGameDelayProcess.end();
	DBOperationParam* pParam = NULL;
	DelayedProcess dp;

	for(begiter; begiter != enditer; ++begiter)
	{
		pParam = *begiter;

		if(pParam->dwOperation == DO_QUERY_ITEMATTRIB)
		{
			dp.uOp = DP_ITEMLOADED;
			dp.uParam0 = (unsigned int)pParam;
			GameWorld::GetInstance().AddDelayedProcess(&dp);
		}
		else if(pParam->dwOperation == DO_QUERY_MONSATTRIB)
		{
			dp.uOp = DP_MONSLOADED;
			dp.uParam0 = (unsigned int)pParam;
			GameWorld::GetInstance().AddDelayedProcess(&dp);
		}
		else if(pParam->dwOperation == DO_QUERY_DROPITEM)
		{
			dp.uOp = DP_DROPITEMLOADED;
			dp.uParam0 = (unsigned int)pParam;
			GameWorld::GetInstance().AddDelayedProcess(&dp);
		}
		else if(pParam->dwOperation == DO_QUERY_DROPITEMEX)
		{
			dp.uOp = DP_DROPITEMEXLOADED;
			dp.uParam0 = (unsigned int)pParam;
			GameWorld::GetInstance().AddDelayedProcess(&dp);
		}
	}
	m_xGameDelayProcess.clear();
}

/************************************************************************/
/* bool ConnectDB(const char* _pszDBName)
/************************************************************************/
bool DBThread::ConnectDB(const char* _pszDBName)
{
	int nAvailableIndex = -1;

	for(int i = 0; i < MAX_DATABASE_NUM; ++i)
	{
		if(m_stSql[i].pSql == NULL)
		{
			nAvailableIndex = i;
			break;
		}
	}

	if(nAvailableIndex >= 0)
	{
		if(SQLITE_OK == sqlite3_open(_pszDBName, &m_stSql[nAvailableIndex].pSql))
		{
#ifdef _DEBUG
			LOG(INFO) << "The database [" << _pszDBName << "] was opened";
#endif
			strcpy(m_stSql[nAvailableIndex].szName, ::PathFindFileName(_pszDBName));
//#ifdef NDEBUG
			if(nAvailableIndex == SQL_ITEMATTRIB)
			{
				DWORD dwPsw[2];
				char* pWrt = (char*)dwPsw;
				pWrt[5] = 0;
				pWrt[4] = 'G';
				pWrt[3] = 'e';
				pWrt[2] = '3';
				pWrt[1] = 'S';
				pWrt[0] = 'x';
				//LOG(INFO) << "password is " << pWrt;


				PROTECT_START_VM

				if(SQLITE_OK == sqlite3_key(m_stSql[nAvailableIndex].pSql, pWrt, 5))
				{

				}
				else
				{
					LOG(FATAL) << "Can't open database!";
					sqlite3_close(m_stSql[nAvailableIndex].pSql);
					m_stSql[nAvailableIndex].pSql = NULL;
				}

				PROTECT_END_VM
			}
//#endif
			return true;
		}
		else
		{
			LOG(ERROR) << "Error occur when opening database]";
			m_stSql[nAvailableIndex].pSql = NULL;
		}
	}

	return false;
}
/************************************************************************/
/* void DisconnectDB(const char* _pszDBName = NULL)
/************************************************************************/
void DBThread::DisconnectDB()
{
	for(int i = 0; i < MAX_DATABASE_NUM; ++i)
	{
		if(m_stSql[i].pSql)
		{
			sqlite3_close(m_stSql[i].pSql);
			m_stSql[i].pSql = NULL;
		}
	}
}
/************************************************************************/
/* void AddProcess(const DelayedDBProcess* _pProc)
/************************************************************************/
void DBThread::AddProcess(const DelayedDBProcess* _pProc)
{
	LockProcess();

	try
	{
		if(_pProc->uOp > DO_QUERY_BEGIN &&
			_pProc->uOp < DO_QUERY_END)
		{
			m_xTransactionQuery << *_pProc;
		}
		else if(_pProc->uOp > DO_INSERT_BEGIN &&
			_pProc->uOp < DO_INSERT_END)
		{
			m_xTransactionInsert << *_pProc;
		}
		else if(_pProc->uOp > DO_UPATE_BEGIN &&
			_pProc->uOp < DO_UPDATE_END)
		{
			m_xTransactionUpdate << *_pProc;
		}
	}
	BUFFER_EXCEPTION_CATCH_RETURN_VOID;

	UnLockProcess();
}

void DBThread::AsynExecute(DBOperationParam* _pParam)
{
	LockProcess();

	if(_pParam->dwOperation > DO_QUERY_BEGIN &&
		_pParam->dwOperation < DO_QUERY_END)
	{
		m_xQueryOperations.push_back(_pParam);
	}

	UnLockProcess();
}
/************************************************************************/
/* void HandleProcess(const DelayedDBProcess* _pProc)
/************************************************************************/
void DBThread::HandleProcess(const DelayedDBProcess* _pProc)
{
	RECORD_FUNCNAME_DB;

	int nRet = 0;
	char* pszErr = NULL;

	switch(LOWORD(_pProc->uOp))
	{
	case DO_QUERY_ITEMATTRIB:
		{
			//	query item attrib
			sqlite3* sql = m_stSql[SQL_ITEMATTRIB].pSql;
			if(sql)
			{
				nRet = sqlite3_exec(sql, "Begin transaction", 0, 0, &pszErr);
				if(nRet == SQLITE_OK)
				{

				}
				else
				{
					LOG(ERROR) << "Sqlite3 transaction executing error!";
				}
			}
		}break;
	case DO_QUERY:
		{
			//	query
		}break;
	case DO_INSERT:
		{

		}break;
	case DO_UPDATE:
		{
			//	update
		}break;
	}
}

//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* unsigned int __stdcall DBThread::WorkThread(void* _pData)
/************************************************************************/
unsigned int __stdcall DBThread::WorkThread(void* _pData)
{
	RECORD_FUNCNAME_DB;

	DBThread* pIns = (DBThread*)_pData;
	unsigned int uRet = 1;

	//	Open database
//	char szWorkingDrt[MAX_PATH];
//	GetModuleFileName(NULL, szWorkingDrt, MAX_PATH);
//	::PathRemoveFileSpec(szWorkingDrt);
//	strcat(szWorkingDrt, "\\User.db");
//	bool bOk = pIns->ConnectDB(szWorkingDrt);
// 	char* pszErr = NULL;
// 	if(SQLITE_OK != sqlite3_exec(pIns->m_stSql[0].pSql, "create table ff(id int)", NULL, NULL, &pszErr))
// 	{
// 		LOG(WARNING) << pszErr;
// 		sqlite3_free(pszErr);
// 	}
	bool bOk = true;
	srand((unsigned int)time(NULL));

	while(TRUE)
	{
		if(!bOk)
		{
			//	Error occurred before starting this thread
			uRet = 0;
			break;
		}
		if(pIns->m_bTerminate)
		{
			break;
		}

		//	do work
		//try
		{
			pIns->DoWork();
		}
		//catch(...)
		{
			//LOG(ERROR) << "FATAL!!!";
		}
		
		SleepEx(1, TRUE);
	}

	_endthreadex(uRet);
	return uRet;
}

void DBThread::LoadAndAddPlayerItemAttrib(ItemAttrib *_pItem, WORD _dwItemID, DWORD _dwUse, HeroObject *_pPlayer)
{
	DBOperationParam* pParam = new DBOperationParam;
	pParam->dwOperation = DO_QUERY_ITEMATTRIB;
	pParam->dwParam[0] = (DWORD)_pItem;
	pParam->dwParam[1] = MAKELONG(_dwUse, _dwItemID);
	pParam->dwParam[2] = (DWORD)_pPlayer;
	AsynExecute(pParam);
}

void DBThread::UpgradeItems(ItemAttrib* _pItem, int _nProb)
{
	RECORD_FUNCNAME_DB;

	static const int s_nWeaponUpAttrib[] =
	{
		ATTRIB_DC,
		ATTRIB_DC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_MC,
		ATTRIB_MC,
		ATTRIB_SC,
		ATTRIB_SC,
		ATTRIB_SC,
		//ATTRIB_LUCKY,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU
	};

	static const int s_nRingUpAttrib[] =
	{
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

	static const int s_nBraUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

	static const int s_nNeckUpAttrib[] =
	{
		ATTRIB_DC,
		ATTRIB_DC,
		ATTRIB_DC,
		ATTRIB_SC,
		ATTRIB_SC,
		ATTRIB_SC,
		ATTRIB_MC,
		ATTRIB_MC,
		ATTRIB_MC,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_HIDE,
		ATTRIB_LUCKY
	};

	static const int s_nClothUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_SC,
		ATTRIB_MC
	};

	static const int s_nHelmetUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

	static const int s_nShoeUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

	static const int s_nBeltUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

	static const int s_nGemUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

#ifdef _DEBUG
	if(_pItem->type == ITEM_WEAPON ||
		_pItem->type == ITEM_RING ||
		_pItem->type == ITEM_BRACELAT ||
		_pItem->type == ITEM_CLOTH)
	{
		//DEBUG_BREAK;
	}
#endif

	//const int nProp = 35000;
	int nProp = _nProb;
	if(nProp < MAGIC_ITEM_MIN)
	{
		nProp = MAGIC_ITEM_MIN;
	}

	bool bCanUp = false;
	static int nCrash = 1000;
	if(nCrash > /*60000*/nProp/* + 5000*/)
	{
		nCrash = 1000;
	}
	int nRandom = rand() % /*50000*/nProp;
	if(nRandom <= nCrash &&
		nRandom >= (nCrash - 1000))
	{
		bCanUp = true;
	}

	if(!bCanUp)
	{
		return;
	}

	int nTotalUp = 0;
	nRandom -= (nCrash - 1000);

	if(nRandom == 0)
	{
		//	1/1000
		nTotalUp = 8;
	}
	else if(nRandom >= 1 &&
		nRandom <= 4)
	{
		//	4/1000
		nTotalUp = 7;
	}
	else if(nRandom >= 5 &&
		nRandom <= 19)
	{
		//	15/1000
		nTotalUp = 6;
	}
	else if(nRandom >= 20 &&
		nRandom <= 44)
	{
		//	25/1000
		nTotalUp = 5;
	}
	else if(nRandom >= 45 &&
		nRandom <= 94)
	{
		//	50/1000
		nTotalUp = 4;
	}
	else if(nRandom >= 95 &&
		nRandom <= 194)
	{
		//	100/1000
		nTotalUp = 3;
	}
	else if(nRandom >= 195 &&
		nRandom <= 394)
	{
		//	200/1000
		nTotalUp = 2;
	}
	else
	{
		//	605/1000
		nTotalUp = 1;
	}

	/*if(nRandom >= 0 &&
		nRandom <= 0)
	{
		nTotalUp = 7;
	}
	else if(nRandom >= 1 &&
		nRandom <= 3)
	{
		nTotalUp = 6;
	}
	else if(nRandom >= 4 &&
		nRandom <= 8)
	{
		nTotalUp = 5;
	}
	else if(nRandom >= 9 &&
		nRandom <= 14)
	{
		nTotalUp = 4;
	}
	else if(nRandom >= 15 &&
		nRandom <= 80)
	{
		nTotalUp = 3;
	}
	else if(nRandom >= 81 &&
		nRandom <= 250)
	{
		nTotalUp = 2;
	}
	else
	{
		nTotalUp = 1;
	}*/

	int nLoopCounter = 0;
	int nSize = 0;
	int nIndex = 0;
	int nUp = 0;
	bool bOk = false;

	if(_pItem->type == ITEM_WEAPON)
	{
		nSize = sizeof(s_nWeaponUpAttrib) / sizeof(s_nWeaponUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nWeaponUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_CLOTH)
	{
		nSize = sizeof(s_nClothUpAttrib) / sizeof(s_nClothUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nClothUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_RING)
	{
		nSize = sizeof(s_nRingUpAttrib) / sizeof(s_nRingUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nRingUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_HELMET)
	{
		nSize = sizeof(s_nHelmetUpAttrib) / sizeof(s_nHelmetUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nHelmetUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_BRACELAT)
	{
		nSize = sizeof(s_nBraUpAttrib) / sizeof(s_nBraUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nBraUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_NECKLACE)
	{
		nSize = sizeof(s_nNeckUpAttrib) / sizeof(s_nNeckUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nNeckUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			if(nIndex == ATTRIB_LUCKY &&
				_pItem->lucky + nUp > 3)
			{
				continue;
			}

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_SHOE)
	{
		nSize = sizeof(s_nShoeUpAttrib) / sizeof(s_nShoeUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nShoeUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_BELT)
	{
		nSize = sizeof(s_nBeltUpAttrib) / sizeof(s_nBeltUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nBeltUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_GEM)
	{
		nSize = sizeof(s_nGemUpAttrib) / sizeof(s_nGemUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nGemUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}

	nCrash += 1000;
}

bool DBThread::UpgradeAttrib(ItemAttrib* _pItem, int _index, int _value)
{
	RECORD_FUNCNAME_DB;

	bool bOk = false;

	if(_index == ATTRIB_DC)
	{
		_pItem->maxDC += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_MC)
	{
		_pItem->maxMC += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_SC)
	{
		_pItem->maxSC += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_AC)
	{
		_pItem->maxAC += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_MAC)
	{
		_pItem->maxMAC += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_LUCKY)
	{
		_pItem->lucky += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_ACCU)
	{
		_pItem->accuracy += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_HIDE)
	{
		_pItem->hide += _value;
		bOk = true;
	}

	if(bOk)
	{
		int nPreAdd = 0;
		int nValue = 0;

		if(_pItem->level != 0)
		{
			BYTE bLow = LOBYTE(_pItem->level);
			BYTE bHigh = HIBYTE(_pItem->level);
			BYTE bKey = GetItemMakeMask(bHigh);
			nValue = GetMakeMaskValue(bKey);
			nPreAdd = bLow - nValue;
		}
		nPreAdd += _value;

		int nRandom = 1 + rand() % 254;
		nValue = GetItemMakeMask(nRandom);
		nValue = GetMakeMaskValue(nValue);
		nPreAdd += nValue;
		_pItem->level = MAKEWORD(nPreAdd, nRandom);
	}
	return bOk;
}

//////////////////////////////////////////////////////////////////////////
int DBCALLBACK DBThread::DBItemAttribCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	RECORD_FUNCNAME_DB;
	//ItemAttrib* pItem = (ItemAttrib*)_pParam;
	DBOperationParam* pParam = (DBOperationParam*)_pParam;
	ItemAttrib* pItem = (ItemAttrib*)pParam->dwParam[0];

	assert(_nCount == ITEMATTRIB_COL);

	pItem->id		 = atoi(_pValue[0]);
	strcpy(pItem->name, _pValue[1]);
	pItem->lucky	 = atoi(_pValue[2]);
	pItem->curse	 = atoi(_pValue[3]);
	pItem->hide		 = atoi(_pValue[4]);
	pItem->accuracy	 = atoi(_pValue[5]);
	pItem->atkSpeed	 = atoi(_pValue[6]);
	pItem->atkPalsy  = atoi(_pValue[7]);
	pItem->atkPois  = atoi(_pValue[8]);
	pItem->moveSpeed  = atoi(_pValue[9]);
	pItem->weight  = atoi(_pValue[10]);
	pItem->reqType  = atoi(_pValue[11]);
	pItem->reqValue  = atoi(_pValue[12]);
	pItem->sex  = atoi(_pValue[13]);
	pItem->type  = atoi(_pValue[14]);
	pItem->maxDC  = atoi(_pValue[15]);
	pItem->DC  = atoi(_pValue[16]);
	pItem->maxAC  = atoi(_pValue[17]);
	pItem->AC  = atoi(_pValue[18]);
	pItem->maxMAC  = atoi(_pValue[19]);
	pItem->MAC  = atoi(_pValue[20]);
	pItem->maxSC  = atoi(_pValue[21]);
	pItem->SC  = atoi(_pValue[22]);
	pItem->maxMC  = atoi(_pValue[23]);
	pItem->MC  = atoi(_pValue[24]);
	pItem->maxHP  = atoi(_pValue[25]);
	pItem->HP  = atoi(_pValue[26]);
	pItem->maxMP  = atoi(_pValue[27]);
	pItem->MP  = atoi(_pValue[28]);
	pItem->maxEXPR	= atoi(_pValue[29]);
	pItem->EXPR  = atoi(_pValue[30]);
	pItem->level  = atoi(_pValue[31]);
	pItem->extra  = atoi(_pValue[32]);
	pItem->tex  = atoi(_pValue[33]);
	pItem->price = atoi(_pValue[34]);

	//DelayedProcess dp;
	//dp.uOp = DP_ITEMLOADED;
	//dp.uParam0 = (unsigned int)pParam;
	//GameWorld::GetInstance().AddDelayedProcess(&dp);
	//m_xGameDelayProcess.push_back(pParam);
	DBThread::GetInstance()->PushGameDelay(pParam);
	return 0;
}

int DBCALLBACK DBThread::DBItemVerifyCallback(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	ItemAttrib stOriginItem;
	ItemAttrib* pItem = &stOriginItem;
	assert(_nCount == ITEMATTRIB_COL);

	if(s_bHeroValid == false)
	{
		return 0;
	}

	pItem->id		 = atoi(_pValue[0]);
	strcpy(pItem->name, _pValue[1]);
	pItem->lucky	 = atoi(_pValue[2]);
	pItem->curse	 = atoi(_pValue[3]);
	pItem->hide		 = atoi(_pValue[4]);
	pItem->accuracy	 = atoi(_pValue[5]);
	pItem->atkSpeed	 = atoi(_pValue[6]);
	pItem->atkPalsy  = atoi(_pValue[7]);
	pItem->atkPois  = atoi(_pValue[8]);
	pItem->moveSpeed  = atoi(_pValue[9]);
	pItem->weight  = atoi(_pValue[10]);
	pItem->reqType  = atoi(_pValue[11]);
	pItem->reqValue  = atoi(_pValue[12]);
	pItem->sex  = atoi(_pValue[13]);
	pItem->type  = atoi(_pValue[14]);
	pItem->maxDC  = atoi(_pValue[15]);
	pItem->DC  = atoi(_pValue[16]);
	pItem->maxAC  = atoi(_pValue[17]);
	pItem->AC  = atoi(_pValue[18]);
	pItem->maxMAC  = atoi(_pValue[19]);
	pItem->MAC  = atoi(_pValue[20]);
	pItem->maxSC  = atoi(_pValue[21]);
	pItem->SC  = atoi(_pValue[22]);
	pItem->maxMC  = atoi(_pValue[23]);
	pItem->MC  = atoi(_pValue[24]);
	pItem->maxHP  = atoi(_pValue[25]);
	pItem->HP  = atoi(_pValue[26]);
	pItem->maxMP  = atoi(_pValue[27]);
	pItem->MP  = atoi(_pValue[28]);
	pItem->maxEXPR	= atoi(_pValue[29]);
	pItem->EXPR  = atoi(_pValue[30]);
	pItem->level  = atoi(_pValue[31]);
	pItem->extra  = atoi(_pValue[32]);
	pItem->tex  = atoi(_pValue[33]);
	pItem->price = atoi(_pValue[34]);

	ItemAttrib* pCheckItem = (ItemAttrib*)_pParam;
	int nUpgrade = 0;
	int nDif = 0;

	if(pCheckItem->reqType != pItem->reqType)
	{
		s_bHeroValid = false;
		return 0;
	}
	if(pCheckItem->reqValue != pItem->reqValue)
	{
		s_bHeroValid = false;
		return 0;
	}
	if(pCheckItem->weight != pItem->weight)
	{
		s_bHeroValid = false;
		return 0;
	}

	if(pCheckItem->DC != pItem->DC)
	{
		s_bHeroValid = false;
		return 0;
	}
	nDif = pCheckItem->maxDC - pItem->maxDC;
	if(nDif < 0)
	{
		s_bHeroValid = false;
		return 0;
	}
	nUpgrade += nDif;
	if(pCheckItem->MC != pItem->MC)
	{
		s_bHeroValid = false;
		return 0;
	}
	nDif = pCheckItem->maxMC - pItem->maxMC;
	if(nDif < 0)
	{
		s_bHeroValid = false;
		return 0;
	}
	nUpgrade += nDif;
	if(pCheckItem->SC != pItem->SC)
	{
		s_bHeroValid = false;
		return 0;
	}
	nDif = pCheckItem->maxSC - pItem->maxSC;
	if(nDif < 0)
	{
		s_bHeroValid = false;
		return 0;
	}
	nUpgrade += nDif;
	if(pCheckItem->AC != pItem->AC)
	{
		s_bHeroValid = false;
		return 0;
	}
	nDif = pCheckItem->maxAC - pItem->maxAC;
	if(nDif < 0)
	{
		s_bHeroValid = false;
		return 0;
	}
	nUpgrade += nDif;
	if(pCheckItem->MAC != pItem->MAC)
	{
		s_bHeroValid = false;
		return 0;
	}
	nDif = pCheckItem->maxMAC - pItem->maxMAC;
	if(nDif < 0)
	{
		s_bHeroValid = false;
		return 0;
	}
	nUpgrade += nDif;

	nDif = pCheckItem->accuracy - pItem->accuracy;
	if(nDif < 0)
	{
		s_bHeroValid = false;
		return false;
	}
	nUpgrade += nDif;

	nDif = pCheckItem->hide - pItem->hide;
	if(nDif < 0)
	{
		s_bHeroValid = false;
		return false;
	}
	nUpgrade += nDif;

	nDif = pCheckItem->moveSpeed - pItem->moveSpeed;
	if(nDif < 0)
	{
		s_bHeroValid = false;
		return false;
	}
	nUpgrade += nDif;

	nDif = pCheckItem->atkSpeed - pItem->atkSpeed;
	if(nDif < 0)
	{
		s_bHeroValid = false;
		return false;
	}
	nUpgrade += nDif;

	nDif = pCheckItem->lucky - pItem->lucky;
	if(nDif < 0)
	{
		s_bHeroValid = false;
		return false;
	}
	nUpgrade += nDif;

	if(nUpgrade > 0)
	{
		if(nUpgrade > 8)
		{
			s_bHeroValid = false;
			return 0;
		}
		else
		{
			BYTE bLow = LOBYTE(pCheckItem->level);
			BYTE bHigh = HIBYTE(pCheckItem->level);

			if(bHigh == 0 &&
				bLow == 0)
			{
				s_bHeroValid = false;
				return 0;
				//	Just for old version, new version game will remove this
				if(bLow != nUpgrade)
				{
					//s_bHeroValid = false;
					//return 0;
					//	Only for this version
					//	Early 1.152 version have no over attrib level flag
					if(s_uHeroVersion == BACKMIR_VERSION115)
					{
						/*if(bLow == 0)
						{
							pCheckItem->level = MAKEWORD(nUpgrade, bHigh);
						}
						else*/
						{
							s_bHeroValid = false;
							return 0;
						}
					}
					else
					{
						s_bHeroValid = false;
						return 0;
					}
				}
				else
				{
					//	1.153 has low level flag but has no high level flag, just recreate the level flag
					int nRandom = rand() % 254 + 1;
					BYTE bTag = nRandom;
					BYTE bKey = GetItemMakeMask(bTag);
					int nValue = GetMakeMaskValue(bKey);
					bLow += nValue;
					WORD wLevel = MAKEWORD(bLow, bTag);
					pCheckItem->level = wLevel;
				}
			}
			else
			{
				BYTE bKey = GetItemMakeMask(bHigh);
				int nValue = GetMakeMaskValue(bKey);
				if((int)bLow - nValue < 0)
				{
					s_bHeroValid = false;
					return 0;
				}
				nValue = (int)bLow - nValue;
				if(nValue != nUpgrade)
				{
					s_bHeroValid = false;
					return 0;
				}
			}
		}
	}

	return 0;
}

int DBCALLBACK DBThread::DBItemVerifySimpleCallback(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	ItemAttrib stOriginItem;
	ItemAttrib* pItem = &stOriginItem;
	assert(_nCount == ITEMATTRIB_COL);

	if(s_bHeroValidThread2 == false)
	{
		return 0;
	}

	pItem->id		 = atoi(_pValue[0]);
	strcpy(pItem->name, _pValue[1]);
	pItem->lucky	 = atoi(_pValue[2]);
	pItem->curse	 = atoi(_pValue[3]);
	pItem->hide		 = atoi(_pValue[4]);
	pItem->accuracy	 = atoi(_pValue[5]);
	pItem->atkSpeed	 = atoi(_pValue[6]);
	pItem->atkPalsy  = atoi(_pValue[7]);
	pItem->atkPois  = atoi(_pValue[8]);
	pItem->moveSpeed  = atoi(_pValue[9]);
	pItem->weight  = atoi(_pValue[10]);
	pItem->reqType  = atoi(_pValue[11]);
	pItem->reqValue  = atoi(_pValue[12]);
	pItem->sex  = atoi(_pValue[13]);
	pItem->type  = atoi(_pValue[14]);
	pItem->maxDC  = atoi(_pValue[15]);
	pItem->DC  = atoi(_pValue[16]);
	pItem->maxAC  = atoi(_pValue[17]);
	pItem->AC  = atoi(_pValue[18]);
	pItem->maxMAC  = atoi(_pValue[19]);
	pItem->MAC  = atoi(_pValue[20]);
	pItem->maxSC  = atoi(_pValue[21]);
	pItem->SC  = atoi(_pValue[22]);
	pItem->maxMC  = atoi(_pValue[23]);
	pItem->MC  = atoi(_pValue[24]);
	pItem->maxHP  = atoi(_pValue[25]);
	pItem->HP  = atoi(_pValue[26]);
	pItem->maxMP  = atoi(_pValue[27]);
	pItem->MP  = atoi(_pValue[28]);
	pItem->maxEXPR	= atoi(_pValue[29]);
	pItem->EXPR  = atoi(_pValue[30]);
	pItem->level  = atoi(_pValue[31]);
	pItem->extra  = atoi(_pValue[32]);
	pItem->tex  = atoi(_pValue[33]);
	pItem->price = atoi(_pValue[34]);
	

	ItemAttrib stCheckItem;
	ItemAttrib* pCheckItem = (ItemAttrib*)_pParam;
	memcpy(&stCheckItem, pCheckItem, sizeof(ItemAttrib));
	//	Decrypt
	ObjectValid::DecryptAttrib(&stCheckItem);
	pCheckItem = &stCheckItem;

	int nUpgrade = 0;
	int nDif = 0;

	if(pCheckItem->reqType != pItem->reqType)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	if(pCheckItem->reqValue != pItem->reqValue)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	if(pCheckItem->weight != pItem->weight)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}

	if(pCheckItem->DC != pItem->DC)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	nDif = pCheckItem->maxDC - pItem->maxDC;
	if(nDif < 0)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	nUpgrade += nDif;
	if(pCheckItem->MC != pItem->MC)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	nDif = pCheckItem->maxMC - pItem->maxMC;
	if(nDif < 0)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	nUpgrade += nDif;
	if(pCheckItem->SC != pItem->SC)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	nDif = pCheckItem->maxSC - pItem->maxSC;
	if(nDif < 0)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	nUpgrade += nDif;
	if(pCheckItem->AC != pItem->AC)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	nDif = pCheckItem->maxAC - pItem->maxAC;
	if(nDif < 0)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	nUpgrade += nDif;
	if(pCheckItem->MAC != pItem->MAC)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	nDif = pCheckItem->maxMAC - pItem->maxMAC;
	if(nDif < 0)
	{
		s_bHeroValidThread2 = false;
		return 0;
	}
	nUpgrade += nDif;

	nDif = pCheckItem->accuracy - pItem->accuracy;
	if(nDif < 0)
	{
		s_bHeroValidThread2 = false;
		return false;
	}
	nUpgrade += nDif;

	nDif = pCheckItem->hide - pItem->hide;
	if(nDif < 0)
	{
		s_bHeroValidThread2 = false;
		return false;
	}
	nUpgrade += nDif;

	nDif = pCheckItem->moveSpeed - pItem->moveSpeed;
	if(nDif < 0)
	{
		s_bHeroValidThread2 = false;
		return false;
	}
	nUpgrade += nDif;

	nDif = pCheckItem->atkSpeed - pItem->atkSpeed;
	if(nDif < 0)
	{
		s_bHeroValidThread2 = false;
		return false;
	}
	nUpgrade += nDif;

	nDif = pCheckItem->lucky - pItem->lucky;
	if(nDif < 0)
	{
		s_bHeroValidThread2 = false;
		return false;
	}
	nUpgrade += nDif;

	if(nUpgrade > 0)
	{
		if(nUpgrade > 8)
		{
			s_bHeroValid = false;
			return 0;
		}
		else
		{
			BYTE bLow = LOBYTE(pCheckItem->level);
			BYTE bHigh = HIBYTE(pCheckItem->level);

			if(bHigh == 0 &&
				bLow == 0)
			{
				s_bHeroValidThread2 = false;
				return 0;
			}
			else
			{
				BYTE bKey = GetItemMakeMask(bHigh);
				int nValue = GetMakeMaskValue(bKey);
				if((int)bLow - nValue < 0)
				{
					s_bHeroValidThread2 = false;
					return 0;
				}
				nValue = (int)bLow - nValue;
				if(nValue != nUpgrade)
				{
					s_bHeroValidThread2 = false;
					return 0;
				}
			}
		}
	}

	return 0;
}

int DBCALLBACK DBThread::DBMonsAttribCallback(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	RECORD_FUNCNAME_DB;

	DBOperationParam* pParam = (DBOperationParam*)_pParam;
	GameObject* pObj = (GameObject*)pParam->dwParam[0];
	ItemAttrib* pItem = &pObj->GetUserData()->stAttrib;

	assert(_nCount == ITEMATTRIB_COL);

	pItem->id		 = atoi(_pValue[0]);
	strcpy(pItem->name, _pValue[1]);
	pItem->lucky	 = atoi(_pValue[2]);
	pItem->curse	 = atoi(_pValue[3]);
	pItem->hide		 = atoi(_pValue[4]);
	pItem->accuracy	 = atoi(_pValue[5]);
	pItem->atkSpeed	 = atoi(_pValue[6]);
	pItem->atkPalsy  = atoi(_pValue[7]);
	pItem->atkPois  = atoi(_pValue[8]);
	pItem->moveSpeed  = atoi(_pValue[9]);
	pItem->weight  = atoi(_pValue[10]);
	pItem->reqType  = atoi(_pValue[11]);
	pItem->reqValue  = atoi(_pValue[12]);
	pItem->sex  = atoi(_pValue[13]);
	pItem->type  = atoi(_pValue[14]);
	pItem->maxDC  = atoi(_pValue[15]);
	pItem->DC  = atoi(_pValue[16]);
	pItem->maxAC  = atoi(_pValue[17]);
	pItem->AC  = atoi(_pValue[18]);
	pItem->maxMAC  = atoi(_pValue[19]);
	pItem->MAC  = atoi(_pValue[20]);
	pItem->maxSC  = atoi(_pValue[21]);
	pItem->SC  = atoi(_pValue[22]);
	pItem->maxMC  = atoi(_pValue[23]);
	pItem->MC  = atoi(_pValue[24]);
	pItem->maxHP  = atoi(_pValue[25]);
	pItem->HP  = atoi(_pValue[26]);
	pItem->maxMP  = atoi(_pValue[27]);
	pItem->MP  = atoi(_pValue[28]);
	pItem->maxEXPR	= atoi(_pValue[29]);
	pItem->EXPR  = atoi(_pValue[30]);
	pItem->level  = atoi(_pValue[31]);
	pItem->extra  = atoi(_pValue[32]);
	pItem->tex  = atoi(_pValue[33]);
	pItem->price = atoi(_pValue[34]);

	//DelayedProcess dp;
	//dp.uOp = DP_MONSLOADED;
	//dp.uParam0 = (unsigned int)pParam;
	//GameWorld::GetInstance().AddDelayedProcess(&dp);
	//m_xGameDelayProcess.push_back(pParam);
	DBThread::GetInstance()->PushGameDelay(pParam);
	return 0;
}

int DBCALLBACK DBThread::DBDropItemCallback(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	RECORD_FUNCNAME_DB;

	DBOperationParam* pParam = (DBOperationParam*)_pParam;
	std::list<int>* pDropItems = (std::list<int>*)pParam->dwParam[2];
	int nCanDrop = LOWORD(pParam->dwParam[6]);
	if(HIWORD(pParam->dwParam[6] != 250))
	{
		return 0;
	}
	if(nCanDrop != 1)
	{
		return 0;
	}

	if(_nCount <= 1)
	{
		return 0;
	}

	float fProbMulti = LOWORD(pParam->dwParam[3]);
	if(fProbMulti == 0.0f)
	{
		fProbMulti = 1.0f;
	}
	else
	{
		fProbMulti /= 10;
	}

	if(DBThread::GetInstance()->m_nWorldDropMulti > 0 &&
		DBThread::GetInstance()->m_nWorldDropMulti < 5)
	{
		fProbMulti *= DBThread::GetInstance()->m_nWorldDropMulti;
	}

	const char* pData = _pValue[1];
	size_t nLen = strlen(pData);
	char szProperty[10];
	szProperty[0] = 0;
	char szItemID[10];
	szItemID[0] = 0;
	int nIndex = 0;
	int nItemID = 0;
	int nProperty = 0;
	int nRandom = 0;
	bool bReadItemID = true;

	int nMaxRandomValue = 0;
	int nCrashNumber = 1;

	for(size_t i = 0; i <= nLen; ++i)
	{
		if(pData[i] == ',' ||
			pData[i] == 0)
		{
			szProperty[nIndex] = 0;
			nProperty = atoi(szProperty);
			nIndex = 0;
			if(strlen(szItemID))
			{
				nItemID = atoi(szItemID);
				szItemID[0] = 0;

				nRandom = 0;
				if(nProperty != 0)
				{
					if(fProbMulti == 1.0f)
					{
						nMaxRandomValue = nProperty - 1;
						nRandom = rand() % nProperty;
					}
					else
					{
						float fProb = nProperty;
						fProb /= fProbMulti;

						if(fProb < 1.0f)
						{
							fProb = 1.0f;
						}

						nProperty = fProb;
						nMaxRandomValue = nProperty - 1;
						nRandom = rand() % nProperty;
					}
				}

				++nCrashNumber;
				if(nCrashNumber > nMaxRandomValue)
				{
					nCrashNumber = 0;
				}
				
				if(nRandom == nCrashNumber)
				{
					pDropItems->push_back(nItemID);
#ifdef _DEBUG
					LOG(INFO) << "掉落物品[" << nItemID << "]";
#endif
				}
			}
			bReadItemID = true;
		}
		else if(pData[i] == '|')
		{
			szItemID[nIndex] = 0;
			nIndex = 0;
			bReadItemID = false;
		}
		else
		{
			if(bReadItemID)
			{
				szItemID[nIndex++] = pData[i];
			}
			else
			{
				szProperty[nIndex++] = pData[i];
			}
		}
	}

	//DelayedProcess dp;
	//dp.uOp = DP_DROPITEMLOADED;
	//dp.uParam0 = (unsigned int)pParam;
	//GameWorld::GetInstance().AddDelayedProcess(&dp);
	//m_xGameDelayProcess.push_back(pParam);
	DBThread::GetInstance()->PushGameDelay(pParam);
	return 0;
}

int DBCALLBACK DBThread::DBDropItemExCallback(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	RECORD_FUNCNAME_DB;

	DBOperationParam* pParam = (DBOperationParam*)_pParam;
	std::list<int>* pDropItems = (std::list<int>*)pParam->dwParam[2];
	int nCanDrop = LOWORD(pParam->dwParam[6]);
	/*if(HIWORD(pParam->dwParam[6] != 250))
	{
		return 0;
	}*/
	if(nCanDrop != 1)
	{
		return 0;
	}

	if(_nCount <= 1)
	{
		return 0;
	}

	float fProbMulti = 1.0f;
	BYTE bMonsType = LOWORD(pParam->dwParam[3]);

	if(bMonsType == 1)
	{
		//	Elite
		fProbMulti = MONS_ELITE_DROP;
	}
	else if(bMonsType == 2)
	{
		//	Leader
		fProbMulti = MONS_LEADER_DROP;
	}

	if(DBThread::GetInstance()->m_nWorldDropMulti > 0 &&
		DBThread::GetInstance()->m_nWorldDropMulti < 5 &&
		false)
	{
		fProbMulti *= DBThread::GetInstance()->m_nWorldDropMulti;
	}
	
	if(DBThread::GetInstance()->m_nWorldDropMulti == 1)
	{
		int nHumDropMulti = LOWORD(pParam->dwParam[4]);
		if(nHumDropMulti == 2)
		{
			fProbMulti *= nHumDropMulti;
		}
	}

	/*if(fProbMulti == 0.0f)
	{
		fProbMulti = 1.0f;
	}
	else
	{
		fProbMulti /= 10;
	}*/

	const char* pData = _pValue[1];
	size_t nLen = strlen(pData);
	char szProperty[10];
	szProperty[0] = 0;
	char szItemID[10];
	szItemID[0] = 0;
	int nIndex = 0;
	int nItemID = 0;
	int nProperty = 0;
	int nRandom = 0;
	bool bReadItemID = true;

	int nMaxRandomValue = 0;
	int nCrashNumber = 1;

	for(size_t i = 0; i <= nLen; ++i)
	{
		if(pData[i] == ',' ||
			pData[i] == 0)
		{
			szProperty[nIndex] = 0;
			nProperty = atoi(szProperty);
			nIndex = 0;
			if(strlen(szItemID))
			{
				nItemID = atoi(szItemID);
				szItemID[0] = 0;

				nRandom = 0;
				if(nProperty != 0)
				{
					if(fProbMulti == 1.0f)
					{
						nMaxRandomValue = nProperty - 1;
						nRandom = rand() % nProperty;
					}
					else
					{
						float fProb = nProperty;
						fProb /= fProbMulti;

						if(fProb < 1.0f)
						{
							fProb = 1.0f;
						}

						nProperty = fProb;
						nMaxRandomValue = nProperty - 1;
						nRandom = rand() % nProperty;
					}
				}

				++nCrashNumber;
				if(nCrashNumber > nMaxRandomValue)
				{
					nCrashNumber = 0;
				}

				if(nRandom == nCrashNumber)
				{
					pDropItems->push_back(nItemID);
#ifdef _DEBUG
					LOG(INFO) << "掉落物品[" << nItemID << "]";
#endif
				}
			}
			bReadItemID = true;
		}
		else if(pData[i] == '|')
		{
			szItemID[nIndex] = 0;
			nIndex = 0;
			bReadItemID = false;
		}
		else
		{
			if(bReadItemID)
			{
				szItemID[nIndex++] = pData[i];
			}
			else
			{
				szProperty[nIndex++] = pData[i];
			}
		}
	}

	int nExtraProb = MAGIC_ITEM_BASE;
	//nExtraProb -= (LOWORD(pParam->dwParam[3]) * 1000);
	if(bMonsType == 1)
	{
		nExtraProb -= MONS_ELITE_MITEM;
	}
	else if(bMonsType == 2)
	{
		nExtraProb -= MONS_LEADER_MITEM;
	}
	nExtraProb -= (HIWORD(pParam->dwParam[6]) * 1000);

	if(nExtraProb < MAGIC_ITEM_MIN)
	{
		nExtraProb = MAGIC_ITEM_MIN;
	}

	//	Create ground item
	std::list<int> xGroundItems;
	int nDropItemAttribID = 0;
	int nPosSize = 0;
	int nPosX = 0;
	int nPosY = 0;
	bool bCanDrop = true;
	int nWhileCounter = 0;
	WORD wPosX = LOWORD(pParam->dwParam[1]);
	WORD wPosY = HIWORD(pParam->dwParam[1]);
	GameScene* pScene = GameSceneManager::GetInstance()->GetScene(HIWORD(pParam->dwParam[3]));

	if(!pDropItems->empty())
	{
		std::list<int>::const_iterator begIter = pDropItems->begin();
		std::list<int>::const_iterator endIter = pDropItems->end();

		for(begIter;
			begIter != endIter;
			++begIter)
		{
			nDropItemAttribID = *begIter;

			bCanDrop = true;
			nWhileCounter = 0;

			nPosX = wPosX;
			nPosY = wPosY;
			nPosX += g_nSearchPoint[nPosSize * 2];
			nPosY += g_nSearchPoint[nPosSize * 2 + 1];
			++nPosSize;
			if(nPosSize >= sizeof(g_nSearchPoint) / 2)
			{
				nPosSize = 0;
			}

			nWhileCounter = 0;

			//	Thread-safe function
			while(!pScene->CanDrop(nPosX, nPosY))
			{
				nPosX = wPosX;
				nPosY = wPosY;
				nPosX += g_nSearchPoint[nPosSize * 2];
				nPosY += g_nSearchPoint[nPosSize * 2 + 1];
				++nPosSize;
				if(nPosSize >= sizeof(g_nSearchPoint) / 2)
				{
					nPosSize = 0;
				}
				++nWhileCounter;
				if(nWhileCounter >= 25)
				{
					bCanDrop = false;
					break;
				}
			}

			if(bCanDrop)
			{
				GroundItem* pItem = new GroundItem;
				pItem->wPosX = nPosX;
				pItem->wPosY = nPosY;
				if(GetRecordInItemTable(nDropItemAttribID, &pItem->stAttrib))
				{
					DBThread::GetInstance()->UpgradeItems(&pItem->stAttrib, nExtraProb);
					xGroundItems.push_back((int)pItem);
				}
				else
				{
					SAFE_DELETE(pItem);
				}
			}
		}
	}

	pDropItems->clear();
	if(!xGroundItems.empty())
	{
		*pDropItems = xGroundItems;
	}

	//DelayedProcess dp;
	//dp.uOp = DP_DROPITEMLOADED;
	//dp.uParam0 = (unsigned int)pParam;
	//GameWorld::GetInstance().AddDelayedProcess(&dp);
	//m_xGameDelayProcess.push_back(pParam);
	DBThread::GetInstance()->PushGameDelay(pParam);
	return 0;
}

int DBCALLBACK DBThread::DBDropItemExCallback_Lua(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	RECORD_FUNCNAME_DB;

	DBOperationParam* pParam = (DBOperationParam*)_pParam;
	std::list<int>* pDropItems = (std::list<int>*)pParam->dwParam[2];
	int nCanDrop = LOWORD(pParam->dwParam[6]);
	/*if(HIWORD(pParam->dwParam[6] != 250))
	{
		return 0;
	}*/
	if(nCanDrop != 1)
	{
		return 0;
	}

	if(_nCount <= 1)
	{
		return 0;
	}

	float fProbMulti = 1.0f;
	BYTE bMonsType = LOWORD(pParam->dwParam[3]);

	if(bMonsType == 1)
	{
		//	Elite
		fProbMulti = MONS_ELITE_DROP;
	}
	else if(bMonsType == 2)
	{
		//	Leader
		fProbMulti = MONS_LEADER_DROP;
	}

	if(DBThread::GetInstance()->m_nWorldDropMulti > 0 &&
		DBThread::GetInstance()->m_nWorldDropMulti < 5 &&
		false)
	{
		fProbMulti *= DBThread::GetInstance()->m_nWorldDropMulti;
	}
	
	if(DBThread::GetInstance()->m_nWorldDropMulti == 1)
	{
		int nHumDropMulti = LOWORD(pParam->dwParam[4]);
		if(nHumDropMulti == 2)
		{
			fProbMulti *= nHumDropMulti;
		}
	}

	/*if(fProbMulti == 0.0f)
	{
		fProbMulti = 1.0f;
	}
	else
	{
		fProbMulti /= 10;
	}*/

	const char* pData = _pValue[1];
	size_t nLen = strlen(pData);

	int nExtraProb = MAGIC_ITEM_BASE;
	//nExtraProb -= (LOWORD(pParam->dwParam[3]) * 1000);
	if(bMonsType == 1)
	{
		nExtraProb -= MONS_ELITE_MITEM;
	}
	else if(bMonsType == 2)
	{
		nExtraProb -= MONS_LEADER_MITEM;
	}
	nExtraProb -= (HIWORD(pParam->dwParam[6]) * 1000);

	if(nExtraProb < MAGIC_ITEM_MIN)
	{
		nExtraProb = MAGIC_ITEM_MIN;
	}

	//	Create ground item
	ItemIDList xGroundItems;
	int nPosSize = 0;
	int nPosX = 0;
	int nPosY = 0;
	bool bCanDrop = true;
	int nWhileCounter = 0;
	int nDropItemIndex = 0;
	WORD wPosX = LOWORD(pParam->dwParam[1]);
	WORD wPosY = HIWORD(pParam->dwParam[1]);
	GameScene* pScene = GameSceneManager::GetInstance()->GetScene(HIWORD(pParam->dwParam[3]));

	//	这里lua内保存了一份掉落表
	lua_State* L = DBThread::GetInstance()->GetLuaState();
	lua_getglobal(L, "DropItemCallback");
	lua_pushstring(L, pData);
	tolua_pushusertype(L, pScene, "GameScene");
	lua_pushnumber(L, fProbMulti);
	int nRet = lua_pcall(L, 3, 0, 0);
	if(0 != nRet)
	{
#ifdef _DEBUG
		LOG(WARNING) << lua_tostring(L, -1);
#endif
		lua_pop(L, 1);
	}

	//	drop items内的内容应该和lua表内一样
	//if(!pDropItems->empty())
	{
		//std::list<int>::const_iterator begIter = pDropItems->begin();
		//std::list<int>::const_iterator endIter = pDropItems->end();

		/*for(begIter;
			begIter != endIter;
			++begIter)*/
		for(; ;)
		{
			bCanDrop = true;
			nWhileCounter = 0;

			nPosX = wPosX;
			nPosY = wPosY;
			nPosX += g_nSearchPoint[nPosSize * 2];
			nPosY += g_nSearchPoint[nPosSize * 2 + 1];
			++nPosSize;
			if(nPosSize >= sizeof(g_nSearchPoint) / 2)
			{
				nPosSize = 0;
			}

			nWhileCounter = 0;

			//	Thread-safe function
			while(!pScene->CanDrop(nPosX, nPosY))
			{
				nPosX = wPosX;
				nPosY = wPosY;
				nPosX += g_nSearchPoint[nPosSize * 2];
				nPosY += g_nSearchPoint[nPosSize * 2 + 1];
				++nPosSize;
				if(nPosSize >= sizeof(g_nSearchPoint) / 2)
				{
					nPosSize = 0;
				}
				++nWhileCounter;
				if(nWhileCounter >= 25)
				{
					bCanDrop = false;
					break;
				}
			}

			if(bCanDrop)
			{
				/*GroundItem* pItem = new GroundItem;
				pItem->wPosX = nPosX;
				pItem->wPosY = nPosY;
				if(GetRecordInItemTable(nDropItemAttribID, &pItem->stAttrib))
				{
					DBThread::GetInstance()->UpgradeItems(&pItem->stAttrib, nExtraProb);
					xGroundItems.push_back((int)pItem);
				}
				else
				{
					SAFE_DELETE(pItem);
				}*/

				lua_getglobal(L, "CreateGroundItem");
				tolua_pushusertype(L, &xGroundItems, "ItemIDList");
				lua_pushnumber(L, (int)nPosX);
				lua_pushnumber(L, (int)nPosY);
				//lua_pushnumber(L, (int)nDropItemAttribID);
				lua_pushnumber(L, (int)nExtraProb);
				lua_pushnumber(L, (int)nDropItemIndex);
				nRet = lua_pcall(L, 5, 1, 0);

				if(0 != nRet)
				{
#ifdef _DEBUG
					LOG(WARNING) << lua_tostring(L, -1);
#endif
					lua_pop(L, 1);
				}
				else
				{
					bool bRet = lua_toboolean(L, -1);
					lua_pop(L, 1);

					if(bRet == false)
					{
						break;
					}
				}
			}

			++nDropItemIndex;
		}
	}

	pDropItems->clear();
	if(!xGroundItems.empty())
	{
		*pDropItems = *(list<int>*)&xGroundItems;
	}

	//DelayedProcess dp;
	//dp.uOp = DP_DROPITEMLOADED;
	//dp.uParam0 = (unsigned int)pParam;
	//GameWorld::GetInstance().AddDelayedProcess(&dp);
	//m_xGameDelayProcess.push_back(pParam);
	DBThread::GetInstance()->PushGameDelay(pParam);
	return 0;
}

int DBCALLBACK DBThread::DBDropItemExCallback_Lua2(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	RECORD_FUNCNAME_DB;

	DBOperationParam* pParam = (DBOperationParam*)_pParam;
	DBDropDownContext xLuaDropContext(pParam);

	const char* pData = _pValue[1];
	size_t nLen = strlen(pData);

	//	这里lua内保存了一份掉落表
	lua_State* L = DBThread::GetInstance()->GetLuaState();
	lua_getglobal(L, "DropDownMonsterItemsDB");
	lua_pushstring(L, pData);
	tolua_pushusertype(L, &xLuaDropContext, "DBDropDownContext");
	int nRet = lua_pcall(L, 2, 0, 0);
	if(0 != nRet)
	{
#ifdef _DEBUG
		LOG(WARNING) << lua_tostring(L, -1);
#endif
		lua_pop(L, 1);
	}

	DBThread::GetInstance()->PushGameDelay(pParam);
	return 0;
}

int DBCALLBACK DBThread::DBItemPriceCallback(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	assert(_nCount == ITEMATTRIB_COL);
	int nID = atoi(_pValue[0]);
	assert(nID >= 0 &&
		nID < ITEM_ID_MAX);
	g_nItemPrice[nID] = atoi(_pValue[34]);
	return 0;
}

int DBCALLBACK DBThread::DBMagicAttribCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	assert(_nCount == MAGICATTRIB_COL);
	int nID = atoi(_pValue[0]);

	if(nID >= 0 &&
		nID < MEFF_USERTOTAL)
	{
		g_xMagicInfoTable[nID].wID = nID;
		g_xMagicInfoTable[nID].wLevel[0] = atoi(_pValue[1]);
		g_xMagicInfoTable[nID].wLevel[1] = atoi(_pValue[2]);
		g_xMagicInfoTable[nID].wLevel[2] = atoi(_pValue[3]);
		g_xMagicInfoTable[nID].wLevel[3] = atoi(_pValue[4]);
		g_xMagicInfoTable[nID].wLevel[4] = atoi(_pValue[5]);
		g_xMagicInfoTable[nID].wLevel[5] = atoi(_pValue[6]);
		g_xMagicInfoTable[nID].wLevel[6] = atoi(_pValue[7]);
		g_xMagicInfoTable[nID].bJob = atoi(_pValue[8]);
		g_xMagicInfoTable[nID].bMultiple = atoi(_pValue[9]);
		g_xMagicInfoTable[nID].wIncrease = atoi(_pValue[10]);
		g_xMagicInfoTable[nID].bBaseCost = atoi(_pValue[11]);
		g_xMagicInfoTable[nID].dwDelay = atoi(_pValue[12]);
	}
	return 0;
}








//////////////////////////////////////////////////////////////////////////

/*int GetItemMakeMask(BYTE _bRandom)
{
	_bRandom &= ITEM_MAKE_MASK;
	bool bZero = false;

	if((_bRandom & 0x40) == 0)
	{
		bZero = ((_bRandom & 0x10) != 0 ? false : true);
		if(bZero)
		{
			_bRandom |= 0x10;
		}
		else
		{
			_bRandom &= (~0x10);
		}
	}

	if((_bRandom & 0x02) == 0)
	{
		bZero = ((_bRandom & 0x08) != 0 ? false : true);
		if(bZero)
		{
			_bRandom |= 0x08;
		}
		else
		{
			_bRandom &= (~0x08);
		}
	}
	return _bRandom;
}

int GetMakeMaskValue(BYTE _bMask)
{
	static int s_nValueTable[] =
	{
		9, 2, 1, 3, 4, 7, 8, 5
	};
	static BYTE s_btMaskTable[] =
	{
		0x80, 0x40, 0x20, 0x10,
		0x08, 0x04, 0x02, 0x01
	};

	int nValue = 0;
	for(int i = 0; i < 8; ++i)
	{
		if((_bMask & s_btMaskTable[i]) != 0)
		{
			nValue += s_nValueTable[i];
		}
	}
	return nValue;
}*/