#include "../CMainServer/CMainServer.h"
#include "GameSceneManager.h"
#include "ObjectEngine.h"
#include "MonsterObject.h"
#include <io.h>
#include <Shlwapi.h>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include "DBThread.h"
#include "../Helper.h"
#include "GameWorld.h"
#include "struct.h"
#include "ObjectValid.h"
#include "TeammateControl.h"
#include "ExceptionHandler.h"
#include "../../CommonModule/SettingLoader.h"
#include "../../CommonModule/ExecuteTimer.h"
#include "GameInstanceScene.h"
#include "MonsterTemplateObject.h"
//////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "shlwapi.lib")
//////////////////////////////////////////////////////////////////////////
extern HWND g_hServerDlg;
//////////////////////////////////////////////////////////////////////////
const int g_nSearchPoint[] =
{
	0, 0,
	-1, 0,
	1,0,
	0,-1,
	-1,-1,
	1,-1,
	0,1,
	-1,1,
	1,1,
	-2,-2,
	-1,-2,
	0,-2,
	1,-2,
	2,-2,
	-2,-1,
	2,-1,
	-2,0,
	2,0,
	-2,1,
	2,1,
	-2,2,
	-1,2,
	0,2,
	1,2,
	2,2


	//////////////////////////////////////////////////////////////////////////
	,
	-3, -3,
	-2, -3,
	-1, -3,
	0, -3,
	1, -3,
	2, -3,
	3, -3,
	3, -2,
	3, -1,
	3, 0,
	3, 1,
	3, 2,
	3, 3,
	2, 3,
	1, 3,
	0, 3,
	-1, 3,
	-2, 3,
	-3, 3,
	-3, 2,
	-3, 1,
	-3, 0,
	-3, -1,
	-3, -2
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* Class GameScene
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
GameScene::GameScene()
{
	//m_pMapData = NULL;
	m_pCellData = NULL;
	memset(&m_stMapInfo, 0, sizeof(m_stMapInfo));
	memset(&m_stCityRect, 0, sizeof(RECT));
	InitializeCriticalSection(&m_csPlayer);
	m_dwExecuteScriptInterval = 0;
	m_dwLoopTime = 0;
	//m_dwLastUpdateObjectBlockTime = 1;
	m_dwLastGenerateMonsterTime = 0;
	m_dwLastCleanItemTime = 0;
	m_dwCityCenter = 0;
	m_bAutoReset = false;
	m_nCurPlayers = 0;
	m_nCurMonsters = 0;
	m_bIsInstance = false;
	m_bIsTreasureMap = false;
	m_bKilledMonster = false;
	m_bGiveReward = 0;
	m_dwTreasureTipTime = 0;
	m_bSaveAndStay = true;
	m_bCanUseScroll = true;
	m_dwMapUID = 0xFFFFFFFF;
	m_bCanUseMove = true;
	m_fSceneMonsterAttackMulti = m_fSceneMonsterDefenceMulti = 0.0f;
	m_nChallengeID = 0;

	m_nTimeLimitID = 0;
	m_nTimeLimitSec = 0;

	m_eMapPkType = kMapPkType_None;
}

GameScene::~GameScene()
{
	//delete[] m_pMapData;
	Release();
	m_pCellData = NULL;
	//m_pMapData = NULL;
	DeleteCriticalSection(&m_csPlayer);
}

//////////////////////////////////////////////////////////////////////////
bool GameScene::Initialize(DWORD _dwMapID)
{
	char szRootPath[MAX_PATH];
	char szMapFile[MAX_PATH];
	bool bRet = true;
	m_dwMapID = _dwMapID;

	m_xDelaySendBuf.Resize(10);
	m_xWaitDelay.Resize(10);

	GetModuleFileName(NULL, szRootPath, sizeof(szRootPath));
	PathRemoveFileSpec(szRootPath);
	strcpy(szMapFile, szRootPath);

	const char* pszMapName = NULL;

	if(_dwMapID < 100)
	{
		pszMapName = GameSceneManager::GetInstance()->GetRunMap(_dwMapID);
	}
	else
	{
		pszMapName = GameSceneManager::GetInstance()->GetInstanceMap(_dwMapID);
	}

	if(NULL == pszMapName)
	{
		return false;
	}

	sprintf(szMapFile, "%s\\Map\\%s.map",
		szRootPath, pszMapName);

	TileMap xMap;
	DWORD* pMapData = NULL;
	if(xMap.GetMapSnapShot(szMapFile, &pMapData, &m_stMapInfo))
	{
		LOG(INFO) << "载入地图[" << pszMapName << "]成功";
		//return true;
	}
	else
	{
		LOG(ERROR) << "载入地图[" << pszMapName << "]失败";
		bRet = false;
		//return false;
	}

	if(bRet == true)
	{
		if(m_pCellData)
		{
			Release();
		}
		m_pCellData = new MapCellInfo[m_stMapInfo.nCol * m_stMapInfo.nRow];
		memset(m_pCellData, 0, sizeof(MapCellInfo) * m_stMapInfo.nCol * m_stMapInfo.nRow);
		for(int i = 0; i < m_stMapInfo.nRow; ++i)
		{
			for(int j = 0; j < m_stMapInfo.nCol; ++j)
			{
				if(pMapData[j + i * m_stMapInfo.nCol])
				{
					m_pCellData[j + i * m_stMapInfo.nCol].bFlag |= BLOCK_MASK;
				}
			}
		}
	}
	delete[] pMapData;

	//	Load the script file
#ifdef _DEBUG
	sprintf(szMapFile, "%s\\Help\\",
		szRootPath);
	m_xScript.SetModulePath(szMapFile);
	m_xScript.SetLuaLoadPath(szMapFile);
#else
	sprintf(szMapFile, "%s\\Help\\dog.idx",
		szRootPath);
	m_xScript.SetModulePath(szMapFile, LOADMODE_ZIP);
	m_xScript.SetLuaLoadPath(szMapFile);
#endif
	if(!LoadScript())
	{
		LOG(ERROR) << "运行[" << "map script " << _dwMapID << "]失败";
		return false;
	}
	else
	{
#ifdef _DEBUG
		LOG(INFO) << "载入地图脚本文件[" << _dwMapID << "]成功";
#endif
		int nRet = 0;
		lua_getglobal(m_xScript.GetVM(), "OnMapLoaded");
		tolua_pushusertype(m_xScript.GetVM(), this, "GameScene");
		nRet = lua_pcall(m_xScript.GetVM(), 1, 0, 0);

		if(0 == nRet)
		{
#ifdef _DEBUG
			LOG(INFO) << "执行" << pszMapName << "[OnMapLoaded]脚本函数";
#endif
		}
		else
		{
			LOG(WARNING) << "Execute script(OnMapLoaded) On[" << _dwMapID << "]" << lua_tostring(m_xScript.GetVM(), -1);
			lua_pop(m_xScript.GetVM(), 1);
		}

		lua_getglobal(m_xScript.GetVM(), "GetVersion");
		nRet = lua_pcall(m_xScript.GetVM(), 0, 0, 0);
		if(0 == nRet)
		{

		}
		else
		{
			lua_pop(m_xScript.GetVM(), 1);
			return false;
		}
	}

	return bRet;
/*#ifdef _DEBUG
	sprintf(szMapFile, "%s\\Help\\scene%d.lua",
		szRootPath, _dwMapID);
	m_xScript.Create();
	m_xScript.SetUserTag(_dwMapID);
	tolua_BackMirServer_open(m_xScript.GetVM());

	if(m_xScript.LoadFile(szMapFile))
	{
		LOG(INFO) << "载入地图脚本文件[" << szMapFile << "]成功";
		int nRet = 0;
		lua_getglobal(m_xScript.GetVM(), "OnMapLoaded");
		tolua_pushusertype(m_xScript.GetVM(), this, "GameScene");
		nRet = lua_pcall(m_xScript.GetVM(), 1, 0, 0);

		if(0 == nRet)
		{
			LOG(INFO) << "执行" << pszMapName << "[OnMapLoaded]脚本函数";
		}
		else
		{
			LOG(WARNING) << "Execute script(OnMapLoaded) On[" << pszMapName << "]" << lua_tostring(m_xScript.GetVM(), -1);
			lua_pop(m_xScript.GetVM(), 1);
		}

		lua_getglobal(m_xScript.GetVM(), "GetVersion");
		nRet = lua_pcall(m_xScript.GetVM(), 0, 0, 0);
		if(0 == nRet)
		{

		}
		else
		{
			lua_pop(m_xScript.GetVM(), 1);
			return false;
		}
	}
	else
	{
		LOG(ERROR) << "运行[" << szMapFile << "]失败";
		bRet = false;
	}

	return bRet;
#else
	sprintf(szMapFile, "%s\\Help\\dog.idx",
		szRootPath);
	m_xScript.Create();
	m_xScript.SetUserTag(_dwMapID);
	tolua_BackMirServer_open(m_xScript.GetVM());
	char szFile[50];
#ifdef _LUAJIT_
	sprintf(szFile, "scene%d.bjt",
		_dwMapID);
#else
	sprintf(szFile, "scene%d.bbt",
		_dwMapID);
#endif

	if(m_xScript.ExecuteZip(szMapFile, szFile))
	{
		int nRet = 0;
		lua_getglobal(m_xScript.GetVM(), "OnMapLoaded");
		tolua_pushusertype(m_xScript.GetVM(), this, "GameScene");
		nRet = lua_pcall(m_xScript.GetVM(), 1, 0, 0);

		if(0 == nRet)
		{
			
		}
		else
		{
			LOG(WARNING) << "Execute script On[" << m_xScript.GetUserTag() << "]" << lua_tostring(m_xScript.GetVM(), -1);
			lua_pop(m_xScript.GetVM(), 1);
		}

		lua_getglobal(m_xScript.GetVM(), "GetVersion");
		nRet = lua_pcall(m_xScript.GetVM(), 0, 0, 0);
		if(0 == nRet)
		{

		}
		else
		{
			lua_pop(m_xScript.GetVM(), 1);
			return false;
		}
	}
	else
	{
		LOG(ERROR) << "运行[" << szMapFile << "]失败";
		bRet = false;
	}

	return bRet;
#endif*/

}

//////////////////////////////////////////////////////////////////////////
void GameScene::Release()
{
	//delete[] m_pMapData;
	//m_pMapData = NULL;
	if(m_pCellData)
	{
		for(int i = 0; i < m_stMapInfo.nRow; ++i)
		{
			for(int j = 0; j < m_stMapInfo.nCol; ++j)
			{
				if(NULL != m_pCellData[j + i * m_stMapInfo.nCol].pCellObjects)
				{
					//	Other CellInfo .. delete
					delete m_pCellData[j + i * m_stMapInfo.nCol].pCellObjects;
					m_pCellData[j + i * m_stMapInfo.nCol].pCellObjects = NULL;
				}
			}
		}
		delete[] m_pCellData;
	}
	m_pCellData = NULL;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::DeleteAllMonster()
{
	RECORD_FUNCNAME_WORLD;
	//	Only use in a instance map when all players have gone
	if(!m_xNPCs.empty())
	{
		std::map<DWORD, GameObject*>::const_iterator begiter = m_xNPCs.begin();
		GameObject* pObj = NULL;

		for(begiter;
			begiter != m_xNPCs.end();
			)
		{
			pObj = begiter->second;
			if(pObj->GetType() == SOT_MONSTER)
			{
				//if(!RemoveMappedObject(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY, pObj))
				// 宝宝也删了 导致玩家主动杀死宝宝时候 服务器崩溃 [11/15/2013 yuanxj]
				MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
				if(pMonster->GetMaster() == NULL)
				{
					if(!RemoveMappedObject(pObj->GetValidPositionX(), pObj->GetValidPositionY(), pObj))
					{
						char szName[20];
						ObjectValid::GetItemName(&pObj->GetUserData()->stAttrib, szName);
						LOG(ERROR) << "Name:" << szName << " Coordinate x:" << pObj->GetUserData()->wCoordX << " y:" << pObj->GetUserData()->wCoordY
							<< "Valid pos x:" << pObj->GetValidPositionX() << " pos y:" << pObj->GetValidPositionY();
					}
					delete pObj;
					begiter = m_xNPCs.erase(begiter);
				}
				else
				{
					//	清除宠物
					HeroObject* pMaster = static_cast<HeroObject*>(pMonster->GetMaster());
					pMaster->ClearSlave(pMonster);
					pMonster->SetMaster(NULL);

					if(!RemoveMappedObject(pObj->GetValidPositionX(), pObj->GetValidPositionY(), pObj))
					{
						char szName[20];
						ObjectValid::GetItemName(&pObj->GetUserData()->stAttrib, szName);
						LOG(ERROR) << "Name:" << szName << " Coordinate x:" << pObj->GetUserData()->wCoordX << " y:" << pObj->GetUserData()->wCoordY
							<< "Valid pos x:" << pObj->GetValidPositionX() << " pos y:" << pObj->GetValidPositionY();
					}
					delete pObj;
					begiter = m_xNPCs.erase(begiter);

					//++begiter;
				}
			}
			else
			{
				++begiter;
			}
		}
	}
}

void GameScene::DeleteAllItem()
{
	RECORD_FUNCNAME_WORLD;

	if(!m_xItems.empty())
	{
		std::map<DWORD, GroundItem*>::const_iterator begiter = m_xItems.begin();
		GroundItem* pItem = NULL;

		for(begiter;
			begiter != m_xItems.end();
			++begiter)
		{
			pItem = begiter->second;
			delete pItem;
		}

		m_xItems.clear();
	}
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::GetSceneData(HeroObject* _pObj)
{
	RECORD_FUNCNAME_WORLD;

	PkgPlayerUpdateAttribNtf uantf;
	PkgNewPlayerNot not;
	not.bNew = true;

	//	Players
	HeroObject* pHero = NULL;
	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		pHero = (HeroObject*)iter->second;
		g_xThreadBuffer.Reset();
		if(iter->second->GetID() != _pObj->GetID())
		{
			memcpy(&not.stData, iter->second->GetUserData(), sizeof(UserData));
			ObjectValid::DecryptAttrib(&not.stData.stAttrib);
			not.uHandlerID = iter->second->GetID();
			not.dwLook1 = MAKELONG(GETITEMATB(pHero->GetEquip(PLAYER_ITEM_CLOTH), ID), GETITEMATB(pHero->GetEquip(PLAYER_ITEM_WEAPON), ID));

			//	技能等级
			int nSkillLevel = pHero->GetMagicLevel(MEFF_SHIELD);
			if(0 != nSkillLevel)
			{
				not.xSkillInfo.push_back(MEFF_SHIELD);
				not.xSkillInfo.push_back(nSkillLevel);
			}

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			//SendBuffer(dwUserIndex, &g_xThreadBuffer);
			_pObj->SendPlayerBuffer(g_xThreadBuffer);

			//	vip等级
			/*int nVipLevel = pHero->GetDonateLevel();
			if(0 != nVipLevel)
			{
				PkgPlayerVipNot ppvn;
				ppvn.dwID = pHero->GetID();
				ppvn.nVipLevel = nVipLevel;
				ppvn.uTargetId = pHero->GetID();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ppvn;
				_pObj->SendPlayerBuffer(g_xThreadBuffer);
			}

			//	该玩家的状态
			PkgPlayerGStatusNtf statusNtf;
			pHero->GetStatusInfo(statusNtf);
			if(!statusNtf.xStatus.empty())
			{
				_pObj->SendPacket(statusNtf);
			}

			//	玩家的外观
			PkgPlayerExtendAttribNot pean;
			pean.uTargetId = pHero->GetID();*/
			pHero->SendPlayerDataTo(_pObj);
		}
	}

	//	NPCs
	PkgNewNPCNot stNpcNot;
	MonsterObject* pMonster = NULL;
	bool bNotify = true;
	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xNPCs.begin();
		iter != m_xNPCs.end();
		++iter)
	{
		bNotify = true;
		if(iter->second->GetType() == SOT_MONSTER)
		{
			if(iter->second->GetUserData()->eGameState == OS_DEAD &&
				TEST_FLAG_BOOL(iter->second->GetObject_MaxExpr(), MAXEXPR_MASK_DEADHIDE))
			{
				bNotify = false;
			}
		}

		if(bNotify)
		{
			stNpcNot.uHandlerID = iter->second->GetID();
			stNpcNot.uMonsID = iter->second->GetObject_ID();
			stNpcNot.uPosition = MAKELONG(iter->second->GetUserData()->wCoordX,
				iter->second->GetUserData()->wCoordY);
			stNpcNot.uParam = iter->second->GetObject_HP();

			if(TEST_FLAG_BOOL(iter->second->GetObject_MaxExpr(), MAXEXPR_MASK_ELITE) ||
				TEST_FLAG_BOOL(iter->second->GetObject_MaxExpr(), MAXEXPR_MASK_LEADER))
			{
				stNpcNot.uFlag = iter->second->GetObject_MaxExpr();
			}
			else
			{
				stNpcNot.uFlag = 0;
			}
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << stNpcNot;
			_pObj->SendPlayerBuffer(g_xThreadBuffer);
		}

		if(iter->second->GetType() == SOT_MONSTER)
		{
			pMonster = static_cast<MonsterObject*>(iter->second);
			if(pMonster->GetMaster() != NULL &&
				bNotify)
			{
				uantf.uTargetId = pMonster->GetID();
				uantf.bType = UPDATE_MASTER;
				uantf.dwParam = pMonster->GetMaster()->GetID();
				_pObj->SendPacket(uantf);

				uantf.bType = UPDATE_SLAVELV;
				uantf.dwParam = pMonster->GetObject_MaxAC();
				_pObj->SendPacket(uantf);
			}
		}
	}

	//	Items
	PkgSystemNewItemNot stItemNot;
	stItemNot.uTargetId = _pObj->GetID();
	for(std::map<DWORD, GroundItem*>::const_iterator iter = m_xItems.begin();
		iter != m_xItems.end();
		++iter)
	{
		stItemNot.dwTag = iter->first;
		stItemNot.dwID = GETITEMATB(&iter->second->stAttrib, ID);
		stItemNot.dwPos = MAKELONG(iter->second->wPosX, iter->second->wPosY);
		stItemNot.uUserId = GETITEMATB(&iter->second->stAttrib, Level);
		stItemNot.dwItemHideAttrib = GETITEMATB(&iter->second->stAttrib, MaxMP);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << stItemNot;
		_pObj->SendPlayerBuffer(g_xThreadBuffer);
	}

	//	doors
	PkgPlayerShowDoorAniAck dack;

	for(DOORPOSITIONLIST::const_iterator begiter = m_xDoorPos.begin();
		begiter != m_xDoorPos.end();
		++begiter)
	{
		dack.wMgcID = MEFF_DOOR;
		dack.wPosX = LOWORD((*begiter));
		dack.wPosY = HIWORD((*begiter));
		dack.uTargetId = _pObj->GetID();
		//	door time
		MapCellInfo* pCell = GetMapData(dack.wPosX, dack.wPosY);
		if(pCell)
		{
			if(pCell->pCellObjects != NULL)
			{
				CELLDATALIST::const_iterator begIter = pCell->pCellObjects->begin();
				CELLDATALIST::const_iterator endIter = pCell->pCellObjects->end();

				for(begIter;
					begIter != endIter;
					++begIter)
				{
					CellData* pCellData = *begIter;

					if(pCellData)
					{
						if(pCellData->bType == CELL_DOOR)
						{
							DoorEvent* pDoorEvt = (DoorEvent*)pCellData->pData;

							if(pDoorEvt &&
								pDoorEvt->dwTime != 0 &&
								GetTickCount() < pDoorEvt->dwTime)
							{
								dack.uUserId = pDoorEvt->dwTime - GetTickCount() + 1000;
							}
						}
					}
				}
			}
		}

		g_xThreadBuffer.Reset();
		g_xThreadBuffer << dack;
		_pObj->SendPlayerBuffer(g_xThreadBuffer);
	}

	_pObj->SendStatusInfo();

	//	uid
	if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
	{
		PkgPlayerUIDNtf ntf;
		ntf.uTargetId = _pObj->GetID();
		ntf.uUID = _pObj->GetUID();
		_pObj->SendPacket(ntf);
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
GameObject* GameScene::GetPlayer(DWORD _dwID)
{
	RECORD_FUNCNAME_WORLD;

	EnterCriticalSection(&m_csPlayer);
	GameObject* pFind = GetPlayerWithoutLock(_dwID);
	LeaveCriticalSection(&m_csPlayer);
	return pFind;
}

GameObject* GameScene::GetPlayerByName(const char* _pszName)
{
	RECORD_FUNCNAME_WORLD;

	//EnterCriticalSection(&m_csPlayer);
	GameObject* pFind = NULL;
	char szName[20] = {0};

	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		if(iter->second->GetType() == SOT_HERO)
		{
			ObjectValid::GetItemName(&iter->second->GetUserData()->stAttrib, szName);
			if(0 == strcmp(_pszName, szName))
			{
				return iter->second;
			}
		}
	}
	return pFind;
	//LeaveCriticalSection(&m_csPlayer);
}

GameObject* GameScene::GetPlayerWithoutLock(DWORD _dwID)
{
	RECORD_FUNCNAME_WORLD;

	GameObject* pFind = NULL;
	std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.find(_dwID);
	if(iter != m_xPlayers.end())
	{
		pFind = iter->second;
	}
	return pFind;
}
//////////////////////////////////////////////////////////////////////////
GameObject* GameScene::GetMonster(int _nAttribID)
{
	RECORD_FUNCNAME_WORLD;

	GameObject* pMonster = NULL;
	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xNPCs.begin();
		iter != m_xNPCs.end();
		++iter)
	{
		if(iter->second->GetObject_ID() == _nAttribID &&
			iter->second->GetType() == SOT_MONSTER)
		{
			return iter->second;
		}
	}
	return pMonster;
}
//////////////////////////////////////////////////////////////////////////
int GameScene::MoveSomeMonsterTo(int _nAttribID,int _nSum, int _x, int _y)
{
	RECORD_FUNCNAME_WORLD;

	int nMovedCounter = 0;
	int nMoveX = 0;
	int nMoveY = 0;

	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xNPCs.begin();
		iter != m_xNPCs.end();
		++iter)
	{
		if(iter->second->GetObject_ID() == _nAttribID &&
			iter->second->GetType() == SOT_MONSTER &&
			iter->second->GetUserData()->eGameState != OS_DEAD)
		{
			for(int i = 0; i < 8; ++i)
			{
				nMoveX = _x + g_nMoveOft[i * 2];
				nMoveY = _y + g_nMoveOft[i * 2 + 1];
				if(CanThrough(nMoveX, nMoveY))
				{
					iter->second->FlyTo(nMoveX, nMoveY);
					++nMovedCounter;
					break;
				}
			}

			if(nMovedCounter >= _nSum)
			{
				return _nSum;
			}
		}
	}
	return nMovedCounter;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::InsertPlayer(GameObject* _pObj)
{
	RECORD_FUNCNAME_WORLD;

	EnterCriticalSection(&m_csPlayer);
	bool bRet = false;

	if(GameSceneManager::GetInstance()->IsUserNameExist(_pObj->GetUserData()->stAttrib.name))
	{
		return false;
	}

	if(GameWorld::GetInstance().IsInBlackList((HeroObject*)_pObj))
	{
		LOG(INFO) << _pObj->GetUserData()->stAttrib.name << " In black list...";
		return false;
	}

	if(m_xPlayers.find(_pObj->GetID()) == m_xPlayers.end())
	{
		//	Broadcast player login
		g_xThreadBuffer.Reset();
		PkgNewPlayerNot not;
		not.bNew = true;

		if(_pObj->IsEncrypt())
		{
			if(_pObj->GetType() == SOT_HERO)
			{
				HeroObject* pHero = (HeroObject*)_pObj;
				not.dwLook1 = MAKELONG(GETITEMATB(pHero->GetEquip(PLAYER_ITEM_CLOTH), ID), GETITEMATB(pHero->GetEquip(PLAYER_ITEM_WEAPON), ID));
			}
			memcpy(&not.stData, _pObj->GetUserData(), sizeof(UserData));
			ObjectValid::DecryptAttrib(&not.stData.stAttrib);
		}
		else
		{
			if(_pObj->GetType() == SOT_HERO)
			{
				HeroObject* pHero = (HeroObject*)_pObj;
				not.dwLook1 = MAKELONG(pHero->GetEquip(PLAYER_ITEM_CLOTH)->id, pHero->GetEquip(PLAYER_ITEM_WEAPON)->id);
			}
			memcpy(&not.stData, _pObj->GetUserData(), sizeof(UserData));
		}

		//	技能等级
		if(_pObj->GetType() == SOT_HERO)
		{
			HeroObject* pHero = (HeroObject*)_pObj;
			int nSkillLevel = pHero->GetMagicLevel(MEFF_SHIELD);
			if(0 != nSkillLevel)
			{
				not.xSkillInfo.push_back(MEFF_SHIELD);
				not.xSkillInfo.push_back(nSkillLevel);
			}
		}
		
		not.uHandlerID = _pObj->GetID();
		g_xThreadBuffer << not;
		BroadcastPacket(&g_xThreadBuffer, _pObj->GetID());

		if(_pObj->GetType() == SOT_HERO)
		{
			//	广播vip等级
			/*HeroObject* pHero = (HeroObject*)_pObj;

			if(pHero->GetDonateLevel() != 0)
			{
				PkgPlayerVipNot ppvn;
				ppvn.dwID = pHero->GetID();
				ppvn.uTargetId = pHero->GetID();
				ppvn.nVipLevel = pHero->GetDonateLevel();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ppvn;
				BroadcastPacket(&g_xThreadBuffer, pHero->GetID());
			}

			//	技能状态
			PkgPlayerGStatusNtf statusNtf;
			pHero->GetStatusInfo(statusNtf);
			if(!statusNtf.xStatus.empty())
			{
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << statusNtf;
				BroadcastPacket(&g_xThreadBuffer, pHero->GetID());
			}*/
			HeroObject* pHero = (HeroObject*)_pObj;

			for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
				iter != m_xPlayers.end();
				++iter)
			{
				HeroObject* pObj = (HeroObject*)iter->second;
				if(pObj != pHero)
				{
					pHero->SendPlayerDataTo(pObj);
				}
			}
		}

		_pObj->SetMapID(m_dwMapID);
		AddMappedObject(_pObj->GetUserData()->wCoordX,
			_pObj->GetUserData()->wCoordY, CELL_MOVEOBJECT, _pObj);

		//	Get scene
		//GetScene((HeroObject*)_pObj);
		//	restore static magic
		RestoreStaticMagicByFire(_pObj);

		m_xPlayers.insert(std::make_pair(_pObj->GetID(), _pObj));
		bRet = true;
	}

	if(bRet == true)
	{
		if(IsAutoReset())
		{
			AllMonsterHPToFull();
		}
	}

	LeaveCriticalSection(&m_csPlayer);
	return bRet;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::RemovePlayer(DWORD _dwID, bool _bDelete /* = true */)
{
	RECORD_FUNCNAME_WORLD;
	//	Clean up player data...

	GameObject* pObj = NULL;
	bool bRet = false;

	EnterCriticalSection(&m_csPlayer);

	std::map<DWORD, GameObject*>::iterator iter = m_xPlayers.find(_dwID);
	if(iter != m_xPlayers.end())
	{
		EraseTarget(iter->second);

		pObj = iter->second;
		g_xThreadBuffer.Reset();

		PkgNewPlayerNot not;
		not.bNew = false;
		not.uHandlerID = pObj->GetID();
		g_xThreadBuffer << not;
		BroadcastPacket(&g_xThreadBuffer, not.uHandlerID);

		//	删除所有的静态魔法
		RemoveStaticMagicByFire(pObj);

		//bool bRemoveOk = RemoveMappedObject(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY, pObj);
		bool bRemoveOk = RemoveMappedObject(pObj->GetValidPositionX(), pObj->GetValidPositionY(), pObj);
		//	这里需要注意  RemovePlayer这个函数 在玩家下线 或者切换地图时候调用 下线的时候 需要判断是否移除了地图映射数据 而切换地图 在切换地图的函数中已经移除 故不必判断
		if(_bDelete)
		{
			if(!bRemoveOk)
			{
				char szBuf[20];
				ObjectValid::GetItemName(&pObj->GetUserData()->stAttrib, szBuf);
				LOG(ERROR) << "Delete object [" << pObj->GetID() << "] __ Remove mapped data failed.Details: Name:"
					<< szBuf << " Valid positionX:" << pObj->GetValidPositionX() << " Valid positionY:" << pObj->GetValidPositionY()
					<< "..The following information is the cell data of the coordinate";

				MapCellInfo* pCellInfo = GetMapData(pObj->GetValidPositionX(), pObj->GetValidPositionY());
				if(NULL == pCellInfo)
				{
					LOG(ERROR) << "The cell data is null...";
				}
				else
				{
					if(!pCellInfo->pCellObjects)
					{
						LOG(ERROR) << "The cell data object list is null...";
					}
					else
					{
						LOG(ERROR) << "The data doesn't exist...";
					}
				}
			}

			HeroObject* pHero = static_cast<HeroObject*>(pObj);
			pHero->KillAllSlave();

			if(_bDelete)
			{
				//	死亡状态 扣经验
				if(pHero->GetUserData()->eGameState == OS_DEAD &&
					GetMapPkType() != kMapPkType_All)
				{
					pHero->SetObject_Expr(pHero->GetObject_Expr() * 0.9995f);
				}
				pHero->SendHumDataV2(false, true);
			}

			char szName[20];
			ObjectValid::GetItemName(&pHero->GetUserData()->stAttrib, szName);

			//////////////////////////////////////////////////////////////////////////
			//	Team
			if(pHero->GetTeamID() != 0)
			{
				GameTeam* pTeam = GameTeamManager::GetInstance()->GetTeam(pHero->GetTeamID());
				if(pTeam)
				{
					pTeam->Remove(pHero);
					std::string xMsg;
					xMsg = szName;
					xMsg += " 退出队伍";
					pTeam->SendTeamMessage(xMsg.c_str());

					if(pTeam->GetCount() == 0)
					{
						GameTeamManager::GetInstance()->RemoveTeam(pTeam->GetTeamID());
					}
				}
			}

			UserInfoList::iterator begIter = g_xUserInfoList.begin();
			UserInfoList::const_iterator endIter = g_xUserInfoList.end();

			for(begIter;
				begIter != endIter;
				++begIter)
			{
				if(begIter->uHandlerID == pHero->GetID())
				{
					g_xUserInfoList.erase(begIter);
					break;
				}
			}

			LOG(INFO) << "delete player[" << szName << "] id[" << pHero->GetID() << "] index[" << pHero->GetUserIndex() << "]";

			//	small quit?
			if(pHero->IsSmallQuit())
			{
				PkgPlayerQuitSelChrRsp rsp;
				rsp.uTargetId = pHero->GetID();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << rsp;
				pHero->SendPacket(rsp);
			}

			delete pObj;
		}
		
		m_xPlayers.erase(iter);
		bRet = true;
	}

	LeaveCriticalSection(&m_csPlayer);

	return bRet;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::InsertNPC(GameObject* _pNPC)
{
	RECORD_FUNCNAME_WORLD;

	bool bRet = false;

	PkgNewNPCNot stNpcNot;
	MonsterObject* pMonster = NULL;

	std::map<DWORD, GameObject*>::iterator iter = m_xNPCs.find(_pNPC->GetID());

	if(iter == m_xNPCs.end())
	{
		if(_pNPC->GetType() == SOT_MONSTER ||
			_pNPC->GetType() == SOT_NPC)
		{
			//	这里用的是Attrib里面的ID 脚本里面调用的话还是用Attrib比较方便
			_pNPC->SetMapID(m_dwMapID);
			m_xNPCs.insert(std::make_pair(_pNPC->GetID(), _pNPC));
			_pNPC->GetLocateScene()->AddMappedObject(_pNPC->GetUserData()->wCoordX,
				_pNPC->GetUserData()->wCoordY, CELL_MOVEOBJECT, _pNPC);

			stNpcNot.uHandlerID = _pNPC->GetID();
			stNpcNot.uMonsID = _pNPC->GetObject_ID();
			stNpcNot.uPosition = MAKELONG(_pNPC->GetUserData()->wCoordX, _pNPC->GetUserData()->wCoordY);
			stNpcNot.uParam = _pNPC->GetObject_HP();
			stNpcNot.uFlag = _pNPC->GetObject_MaxExpr();

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << stNpcNot;
			BroadcastPacket(&g_xThreadBuffer);
			bRet = true;

			if(_pNPC->GetType() == SOT_MONSTER)
			{
				pMonster = static_cast<MonsterObject*>(_pNPC);
				if(pMonster->GetMaster() != NULL)
				{
					PkgPlayerUpdateAttribNtf uantf;
					uantf.uTargetId = pMonster->GetID();

					uantf.bType = UPDATE_MASTER;
					uantf.dwParam = pMonster->GetMaster()->GetID();
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << uantf;
					BroadcastPacket(&g_xThreadBuffer);

					uantf.bType = UPDATE_SLAVELV;
					uantf.dwParam = pMonster->GetObject_MaxAC();
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << uantf;
					BroadcastPacket(&g_xThreadBuffer);
				}
			}
			else if(_pNPC->GetType() == SOT_NPC)
			{
				//	Init the merchant
				lua_getglobal(_pNPC->GetLocateScene()->GetLuaState(), "InitNPC");
				//tolua_pushusertype(_pNPC->GetLocateScene()->GetLuaState(), this, "GameScene");
				lua_pushinteger(_pNPC->GetLocateScene()->GetLuaState(), (int)_pNPC->GetObject_ID());
				tolua_pushusertype(_pNPC->GetLocateScene()->GetLuaState(), _pNPC, "NPCObject");
				int nRet = lua_pcall(GetLuaState(), 2, 0, 0);
				if(nRet != 0)
				{
#ifdef _DEBUG
					LOG(WARNING) << "Can't call InitNPC : " << lua_tostring(GetLuaState(), -1);
#endif
					lua_pop(GetLuaState(), 1);
				}
			}

			//	restore static magic
			RestoreStaticMagicByFire(_pNPC);
		}
	}
	return bRet;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::RemoveNPC(DWORD _dwID, bool _bDelete /* = true */)
{
	RECORD_FUNCNAME_WORLD;

	GameObject* pObj = NULL;
	bool bRet = false;

	std::map<DWORD, GameObject*>::iterator iter = m_xNPCs.find(_dwID);
	if(iter != m_xNPCs.end())
	{
		pObj = iter->second;
		//if(!pObj->GetLocateScene()->RemoveMappedObject(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY, pObj))
		if(!pObj->GetLocateScene()->RemoveMappedObject(pObj->GetValidPositionX(), pObj->GetValidPositionY(), pObj))
		{
			if(_bDelete)
			{
				char szName[20];
				ObjectValid::GetItemName(&pObj->GetUserData()->stAttrib, szName);
				LOG(ERROR) << "Name:" << szName << " Coordinate x:" << pObj->GetUserData()->wCoordX << " y:" << pObj->GetUserData()->wCoordY
					<< "Valid pos x:" << pObj->GetValidPositionX() << " pos y:" << pObj->GetValidPositionY();
			}
		}
		PkgDelNPCNot not;
		not.uTargetId = pObj->GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		BroadcastPacket(&g_xThreadBuffer);

		//	删除所有目标为obj的target
		EraseTarget(pObj);

		//	remove static magic
		RemoveStaticMagicByFire(pObj);

		if(_bDelete)
		{
			//LOG(INFO) << "D" << pObj->GetObject_ID() << "|" << (void*)pObj;
			delete pObj;
			pObj = NULL;
		}
		m_xNPCs.erase(iter);
		bRet = true;
	}

	return bRet;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::Update(DWORD _dwTick)
{
	RECORD_FUNCNAME_WORLD;
	ExecuteTimer xSceneTime;
	xSceneTime.Start();

	//	Prevent repeat map data update
	if(m_dwLoopTime == 0xFFFFFFFF)
	{
		m_dwLoopTime = 0;
	}
	++m_dwLoopTime;

	//	Update the object block information
	//if(_dwTick - m_dwLastUpdateObjectBlockTime > INTERVAL_OBJECTBLOCK)
	{
		//UpdateObjectBlockInfo();
		//m_dwLastUpdateObjectBlockTime = _dwTick;
	}

	//	update all objects
	int nPreUserCount = m_nCurPlayers;
	bool bPlayerSumChanged = false;
	m_nCurPlayers = 0;
	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		iter->second->DoWork(_dwTick);
		++m_nCurPlayers;
	}
	if(nPreUserCount != m_nCurPlayers)
	{
		bPlayerSumChanged = true;
	}

	m_nCurMonsters = 0;
	{
		for(std::map<DWORD, GameObject*>::const_iterator iter = m_xNPCs.begin();
			iter != m_xNPCs.end();
			++iter)
		{
			if(iter->second->GetType() == SOT_MONSTER &&
				iter->second->GetUserData()->eGameState != OS_DEAD)
			{
				MonsterObject* pMonster = static_cast<MonsterObject*>(iter->second);
				if(pMonster->GetMaster() == NULL)
				{
					++m_nCurMonsters;
				}
			}
			ExecuteTimer xMonsTimer;
			xMonsTimer.Start();

			iter->second->DoWork(_dwTick);

			if(GetMapID() == 30)
			{
				//LOG(ERROR) << "Scene[" << GetMapID() << "] cost too long time..." << "Monster sum:" << m_nCurMonsters;
				DWORD dwTime = xMonsTimer.GetExecuteTime();
				if(dwTime > 5)
				{
					int MonID = iter->second->GetObject_ID();
					MonID = MonID;
				}
			}
			else
			{
				DWORD dwTime = xMonsTimer.GetExecuteTime();
				if(dwTime > 5)
				{
					int MonID = iter->second->GetObject_ID();
					MonID = MonID;
				}
			}
		}
	}


	//if(IsInstance())
	if(IsTreasureMap())
	{
		if(m_nCurMonsters == 0)
		{
			if(!m_bGiveReward &&
				m_bKilledMonster)
			{
				m_bGiveReward = true;
				HeroObject* pHero = NULL;

				std::map<DWORD, GameObject*>::const_iterator begiter = m_xPlayers.begin();
				for(begiter;
					begiter != m_xPlayers.end();
					++begiter)
				{
					if(begiter->second->GetType() == SOT_HERO)
					{
						pHero = static_cast<HeroObject*>(begiter->second);
						pHero->SendSystemMessage("打开 藏宝盒 领取奖励");
					}
				}

				lua_getglobal(m_xScript.GetVM(), "GiveReward");
				tolua_pushusertype(m_xScript.GetVM(), this, "GameScene");
				int nRet = lua_pcall(m_xScript.GetVM(), 1, 0, 0);
				if(0 != nRet)
				{
					LOG(ERROR) << lua_tostring(m_xScript.GetVM(), -1);
					lua_pop(m_xScript.GetVM(), 1);
				}
			}
		}
		else if(!m_bGiveReward)
		{
			if(m_nCurPlayers != 0)
			{
				if(_dwTick - m_dwTreasureTipTime > 5000)
				{
					char szBuf[50];
					HeroObject* pHero = NULL;
					sprintf(szBuf, "还剩 %d 只怪物",
						m_nCurMonsters);
					std::map<DWORD, GameObject*>::const_iterator begiter = m_xPlayers.begin();
					for(begiter;
						begiter != m_xPlayers.end();
						++begiter)
					{
						if(begiter->second->GetType() == SOT_HERO)
						{
							pHero = static_cast<HeroObject*>(begiter->second);
							pHero->SendSystemMessage(szBuf);
						}
					}
					m_dwTreasureTipTime = _dwTick;
				}
			}
		}
	}

	//	Send the timer count buf
	FlushDelayBuf();

	//	Execute the script
	/*m_dwExecuteScriptInterval += _dwTick;
	if(m_dwExecuteScriptInterval > EXECUTE_SCRIPT_INTERVAL * 30 &&
		m_xScript.GetScript() != NULL)
	{
		m_dwExecuteScriptInterval = 0;
		lua_getglobal(m_xScript.GetScript(), "OnUpdateScene");
		//lua_pushlightuserdata(m_xScript.GetScript(), this);
		tolua_pushusertype(m_xScript.GetScript(), this, "GameScene");
		int nRet = 0;
		nRet = lua_pcall(m_xScript.GetScript(), 1, 1, 0);
		if(nRet != 0)
		{
			LOG(ERROR) << "Execute script error : " << m_xScript.GetMapName() << " : " << lua_tostring(m_xScript.GetScript(), -1);
			lua_pop(m_xScript.GetScript(), 1);
		}
		else
		{
			//	WRANNING! Don't forget to pop the return value (Memory leak!!!)
			lua_pop(m_xScript.GetScript(), 1);
		}
	}*/

	//	Insert players
	//	Note: Not in DoWork because iterator become invalid by using erase function
	/*if(!m_xWaitInsertPlayers.empty())
	{
		std::list<GameObject*>::const_iterator begiter = m_xWaitInsertPlayers.begin();
		std::list<GameObject*>::const_iterator enditer = m_xWaitInsertPlayers.end();

		for(begiter; begiter != enditer; ++begiter)
		{
			if((*begiter)->GetType() == SOT_HERO)
			{
				InsertPlayer(*begiter);
				GetSceneData((HeroObject*)*begiter);

				//	lua call
				m_xSceneEventExecutor.Call_ScenePlayerEnter(this, (HeroObject*)*begiter);
			}
			else if((*begiter)->GetType() == SOT_MONSTER)
			{
				InsertNPC(*begiter);
			}
		}

		m_xWaitInsertPlayers.clear();
	}
	if(!m_xWaitRemovePlayers.empty())
	{
		std::list<GameObject*>::const_iterator begiter = m_xWaitRemovePlayers.begin();
		std::list<GameObject*>::const_iterator enditer = m_xWaitRemovePlayers.end();

		for(begiter; begiter != enditer; ++begiter)
		{
			if((*begiter)->GetType() == SOT_HERO)
			{
				RemovePlayer((*begiter)->GetID(), false);
			}
			else if((*begiter)->GetType() == SOT_MONSTER)
			{
				RemoveNPC((*begiter)->GetID(), false);
			}
		}

		m_xWaitRemovePlayers.clear();
	}
	if(!m_xWaitDeletePlayers.empty())
	{
		std::list<GameObject*>::const_iterator begiter = m_xWaitDeletePlayers.begin();
		std::list<GameObject*>::const_iterator enditer = m_xWaitDeletePlayers.end();

		for(begiter; begiter != enditer; ++begiter)
		{
			if((*begiter)->GetType() == SOT_HERO)
			{
				RemovePlayer((*begiter)->GetID(), true);
			}
			else if((*begiter)->GetType() == SOT_MONSTER)
			{
				RemoveNPC((*begiter)->GetID(), true);
			}
		}

		m_xWaitDeletePlayers.clear();
	}*/
	ProcessWaitInsert();
	ProcessWaitRemove();
	ProcessWaitDelete();

	//	Generate monster
	if(_dwTick - m_dwLastGenerateMonsterTime > 10000)
	{
		AutoGenerateMonster();
		m_dwLastGenerateMonsterTime = _dwTick;
	}

	//	static magic
	if(_dwTick - m_dwLastUpdateMagicTime > 1000)
	{
		HandleStaticMagic();
		m_dwLastUpdateMagicTime = _dwTick;
	}

	//	clean item
	if(_dwTick - m_dwLastCleanItemTime > 10 * 1000)
	{
		CheckGroundItems();
		UpdateDoorEvent();
		m_dwLastCleanItemTime = _dwTick;
	}

	//	scene event callback
	m_xSceneEventExecutor.Call_SceneUpdate(this);

	if(xSceneTime.GetExecuteTime() > 5000)
	{
		if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
		{
			LOG(ERROR) << "DESTORY_GAMESCENE";
		}
		else
		{
			DESTORY_GAMESCENE;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::ProcessWaitInsert()
{
	if(!m_xWaitInsertPlayers.empty())
	{
		std::list<GameObject*>::const_iterator begiter = m_xWaitInsertPlayers.begin();
		std::list<GameObject*>::const_iterator enditer = m_xWaitInsertPlayers.end();

		for(begiter; begiter != enditer; ++begiter)
		{
			if((*begiter)->GetType() == SOT_HERO)
			{
				HeroObject* pHero = (HeroObject*)(*begiter);
				InsertPlayer(*begiter);
				GetSceneData((HeroObject*)*begiter);

				//	trans animation?
				if(pHero->IsNeedTransAni())
				{
					PkgPlayerPlayAniAck pppaa;
					pppaa.uTargetId = pHero->GetID();
					pppaa.wAniID = 1007;
					pppaa.xPos.push_back(MAKELONG(pHero->GetUserData()->wCoordX, pHero->GetUserData()->wCoordY));
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << pppaa;
					BroadcastPacket(&g_xThreadBuffer);
					pHero->SetNeedTransAni(false);
				}

				//	lua call
				m_xSceneEventExecutor.Call_ScenePlayerEnter(this, (HeroObject*)*begiter);
				LuaEvent_ScenePlayerEnter evt;
				evt.pHero = (HeroObject*)*begiter;
				evt.pScene = this;
				m_xScript.DispatchEvent(kLuaEvent_ScenePlayerEnter, &evt);

				//	is a pk map?
				if(GetMapPkType() == kMapPkType_All)
				{
					evt.pHero->SendSystemMessage("当前地图为PVP地图，请注意安全");
				}
			}
			else if((*begiter)->GetType() == SOT_MONSTER)
			{
				InsertNPC(*begiter);
			}
		}

		m_xWaitInsertPlayers.clear();
	}
}

void GameScene::ProcessWaitRemove()
{
	if(!m_xWaitRemovePlayers.empty())
	{
		std::list<GameObject*>::const_iterator begiter = m_xWaitRemovePlayers.begin();
		std::list<GameObject*>::const_iterator enditer = m_xWaitRemovePlayers.end();

		for(begiter; begiter != enditer; ++begiter)
		{
			if((*begiter)->GetType() == SOT_HERO)
			{
				RemovePlayer((*begiter)->GetID(), false);
			}
			else if((*begiter)->GetType() == SOT_MONSTER)
			{
				RemoveNPC((*begiter)->GetID(), false);
			}
		}

		m_xWaitRemovePlayers.clear();
	}
}

void GameScene::ProcessWaitDelete()
{
	if(!m_xWaitDeletePlayers.empty())
	{
		std::list<GameObject*>::const_iterator begiter = m_xWaitDeletePlayers.begin();
		std::list<GameObject*>::const_iterator enditer = m_xWaitDeletePlayers.end();

		for(begiter; begiter != enditer; ++begiter)
		{
			if((*begiter)->GetType() == SOT_HERO)
			{
				RemovePlayer((*begiter)->GetID(), true);
			}
			else if((*begiter)->GetType() == SOT_MONSTER)
			{
				RemoveNPC((*begiter)->GetID(), true);
			}
		}

		m_xWaitDeletePlayers.clear();
	}
}
//////////////////////////////////////////////////////////////////////////
DWORD GameScene::BroadcastPacket(ByteBuffer* _pBuf, DWORD _dwIgnoreID /* = 0 */)
{
	RECORD_FUNCNAME_WORLD;

	HeroObject* pObj = NULL;
	DWORD dwCounter = 0;

	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		pObj = (HeroObject*)iter->second;
		if(pObj)
		{
			if(pObj->GetID() != _dwIgnoreID)
			{
				//SendBuffer(pObj->GetUserIndex(), _pBuf);
				pObj->SendPlayerBuffer(*_pBuf);
				++dwCounter;
			}
		}
	}
	return dwCounter;
}
//////////////////////////////////////////////////////////////////////////
DWORD GameScene::BroadcastPacketRange(ByteBuffer* _pBuf, RECT& rcRange, DWORD _dwIgnoreID /* = 0 */)
{
	HeroObject* pObj = NULL;
	DWORD dwCounter = 0;

	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		pObj = (HeroObject*)iter->second;
		if(pObj)
		{
			if(pObj->GetID() != _dwIgnoreID)
			{
				//SendBuffer(pObj->GetUserIndex(), _pBuf);
				DWORD dwCoordX = pObj->GetCoordX();
				DWORD dwCoordY = pObj->GetCoordY();

				if(dwCoordX >= rcRange.left &&
					dwCoordX <= rcRange.right &&
					dwCoordY >= rcRange.top &&
					dwCoordY <= rcRange.bottom)
				{
					pObj->SendPlayerBuffer(*_pBuf);
					++dwCounter;
				}
			}
		}
	}
	return dwCounter;
}
//////////////////////////////////////////////////////////////////////////
//	Old version
void GameScene::PushDelayBuf(DelaySendInfo& _inf)
{
	RECORD_FUNCNAME_WORLD;

	try
	{
		m_xDelaySendBuf << _inf;
	}
	BUFFER_EXCEPTION_CATCH_ASSERT("Delay buffer overflow![PushDelayBuf]" == NULL);
}
//////////////////////////////////////////////////////////////////////////
//	New version
void GameScene::PushDelayBuf(SceneDelayMsg* _pMsg)
{
	RECORD_FUNCNAME_WORLD;

	m_xSceneDelayMsgList.push_back(_pMsg);
}
//////////////////////////////////////////////////////////////////////////
//	Old version
void GameScene::FlushDelayBuf()
{
	RECORD_FUNCNAME_WORLD;
	//////////////////////////////////////////////////////////////////////////
	//	New version
	DWORD dwCurrentTime = GetTickCount();
	SceneDelayMsg* pMsg = NULL;

	if(!m_xSceneDelayMsgList.empty())
	{
		SceneDelayMsgList::iterator begIter = m_xSceneDelayMsgList.begin();
		SceneDelayMsgList::iterator endIter = m_xSceneDelayMsgList.end();

		for(begIter;
			begIter != endIter;
			)
		{
			pMsg = *begIter;

			if(pMsg->dwDelayTime < dwCurrentTime)
			{
				//	Send it
				ParseDelayInfo(pMsg);

				//	Remove it
				FreeListManager::GetInstance()->PushFreeSceneDelayMsg(pMsg);
				begIter = m_xSceneDelayMsgList.erase(begIter);
			}
			else
			{
				//	Ignore it
				++begIter;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//	Old version
// 	DWORD dwCurrentTime = GetTickCount();
// 
// 	DelaySendInfo info;
// 	while(m_xDelaySendBuf.GetLength())
// 	{
// 		m_xDelaySendBuf >> info;
// 		if(info.uDelayTime < dwCurrentTime)
// 		{
// 			//	Send it
// 			ParseDelayInfo(info);
// 		}
// 		else
// 		{
// 			//	Save it
// 			m_xWaitDelay << info;
// 		}
// 	}
// 	m_xDelaySendBuf.Reset();
// 	if(m_xWaitDelay.GetLength())
// 	{
// 		m_xDelaySendBuf.Write(m_xWaitDelay.GetHead(), m_xWaitDelay.GetLength());
// 		m_xWaitDelay.Reset();
// 	}
}
//////////////////////////////////////////////////////////////////////////
//	New version
void GameScene::ParseDelayInfo(SceneDelayMsg* _pMsg)
{
	RECORD_FUNCNAME_WORLD;

	SceneDelayMsg& _inf = *_pMsg;

	switch(_pMsg->uOp)
	{
	case PKG_GAME_OBJECT_ACTION_NOT:
		{
			PkgObjectActionNot not;
			not.uTargetId = _inf.uParam[0];
			not.uParam0 = _inf.uParam[0];
			not.uParam1 = _inf.uParam[1];
			not.uParam2 = _inf.uParam[2];

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			BroadcastPacket(&g_xThreadBuffer);
		}break;
	case DELAY_HEROATTACK:
		{
			LOG(ERROR) << "DELAY_HEROATTACK!!!";
			MonsterObject* pMonster = (MonsterObject*)GetNPCByHandleID(_inf.uParam[0]);
			g_xThreadBuffer.Reset();
			AttackMsg* pMsg = FreeListManager::GetInstance()->GetFreeAttackMsg();
			pMsg->wDamage = (WORD)_inf.uParam[1];
			pMsg->dwInfo = _inf.uParam[4];
			pMsg->dwAttacker = _inf.uParam[2];
			pMsg->bType = _inf.uParam[3];
			pMsg->dwMasks = _inf.uParam[5];
			if(pMonster)
			{
				pMonster->AddAttackProcess(pMsg);
			}
		}break;
	case DELAY_SLAVEATTACK:
		{

		}break;
	case DELAY_BIGPOISONSKL:
		{
			/*StaticMagic sm;
			sm.dwFire = _pMsg->uParam[3];
			sm.sPosX = LOWORD(_pMsg->uParam[0]);
			sm.sPoxY = HIWORD(_pMsg->uParam[0]);
			sm.wMaxDC = LOWORD(_pMsg->uParam[1]);
			sm.wMinDC = HIWORD(_pMsg->uParam[1]);
			sm.dwEnableTime = 0;
			sm.dwEffectActive = 2;
			sm.dwExpire = GetTickCount() + _pMsg->uParam[2];
			sm.wMgcID = MEFF_BIGPOISON;
			sm.wMgcLevel = _pMsg->uParam[4];
			PushStaticMagic(&sm);*/
		}break;
	case DELAY_DRAGONBLUSTER:
		{
			/*StaticMagic sm;
			sm.dwFire = _pMsg->uParam[3];
			sm.sPosX = LOWORD(_pMsg->uParam[0]);
			sm.sPoxY = HIWORD(_pMsg->uParam[0]);
			sm.wMaxDC = LOWORD(_pMsg->uParam[1]);
			sm.wMinDC = HIWORD(_pMsg->uParam[1]);
			sm.dwEnableTime = 0;
			sm.dwEffectActive = 2;
			sm.dwExpire = GetTickCount() + _pMsg->uParam[2];
			sm.wMgcID = MEFF_DRAGONBLUSTER;
			sm.wMgcLevel = _pMsg->uParam[4];
			PushStaticMagic(&sm);*/
		}break;
	case DELAY_HEROSTRUCK:
		{
			/*HeroObject* pObj = (HeroObject*)GetPlayerWithoutLock(_inf.uParam[0]);
			if(pObj)
			{
				PkgPlayerStruckAck ack;
				ack.uTargetId = _inf.uParam[0];
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ack;
				pObj->SendPlayerBuffer(g_xThreadBuffer);
			}*/
		}break;
	case DELAY_HEAL:
		{
			/*GameObject* pObj = NULL;
			if(_inf.uParam[2] == 0)
			{
				pObj = GetPlayerWithoutLock(_inf.uParam[0]);
			}
			else if(_inf.uParam[2] == 1)
			{
				pObj = GetNPCByHandleID(_inf.uParam[0]);
			}
			//HeroObject* pObj = (HeroObject*)GetPlayerWithoutLock(_inf.uParam[0]);

			if(pObj)
			{
				//pObj->AddDrugState(_inf.uParam[1], 0);
				pObj->AddHealState(_inf.uParam[1]);
			}*/
		}break;
	case DELAY_MAKESLAVE:
		{
			/*HeroObject* pObj = (HeroObject*)GetPlayerWithoutLock(_inf.uParam[0]);
			if(pObj)
			{
				int nSlaveMax = 1;
				const UserMagic* pUsrMgc = NULL;
				int nSlaveID = _inf.uParam[1];

				switch(_inf.uParam[1])
				{
				case 29://	变异骷髅
					{
						pUsrMgc = pObj->GetUserMagic(MEFF_KULOU);
					}break;
				case 14://	神兽
					{
						pUsrMgc = pObj->GetUserMagic(MEFF_SUMMON);
					}break;
				case 62://	上古神兽
					{
						pUsrMgc = pObj->GetUserMagic(MEFF_SUPERSUMMON);
					}break;
				case 99://	白虎
					{
						pUsrMgc = pObj->GetUserMagic(MEFF_SUMMONTIGER);
					}break;
				case 144:
					{
						pUsrMgc = pObj->GetUserMagic(MEFF_SUMMONBOWMAN);
					}break;
				}

				if(pUsrMgc)
				{
					if(nSlaveID == 29 ||
						nSlaveID == 14 ||
						nSlaveID == 62)
					{
						if(pUsrMgc->bLevel >= 3)
						{
							nSlaveMax = 2;
						}
						else if(pUsrMgc->bLevel == 0)
						{
							nSlaveMax = 0;
						}

						if(pObj->GetSlaveCount() < nSlaveMax)
						{
							pObj->MakeSlave(_inf.uParam[1]);
						}
					}
					else if(nSlaveID == 99)
					{
						if(pUsrMgc->bLevel == 3 ||
							pUsrMgc->bLevel == 6)
						{
							nSlaveMax = 2;
						}

						if(pObj->GetSlaveCount() < nSlaveMax)
						{
							pObj->MakeSlave(_inf.uParam[1]);
						}
					}
					else if(nSlaveID == 144)
					{
						if(pUsrMgc->bLevel == 3 ||
							pUsrMgc->bLevel == 6)
						{
							nSlaveMax = 2;
						}

						if(pObj->GetSlaveCount() < nSlaveMax)
						{
							pObj->MakeSlave(_inf.uParam[1]);
						}
					}
				}
			}*/
		}break;
	default:
		{
			LOG(ERROR) << "Unsolved delay info[" << _inf.uOp << "]";
		}break;
	}
}
//////////////////////////////////////////////////////////////////////////
//	Old version
void GameScene::ParseDelayInfo(DelaySendInfo& _inf)
{
	RECORD_FUNCNAME_WORLD;

	switch(_inf.uOp)
	{
	case PKG_GAME_OBJECT_ACTION_NOT:
		{
			PkgObjectActionNot not;
			not.uTargetId = _inf.uParam[0];
			not.uParam0 = _inf.uParam[0];
			not.uParam1 = _inf.uParam[1];
			not.uParam2 = _inf.uParam[2];

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			BroadcastPacket(&g_xThreadBuffer);
		}break;
	case DELAY_HEROATTACK:
		{
			MonsterObject* pMonster = (MonsterObject*)GetNPCByHandleID(_inf.uParam[0]);
			g_xThreadBuffer.Reset();
			AttackInfo info;
			info.wDamage = (WORD)_inf.uParam[1];
			info.dwInfo = _inf.uParam[4];
			info.dwAttacker = _inf.uParam[2];
			info.bType = _inf.uParam[3];
			g_xThreadBuffer << info.wDamage << info.dwInfo << info.dwAttacker << info.bType;
			if(pMonster)
			{
				pMonster->AddAttackProcess(g_xThreadBuffer);
			}
		}break;
	case DELAY_SLAVEATTACK:
		{

		}break;
	case DELAY_HEROSTRUCK:
		{
			HeroObject* pObj = (HeroObject*)GetPlayerWithoutLock(_inf.uParam[0]);
			if(pObj)
			{
				PkgPlayerStruckAck ack;
				ack.uTargetId = _inf.uParam[0];
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ack;
				pObj->SendPlayerBuffer(g_xThreadBuffer);
			}
		}break;
	case DELAY_HEAL:
		{
			GameObject* pObj = NULL;
			if(_inf.uParam[2] == 0)
			{
				pObj = GetPlayerWithoutLock(_inf.uParam[0]);
			}
			else if(_inf.uParam[2] == 1)
			{
				pObj = GetNPCByHandleID(_inf.uParam[0]);
			}
			//HeroObject* pObj = (HeroObject*)GetPlayerWithoutLock(_inf.uParam[0]);

			if(pObj)
			{
				//pObj->AddDrugState(_inf.uParam[1], 0);
				pObj->AddHealState(_inf.uParam[1]);
			}
		}break;
	case DELAY_MAKESLAVE:
		{
			HeroObject* pObj = (HeroObject*)GetPlayerWithoutLock(_inf.uParam[0]);
			if(pObj)
			{
				int nMaxSlave = 0;
				const UserMagic* pMgc = NULL;
				if(_inf.uParam[1] == MONSTER_KULOU ||
					_inf.uParam[1] == MONSTER_SHENSHOU)
				{
					if(_inf.uParam[1] == MONSTER_SHENSHOU)
					{
						pMgc = pObj->GetUserMagic(MEFF_SUMMON);
					}
					else if(_inf.uParam[1] == MONSTER_KULOU)
					{
						pMgc = pObj->GetUserMagic(MEFF_KULOU);
					}
					if(pMgc)
					{
						nMaxSlave = pMgc->bLevel;

						if(_inf.uParam[1] == MONSTER_SHENSHOU)
						{
							nMaxSlave = 1;
						}
						else if(_inf.uParam[1] == MONSTER_KULOU)
						{
							if(pMgc->bLevel == 1 ||
								pMgc->bLevel == 2)
							{
								nMaxSlave = 1;
							}
							else if(pMgc->bLevel == 3)
							{
								nMaxSlave = 2;
							}
							else
							{
								nMaxSlave = 0;
							}
						}
					}

					if(pObj->GetSlaveCount() < nMaxSlave &&
						pMgc)
					{
						pObj->MakeSlave(_inf.uParam[1]);
					}
				}
				else if(_inf.uParam[1] == 62)
				{
					pMgc = pObj->GetUserMagic(MEFF_SUPERSUMMON);

					if(pObj->GetSlaveCount() == 0 &&
						pMgc)
					{
						pObj->MakeSlave(_inf.uParam[1]);
					}
				}
			}
		}break;
	default:
		{
			LOG(ERROR) << "Unsolved delay info[" << _inf.uOp << "]";
		}break;
	}
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::InsertItem(GroundItem* _pItem, bool _bCopy /* = true */)
{
	RECORD_FUNCNAME_WORLD;

	GroundItem* pNewItem = NULL;
	if(_bCopy)
	{
		pNewItem = new GroundItem;
		memcpy(pNewItem, _pItem, sizeof(GroundItem));
	}
	else
	{
		pNewItem = _pItem;
	}
	//if(m_xItems.find(pNewItem->wID) == m_xItems.end())
	if(m_xItems.find(pNewItem->stAttrib.tag) == m_xItems.end())
	{
		m_xItems.insert(std::make_pair(pNewItem->stAttrib.tag, pNewItem));
		//m_xItems.insert(std::make_pair(pNewItem->wID, pNewItem));
		//	Then broadcast it
		ItemAttrib* pDropedItem = &_pItem->stAttrib;
		/*
		PkgPlayerDropItemNot not;
				not.uTargetId = GetID();
				not.dwID = pDropedItem->id;
				not.dwTag = pDropedItem->tag;
				not.wCoordX = m_stData.wCoordX;
				not.wCoordY = m_stData.wCoordY;*/
		PkgSystemNewItemNot not;
		not.dwID = GETITEMATB(&_pItem->stAttrib, ID);
		not.dwTag = _pItem->stAttrib.tag;
		not.dwPos = MAKELONG(_pItem->wPosX, _pItem->wPosY);
		not.uUserId = GETITEMATB(&_pItem->stAttrib, Level);
		not.dwItemHideAttrib = GETITEMATB(&_pItem->stAttrib, MaxMP);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		BroadcastPacket(&g_xThreadBuffer);

		pNewItem->wID = GetTickCount();

		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::RemoveItem(DWORD _dwItemUniqueID)
{
	RECORD_FUNCNAME_WORLD;

	std::map<DWORD, GroundItem*>::iterator iter = m_xItems.find(_dwItemUniqueID);
	if(iter != m_xItems.end())
	{
		delete iter->second;
		m_xItems.erase(iter);
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
GroundItem* GameScene::GetItem(DWORD _dwTag)
{
	RECORD_FUNCNAME_WORLD;

	std::map<DWORD, GroundItem*>::iterator iter = m_xItems.find(_dwTag);
	if(iter != m_xItems.end())
	{
		return iter->second;
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CheckGroundItems()
{
	RECORD_FUNCNAME_WORLD;

	DWORD dwCurTick = GetTickCount();
	//PkgPlayerClearItemNtf ntf;
	PkgSystemClearGroundItemNtf ntf;

	std::map<DWORD, GroundItem*>::iterator begiter = m_xItems.begin();
	for(begiter;
		begiter != m_xItems.end();
		)
	{
		if(dwCurTick - begiter->second->wID > 180 * 1000)
		{
			ntf.dwTag = begiter->second->stAttrib.tag;
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ntf;
			BroadcastPacket(&g_xThreadBuffer);

			delete begiter->second;
			begiter = m_xItems.erase(begiter);
		}
		else
		{
			++begiter;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::IsUserNameExist(const char* _pszName)
{
	char szName[20];

	std::map<DWORD, GameObject*>::const_iterator begiter = m_xPlayers.begin();
	for(begiter;
		begiter != m_xPlayers.end();
		++begiter)
	{
		ObjectValid::GetItemName(&begiter->second->GetUserData()->stAttrib, szName);
		if(0 == strcmp(_pszName, szName))
		{
			return true;
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::GetRandomPosition(DWORD* _pOut)
{
	RECORD_FUNCNAME_WORLD;

	//if(m_pMapData == NULL)
	if(m_pCellData == NULL)
	{
		LOG(WARNING) << "This scene haven't loaded yet";
		return false;
	}

	WORD wPosX = 0;
	WORD wPosY = 0;

	int nCounter = 0;
	wPosX = rand() % m_stMapInfo.nCol;
	wPosY = rand() % m_stMapInfo.nRow;

	while(m_pCellData[wPosY * m_stMapInfo.nCol + wPosX].bFlag != 0)
	{
		wPosX = rand() % m_stMapInfo.nCol;
		wPosY = rand() % m_stMapInfo.nRow;
		++nCounter;
		if(nCounter > 100)
		{
			return false;
		}
	}

	*_pOut = MAKELONG(wPosX, wPosY);
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::GetDropPosition(WORD _wX, WORD _wY, DWORD* _pOut)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		LOG(WARNING) << "This scene haven't loaded yet";
		return false;
	}

	int nPosX = _wX;
	int nPosY = _wY;
	MapCellInfo* pCell = NULL;
	bool bFindItem = false;

	for(int i = 0; i < sizeof(g_nSearchPoint) / 2; ++i)
	{
		bFindItem = false;
		nPosX = _wX;
		nPosY = _wY;
		nPosX += g_nSearchPoint[i * 2];
		nPosY += g_nSearchPoint[i * 2 + 1];
		if(nPosX >= 0 &&
			nPosX < m_stMapInfo.nCol &&
			nPosY >= 0 &&
			nPosY <= m_stMapInfo.nRow)
		{
			pCell = GetMapData(nPosX, nPosY);
			if(pCell->pCellObjects == NULL)
			{
				*_pOut = MAKELONG(nPosX, nPosY);
				return true;
			}
			else
			{
				CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
				CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();

				for(begiter; begiter != enditer; ++begiter)
				{
					if((*begiter)->bType == CELL_ITEMS)
					{
						bFindItem = true;
						break;
					}
				}
				if(!bFindItem)
				{
					*_pOut = MAKELONG(nPosX, nPosY);
					return true;
				}
			}
		}
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
MapCellInfo* GameScene::GetMapData(int _x, int _y)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		return 0;
	}

	if(_x >= m_stMapInfo.nCol ||
		_x < 0 ||
		_y >= m_stMapInfo.nRow ||
		_y < 0)
	{
		return 0;
	}

	return &m_pCellData[_y * m_stMapInfo.nCol + _x];
}
//////////////////////////////////////////////////////////////////////////
DoorEvent* GameScene::GetDoorEvent(int _nX, int _nY)
{
	MapCellInfo* pCellInfo = GetMapData(_nX, _nY);
	if(NULL == pCellInfo)
	{
		return NULL;
	}

	if(!pCellInfo->pCellObjects)
	{
		return NULL;
	}

	if(pCellInfo)
	{
		DoorEvent* pDoorEvt = NULL;

		CELLDATALIST::const_iterator begiter = pCellInfo->pCellObjects->begin();
		CELLDATALIST::const_iterator enditer = pCellInfo->pCellObjects->end();

		for(begiter; begiter != enditer; ++begiter)
		{
			if((*begiter)->bType == CELL_DOOR)
			{
				pDoorEvt = (DoorEvent*)(*begiter)->pData;
				return pDoorEvt;
				break;
			}
		}
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::UpdateObjectBlockInfo()
{
	//if(m_dwLoopTime != m_dwLastUpdateObjectBlockTime)
	/*
	if(m_pMapData == NULL)
		{
			return;
		}
	
		//	Reset object block flag
		for(int i = 0; i < m_stMapInfo.nRow; ++i)
		{
			for(int j = 0; j < m_stMapInfo.nCol; ++j)
			{
				RESET_FLAG(m_pMapData[i * m_stMapInfo.nCol + j], OBJECT_MASK);
			}
		}
	
		DWORD wCoordX = 0;
		DWORD wCoordY = 0;
		for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
			iter != m_xPlayers.end();
			++iter)
		{
			wCoordX = iter->second->GetUserData()->wCoordX;
			wCoordY = iter->second->GetUserData()->wCoordY;
			if(wCoordX < m_stMapInfo.nCol &&
				wCoordY < m_stMapInfo.nRow)
			{
				SET_FLAG(m_pMapData[wCoordY * m_stMapInfo.nCol + wCoordX], OBJECT_MASK);
			}
		}
		for(std::map<DWORD, GameObject*>::const_iterator iter = m_xNPCs.begin();
			iter != m_xNPCs.end();
			++iter)
		{
			wCoordX = iter->second->GetUserData()->wCoordX;
			wCoordY = iter->second->GetUserData()->wCoordY;
			if(wCoordX < m_stMapInfo.nCol &&
				wCoordY < m_stMapInfo.nRow)
			{
				SET_FLAG(m_pMapData[wCoordY * m_stMapInfo.nCol + wCoordX], OBJECT_MASK);
			}
		}*/
	
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::CanMove(int _x, int _y)
{
	RECORD_FUNCNAME_WORLD;

	//	First check block
	//if(m_pMapData == NULL)
	if(m_pCellData == NULL)
	{
		return false;
	}

	//DWORD dwData = GetMapData(_x, _y);
	MapCellInfo* pCell = GetMapData(_x, _y);
	if(pCell == NULL)
	{
		return false;
	}
	if(pCell->bFlag & BLOCK_MASK)
	{
		return false;
	}
	/*
	if(dwData & OBJECT_MASK)
		{
			return false;
		}*/
	if(pCell->pCellObjects != NULL)
	{
		CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
		CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();

		for(begiter; begiter != enditer; ++begiter)
		{
			if((*begiter)->bType == CELL_MOVEOBJECT)
			{
				GameObject* pObj = (GameObject*)((*begiter)->pData);
				if(pObj->GetUserData()->eGameState != OS_DEAD &&
					pObj->GetUserData()->eGameState != OS_GROUND)
				{
					return false;
				}
			}
		}
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::GetAroundCanMove(int _nX, int _nY, int* _pX, int* _pY)
{
	if(NULL == _pX ||
		NULL == _pY)
	{
		return false;
	}

	int x = 0;
	int y = 0;

	for(int i = 0; i < 8; ++i)
	{
		x = g_nMoveOft[i * 2] + _nX;
		y = g_nMoveOft[i * 2 + 1] + _nY;

		if(CanThrough(x, y))
		{
			*_pX = x;
			*_pY = y;
			return true;
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::CanThrough(int _x, int _y)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		return false;
	}

	//DWORD dwData = GetMapData(_x, _y);
	MapCellInfo* pCell = GetMapData(_x, _y);
	if(pCell == NULL)
	{
		return false;
	}
	if(pCell->bFlag & BLOCK_MASK)
	{
		return false;
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::CanDrop(int _x, int _y)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		return false;
	}

	//DWORD dwData = GetMapData(_x, _y);
	MapCellInfo* pCell = GetMapData(_x, _y);
	if(pCell == NULL)
	{
		return false;
	}
	if(pCell->bFlag & BLOCK_MASK)
	{
		return false;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::CanCross(int _sx, int _sy, int _dx, int _dy)
{
	RECORD_FUNCNAME_WORLD;

	float fEachX = 0;
	float fEachY = 0;
	int nTestX = 0;
	int nTestY = 0;

	int nOftX = _dx - _sx;
	int nOftY = _dy - _sy;
	bool bUseXScale = true;

	if(nOftX == 0 &&
		nOftY == 0)
	{
		return true;
	}

	if(nOftX == 0)
	{
		//	use ofty as a scale
		fEachY = (float)nOftY / abs(nOftY);
		fEachX = 0;
		bUseXScale = false;
	}
	else if(nOftY == 0)
	{
		//	use oftx as a scale
		fEachX = (float)nOftX / abs(nOftX);
		fEachY = 0;
	}
	else
	{
		//	use oftx as a scale
		fEachX = (float)nOftX / abs(nOftX);
		fEachY = nOftY / abs(nOftX);
	}

	if(bUseXScale)
	{
		for(int i = 0; i < abs(nOftX); ++i)
		{
			nTestX = _sx + fEachX * i;
			nTestY = _sy + fEachY * i;

			if(!CanThrough(nTestX, nTestY))
			{
				return false;
			}
		}
	}
	else
	{
		for(int i = 0; i < abs(nOftY); ++i)
		{
			nTestX = _sx + fEachX * i;
			nTestY = _sy + fEachY * i;

			if(!CanThrough(nTestX, nTestY))
			{
				return false;
			}
		}
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::LoadScript()
{
	char szScriptFile[MAX_PATH];
	sprintf(szScriptFile, "server/scenes/scene%d",
		GetMapID());

	m_xScript.Destroy();
	m_xScript.Create();
	tolua_BackMirServer_open(m_xScript.GetVM());
	m_xScript.PushEngine();

	//	压入脚本引擎
	//tolua_pushusertype(m_xScript.GetVM(), &m_xScript, "LuaServerEngine");
	//lua_setglobal(m_xScript.GetVM(), "ENGINE");

	return m_xScript.DoModule(szScriptFile);
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::ReloadScript()
{
	return LoadScript();

	RECORD_FUNCNAME_WORLD;

	/*if(m_xScript.GetScript() == NULL)
	{
		return false;
	}*/
	/*m_xScript.Destroy();
	m_xScript.Create();

	char szBuf[MAX_PATH];
#ifdef _DEBUG
	sprintf(szBuf, "%s\\Help\\scene%d.lua",
		GetRootPath(), m_xScript.GetUserTag());

	tolua_BackMirServer_open(m_xScript.GetVM());

	if(m_xScript.LoadFile(szBuf))
	{
		return true;
	}
	return false;
#else
	char szFile[50];
#ifdef _LUAJIT_
	sprintf(szFile, "scene%d.bjt",
		m_xScript.GetUserTag());
#else
	sprintf(szFile, "scene%d.bbt",
		m_xScript.GetUserTag());
#endif
	sprintf(szBuf, "%s\\Help\\dog.idx",
		GetRootPath());

	if(m_xScript.ExecuteZip(szBuf, szFile))
	{
		return true;
	}
	return false;
#endif*/
}
//////////////////////////////////////////////////////////////////////////
void GameScene::BroadcastChatMessage(std::string& _xMsg, unsigned int _dwExtra)
{
	RECORD_FUNCNAME_WORLD;

	HeroObject* pObj = NULL;

	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		pObj = (HeroObject*)iter->second;
		if(pObj)
		{
			pObj->SendChatMessage(_xMsg, _dwExtra);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::BroadcastChatMessage(const char* _pszMsg, unsigned int _dwExtra)
{
	RECORD_FUNCNAME_WORLD;

	HeroObject* pObj = NULL;

	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		pObj = (HeroObject*)iter->second;
		if(pObj)
		{
			pObj->SendChatMessage(_pszMsg, _dwExtra);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::BroadcastSceneSystemMessage(const char* _pszMsg)
{
	HeroObject* pObj = NULL;

	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		pObj = (HeroObject*)iter->second;
		if(pObj)
		{
			pObj->SendSystemMessage(_pszMsg);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::BroadcastSystemNotify(const char* _pszMsg)
{
	HeroObject* pObj = NULL;

	PkgSystemNotifyNot not;
	not.dwColor = 0xffffff00;
	not.dwTimes = 1;
	not.xMsg = _pszMsg;

	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		pObj = (HeroObject*)iter->second;
		if(pObj)
		{
			pObj->SendPacket(not);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
int GameScene::GetPlayerSum()
{
	RECORD_FUNCNAME_WORLD;

	return (int)m_xPlayers.size();
}
//////////////////////////////////////////////////////////////////////////
int GameScene::GetSlaveSum()
{
	int nCounter = 0;

	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xNPCs.begin();
		iter != m_xNPCs.end();
		++iter)
	{
		if(iter->second->GetType() == SOT_MONSTER)
		{
			MonsterObject* pMonster = static_cast<MonsterObject*>(iter->second);
			if(pMonster->GetMaster() != NULL)
			{
				++nCounter;
			}
		}
	}
	return nCounter;
}
//////////////////////////////////////////////////////////////////////////
int GameScene::GetMonsterSum(unsigned int _uID)
{
	RECORD_FUNCNAME_WORLD;

	int nCounter = 0;
	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xNPCs.begin();
		iter != m_xNPCs.end();
		++iter)
	{
		GameObject* pObj = iter->second;
		if(pObj->GetObject_ID() == _uID &&
			pObj->GetType() == SOT_MONSTER)
		{
			++nCounter;
		}
	}
	return nCounter;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateNPC(unsigned int _uID, unsigned short _uX, unsigned short _uY)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		LOG(ERROR) << "No data have loaded!CreateNPC error!";
		return;
	}
	NPCObject* pNPC = new NPCObject;
	pNPC->SetID(GameWorld::GetInstance().GenerateObjectID());
	pNPC->GetUserData()->stAttrib.id = _uID;
	pNPC->GetUserData()->wCoordX = _uX;
	pNPC->GetUserData()->wCoordY = _uY;
	pNPC->GetUserData()->wMapID = (WORD)m_dwMapID;

#ifdef _SYNC_CREATE
	if(GetRecordInMonsterTable(_uID, &pNPC->GetUserData()->stAttrib))
	{
		pNPC->EncryptObject();
		InsertNPC(pNPC);

#ifdef _DEBUG
		LOG(INFO) << "生成NPC[" << pNPC->GetObject_ID() << "]成功";
#endif
	}
	else
	{
		delete pNPC;
	}
#else
	DBOperationParam* pParam = new DBOperationParam;
	pParam->dwOperation = DO_QUERY_MONSATTRIB;
	pParam->dwParam[0] = (DWORD)pNPC;
	pParam->dwParam[1] = pNPC->GetUserData()->stAttrib.id;
	DBThread::GetInstance()->AsynExecute(pParam);
#endif
#ifdef _DEBUG_NEW_
	LOG(INFO) << "new operate on CreateNPC:" << _uID;
#endif
}
//////////////////////////////////////////////////////////////////////////
void GameScene::InsertGroundItem(GroundItem* _pItem)
{
	RECORD_FUNCNAME_WORLD;

	_pItem->stAttrib.tag = GameWorld::GetInstance().GenerateItemTag();
	ObjectValid::EncryptAttrib(&_pItem->stAttrib);

	if(InsertItem(_pItem, false))
	{
#ifdef _DEBUG
		LOG(INFO) << "地图" << GetMapID() << "生成Item[" << GETITEMATB(&_pItem->stAttrib, ID) << "]成功";
#endif
	}
	else
	{
		SAFE_DELETE(_pItem);
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateGroundItem(unsigned _uID, unsigned short _uX, unsigned short _uY, int _nExtraProb)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		return;
	}

	GroundItem* pItem = new GroundItem;
	pItem->wPosX = _uX;
	pItem->wPosY = _uY;
	pItem->stAttrib.id = _uID;

#ifdef _SYNC_CREATE
	if(GetRecordInItemTable(_uID, &pItem->stAttrib))
	{
		pItem->stAttrib.tag = GameWorld::GetInstance().GenerateItemTag();
		//GameWorld::GetInstance().UpgradeItems(&pItem->stAttrib);
		GameWorld::GetInstance().UpgradeItems(&pItem->stAttrib, _nExtraProb);
		ObjectValid::EncryptAttrib(&pItem->stAttrib);

		if(InsertItem(pItem, false))
		{
#ifdef _DEBUG
			LOG(INFO) << "地图" << GetMapID() << "生成Item[" << GETITEMATB(&pItem->stAttrib, ID) << "]成功";
#endif
		}
		else
		{
			SAFE_DELETE(pItem);
		}
	}
	else
	{
		SAFE_DELETE(pItem);
	}
#else
	DBOperationParam* pParam = new DBOperationParam;
	pParam->dwOperation = DO_QUERY_ITEMATTRIB;
	//	Head of GroundItem, so can transform to ItemAttrib
	pParam->dwParam[0] = (DWORD)pItem;
	pParam->dwParam[1] = MAKELONG(0, _uID);
	pParam->dwParam[2] = MAKELONG(IE_ADDGROUNDITEM, m_dwMapID);
	DBThread::GetInstance()->AsynExecute(pParam);
#endif
#ifdef _DEBUG_NEW_
	LOG(INFO) << "new operate on CreateGroundItem:" << _uID;
#endif
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateGroundItem(unsigned _uID, unsigned short _uX, unsigned short _uY)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		return;
	}

	GroundItem* pItem = new GroundItem;
	pItem->wPosX = _uX;
	pItem->wPosY = _uY;
	pItem->stAttrib.id = _uID;

#ifdef _SYNC_CREATE
	if(GetRecordInItemTable(_uID, &pItem->stAttrib))
	{
		pItem->stAttrib.tag = GameWorld::GetInstance().GenerateItemTag();
		GameWorld::GetInstance().UpgradeItems(&pItem->stAttrib);
		ObjectValid::EncryptAttrib(&pItem->stAttrib);

		if(InsertItem(pItem, false))
		{
#ifdef _DEBUG
			LOG(INFO) << "地图" << GetMapID() << "生成Item[" << GETITEMATB(&pItem->stAttrib, ID) << "]成功";
#endif
		}
		else
		{
			SAFE_DELETE(pItem);
		}
	}
	else
	{
		SAFE_DELETE(pItem);
	}
#else
	DBOperationParam* pParam = new DBOperationParam;
	pParam->dwOperation = DO_QUERY_ITEMATTRIB;
	//	Head of GroundItem, so can transform to ItemAttrib
	pParam->dwParam[0] = (DWORD)pItem;
	pParam->dwParam[1] = MAKELONG(0, _uID);
	pParam->dwParam[2] = MAKELONG(IE_ADDGROUNDITEM, m_dwMapID);
	DBThread::GetInstance()->AsynExecute(pParam);
#endif
#ifdef _DEBUG_NEW_
	LOG(INFO) << "new operate on CreateGroundItem:" << _uID;
#endif
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::IsItemInGround(int _nAttribID, int _x, int _y)
{
	RECORD_FUNCNAME_WORLD;

	std::map<DWORD, GroundItem*>::const_iterator begiter = m_xItems.begin();
	GroundItem* pItem = NULL;

	for(begiter;
		begiter != m_xItems.end();
		++begiter)
	{
		pItem = begiter->second;
		if(pItem->wPosX == _x &&
			pItem->wPosY == _y &&
			GETITEMATB(&pItem->stAttrib, ID) == _nAttribID)
		{
			return true;
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::RemoveGroundItem(int _nAttribID, int _x, int _y)
{
	RECORD_FUNCNAME_WORLD;

	std::map<DWORD, GroundItem*>::const_iterator begiter = m_xItems.begin();
	GroundItem* pItem = NULL;
	bool bFind = false;

	for(begiter;
		begiter != m_xItems.end();
		++begiter)
	{
		pItem = begiter->second;
		if(pItem->wPosX == _x &&
			pItem->wPosY == _y &&
			GETITEMATB(&pItem->stAttrib, ID) == _nAttribID)
		{
			bFind = true;
			break;
		}
	}

	if(NULL != pItem &&
		bFind)
	{
		std::map<DWORD, GroundItem*>::iterator fnditer = m_xItems.find(pItem->stAttrib.tag);
		if(fnditer != m_xItems.end())
		{
			PkgSystemClearGroundItemNtf ntf;

			ntf.dwTag = pItem->stAttrib.tag;
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ntf;
			BroadcastPacket(&g_xThreadBuffer);

			delete pItem;
			m_xItems.erase(fnditer);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
NPCObject* GameScene::GetNPCByID(unsigned int _uID)
{
	RECORD_FUNCNAME_WORLD;

	if(_uID < 10000)
	{
		return NULL;
	}
	else
	{
		std::map<DWORD, GameObject*>::const_iterator iter = m_xNPCs.find((DWORD)_uID);
		if(iter != m_xNPCs.end())
		{
			return (NPCObject*)iter->second;
		}
	}
	return NULL;
}

GameObject* GameScene::GetNPCByHandleID(unsigned int _uID)
{
	RECORD_FUNCNAME_WORLD;

	std::map<DWORD, GameObject*>::const_iterator iter = m_xNPCs.find((DWORD)_uID);
	if(iter != m_xNPCs.end())
	{
		return iter->second;
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
MonsterObject* GameScene::NewMonsterByID(int _nAttribID)
{
	MonsterObject* pNewMonster = NULL;
	int _uID = _nAttribID;

	if(_uID == MONSTER_CHULONGSHEN ||
		_uID == 131)
	{
		pNewMonster = new GroundMonster;
	}
	else if(_uID == MONSTER_SHENSHOU)
	{
		pNewMonster = new ShenShouMonster;
	}
	else if(_uID == 62)
	{
		pNewMonster = new SuperShenShouMonster;
	}
	else if(_uID == 63 ||
		_uID == 130)
	{
		pNewMonster = new WoMaJiaoZhuMonster;
	}
	else if(_uID == MONSTER_WOMAJIAOZHU)
	{
		pNewMonster = new WoMaJiaoZhuMonster;
	}
	else if(_uID == 65)
	{
		pNewMonster = new WoMaJiaoZhuMonster;
	}
	else if(_uID == 66)
	{
		pNewMonster = new WoMaJiaoZhuMonster;
	}
	else if(_uID == 16 ||
		_uID == 129 ||
		_uID == 148)
	{
		pNewMonster = new WoMaJiaoZhuMonster;
	}
	else if(_uID == 79)
	{
		pNewMonster = new WoMaJiaoZhuMonster;
	}
	else if(_uID == 80)
	{
		pNewMonster = new WoMaJiaoZhuMonster;
	}
	else if(_uID == 45)
	{
		//	thunder corpse
		pNewMonster = new Self16DrtMonster;
	}
	else if(_uID == 72 ||
		_uID == 155)
	{
		//	minotaur wizard
		pNewMonster = new Self16DrtMonster;
	}
	else if(_uID == 47 ||
		_uID == 150)
	{
		pNewMonster = new Self16DrtMonster;
	}
	else if(_uID == 85)
	{
		pNewMonster = new Self16DrtMonster;
	}
	else if(_uID == 95)
	{
		pNewMonster = new Self16DrtMonster;
	}
	else if(_uID == 96)
	{
		pNewMonster = new SuHuMonster;
	}
	else if(_uID == 18)
	{
		//	Zuma
		pNewMonster = new ZuMaJiaoZhuMonster;
	}
	else if(_uID == 51)
	{
		//	red moon monster
		pNewMonster = new ChiYueEMoMonster;
	}
	else if(_uID == 74)
	{
		pNewMonster = new ChiYueEMoMonster;
	}
	else if(_uID == 92 ||
		_uID == 142)
	{
		pNewMonster = new ChiYueEMoMonster;
	}
	else if(_uID == 90 ||
		_uID == 91)
	{
		pNewMonster = new MoLongStoneMonster;
	}
	else if(_uID == 86 ||
		_uID == 141)
	{
		pNewMonster = new MoLongKingMonster;
	}
	else if(_uID == 52)
	{
		pNewMonster = new ShadowSpiderMonster;
	}
	else if(_uID == 53)
	{
		pNewMonster = new ExplodeSpiderMonster;
	}
	else if(_uID == 98)
	{
		pNewMonster = new FlyStatueMonster;
	}
	else if(_uID == 99)
	{
		pNewMonster = new BaiHuMonster;
	}
	else if(_uID == 100)
	{
		pNewMonster = new FlameSummonerMonster;
	}
	else if(_uID == 101)
	{
		pNewMonster = new FlameDCMonster;
	}
	else if(_uID == 102)
	{
		pNewMonster = new FlameMCMonster;
	}
	else if(_uID == 103)
	{
		pNewMonster = new IceSoldierMonster;
	}
	else if(_uID == 104)
	{
		pNewMonster = new IceGuardMonster;
	}
	else if(_uID == 105)
	{
		pNewMonster = new IceKingOfWarMonster;
	}
	else if(_uID == 106)
	{
		pNewMonster = new IceKingMonster;
	}
	else if(_uID == 111)
	{
		pNewMonster = new IceSavageMonster;
	}
	else if(_uID == 112)
	{
		pNewMonster = new IceDefenderMonster;
	}
	else if(_uID == 114)
	{
		pNewMonster = new YamaWatcherMonster;
	}
	else if(_uID == 118)
	{
		pNewMonster = new BlueGhasterMonster;
	}
	else if(_uID == 119 ||
		_uID == 120)
	{
		pNewMonster = new ManWormMonster;
	}
	else if(_uID == 123)
	{
		pNewMonster = new BlueGhasterMonster;
	}
	else if(_uID == 125)
	{
		pNewMonster = new BehemothDevourerMonster;
	}
	else if(_uID == 132)
	{
		pNewMonster = new MoonLizardMonster;
	}
	else if(_uID == 133)
	{
		pNewMonster = new MoonStatueMonster;
	}
	else if(_uID == 134)
	{
		pNewMonster = new MoonSpiderMonster;
	}
	else if(_uID == 135)
	{
		pNewMonster = new MoonBeastMonster;
	}
	else if(_uID == 136)
	{
		pNewMonster = new MoonWarLordMonster;
	}
	else if(_uID == 138)
	{
		pNewMonster = new YamaKingMonster;
	}
	else if(_uID == 139)
	{
		pNewMonster = new YamaWatcherMonster;
	}
	else if(_uID == 140 ||
		_uID == 143)
	{
		pNewMonster = new FireDragonMonster;
	}
	else if(_uID == 144)
	{
		pNewMonster = new BowManMonster;
	}
	else if(_uID == 147)
	{
		pNewMonster = new SuperBullKing;
	}
	else if(_uID == 149)
	{
		pNewMonster = new DevilOldManMonster;
	}
	else if(_uID == 128 ||
		_uID == 153)
	{
		pNewMonster = new QieEMonster;
	}
	else if(_uID == 158)
	{
		pNewMonster = new WoMaJiaoZhuMonster;
	}
	else
	{
		pNewMonster = new MonsterObject;
	}

	return pNewMonster;
}
//////////////////////////////////////////////////////////////////////////
int GameScene::CreateNormalMonster(unsigned int _uID, unsigned short _uX, unsigned short _uY)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		LOG(ERROR) << "No data have loaded!CreateMonster error!";
		return 0;
	}
	MonsterObject* pNewMonster = NULL;
	pNewMonster = NewMonsterByID(_uID);

	if(pNewMonster)
	{
		pNewMonster->SetID(GameWorld::GetInstance().GenerateObjectID());
		pNewMonster->GetUserData()->stAttrib.id = _uID;
		pNewMonster->GetUserData()->wCoordX = _uX;
		pNewMonster->GetUserData()->wCoordY = _uY;
		pNewMonster->GetUserData()->wMapID = (WORD)m_dwMapID;

#ifdef _SYNC_CREATE
		if(GetRecordInMonsterTable(_uID, &pNewMonster->GetUserData()->stAttrib))
		{
			pNewMonster->EncryptObject();

			if(InsertNPC(pNewMonster))
			{
#ifdef _DEBUG
				LOG(INFO) << "地图[" << GetMapID() << "]生成Monster[" << pNewMonster->GetObject_ID() << "]成功";
#endif
				OnMonsterCreated(_uID, pNewMonster);
			}
			else
			{
				SAFE_DELETE(pNewMonster);
			}
		}
		else
		{
			SAFE_DELETE(pNewMonster);
		}
#else
		DBOperationParam* pParam = new DBOperationParam;
		pParam->dwOperation = DO_QUERY_MONSATTRIB;
		pParam->dwParam[0] = (DWORD)pNewMonster;
		pParam->dwParam[1] = pNewMonster->GetUserData()->stAttrib.id;
		DBThread::GetInstance()->AsynExecute(pParam);
#endif
	}
#ifdef _DEBUG_NEW_
	LOG(INFO) << "new operate on CreateMonster:" << _uID;
#endif
	if(pNewMonster)
	{
		return pNewMonster->GetID();
	}
	else
	{
		return 0;
	}
}
//////////////////////////////////////////////////////////////////////////
int GameScene::CreateLeaderMonster(unsigned int _uID, unsigned short _uX, unsigned short _uY)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		LOG(ERROR) << "No data have loaded!CreateMonster error!";
		return 0;
	}
	MonsterObject* pNewMonster = NULL;
	pNewMonster = NewMonsterByID(_uID);

	if(pNewMonster)
	{
		pNewMonster->SetID(GameWorld::GetInstance().GenerateObjectID());
		pNewMonster->GetUserData()->stAttrib.id = _uID;
		pNewMonster->GetUserData()->wCoordX = _uX;
		pNewMonster->GetUserData()->wCoordY = _uY;
		pNewMonster->GetUserData()->wMapID = (WORD)m_dwMapID;

#ifdef _SYNC_CREATE
		if(GetRecordInMonsterTable(_uID, &pNewMonster->GetUserData()->stAttrib))
		{
			pNewMonster->EncryptObject();

			//	Evolution
			bool bCanEvolute = true;

			if(pNewMonster->GetObject_ID() == 14 ||
				pNewMonster->GetObject_ID() == 62 ||
				pNewMonster->GetObject_ID() == 82 ||
				pNewMonster->GetObject_ID() == 29)
			{
				bCanEvolute = false;
			}

			if(bCanEvolute)
			{
				//int nRandom = rand() % MONS_EVOLUTE_PROB;
				int nRandom = 0;

				if(nRandom <= 2)
				{
					if(nRandom == 0)
					{
						//	Leader
						UINT uFlag = pNewMonster->GetObject_MaxExpr();
						SET_FLAG(uFlag, MAXEXPR_MASK_LEADER);
						pNewMonster->SetObject_MaxExpr(uFlag);

						pNewMonster->SetObject_MaxHP(pNewMonster->GetObject_MaxHP() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_HP(pNewMonster->GetObject_HP() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_DC(pNewMonster->GetObject_DC() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_MaxDC(pNewMonster->GetObject_MaxDC() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_Expr(pNewMonster->GetObject_Expr() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_MC(pNewMonster->GetObject_MC() / MONS_LEADER_MULTI);
						pNewMonster->SetObject_MaxMC(pNewMonster->GetObject_MaxMC() / MONS_LEADER_MULTI);
						pNewMonster->SetObject_Accuracy(pNewMonster->GetObject_Accuracy() * MONS_LEADER_MULTI);
					}
					else
					{
						//	Elite
						UINT uFlag = pNewMonster->GetObject_MaxExpr();
						SET_FLAG(uFlag, MAXEXPR_MASK_ELITE);
						pNewMonster->SetObject_MaxExpr(uFlag);

						pNewMonster->SetObject_MaxHP(pNewMonster->GetObject_MaxHP() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_HP(pNewMonster->GetObject_HP() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_DC(pNewMonster->GetObject_DC() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_MaxDC(pNewMonster->GetObject_MaxDC() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_Expr(pNewMonster->GetObject_Expr() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_MC(pNewMonster->GetObject_MC() / MONS_ELITE_MULTI);
						pNewMonster->SetObject_MaxMC(pNewMonster->GetObject_MaxMC() / MONS_ELITE_MULTI);
						pNewMonster->SetObject_Accuracy(pNewMonster->GetObject_Accuracy() * MONS_ELITE_MULTI);
					}
				}
			}

			if(InsertNPC(pNewMonster))
			{
#ifdef _DEBUG
				LOG(INFO) << "地图[" << GetMapID() << "]生成Monster[" << pNewMonster->GetObject_ID() << "]成功";
#endif
				OnMonsterCreated(_uID, pNewMonster);
			}
			else
			{
				SAFE_DELETE(pNewMonster);
			}
		}
		else
		{
			SAFE_DELETE(pNewMonster);
		}
#else
		DBOperationParam* pParam = new DBOperationParam;
		pParam->dwOperation = DO_QUERY_MONSATTRIB;
		pParam->dwParam[0] = (DWORD)pNewMonster;
		pParam->dwParam[1] = pNewMonster->GetUserData()->stAttrib.id;
		DBThread::GetInstance()->AsynExecute(pParam);
#endif
	}
#ifdef _DEBUG_NEW_
	LOG(INFO) << "new operate on CreateMonster:" << _uID;
#endif
	if(pNewMonster)
	{
		return pNewMonster->GetID();
	}
	else
	{
		return 0;
	}
}
//////////////////////////////////////////////////////////////////////////
int GameScene::CreateEliteMonster(unsigned int _uID, unsigned short _uX, unsigned short _uY)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		LOG(ERROR) << "No data have loaded!CreateMonster error!";
		return 0;
	}
	MonsterObject* pNewMonster = NULL;
	pNewMonster = NewMonsterByID(_uID);

	if(pNewMonster)
	{
		pNewMonster->SetID(GameWorld::GetInstance().GenerateObjectID());
		pNewMonster->GetUserData()->stAttrib.id = _uID;
		pNewMonster->GetUserData()->wCoordX = _uX;
		pNewMonster->GetUserData()->wCoordY = _uY;
		pNewMonster->GetUserData()->wMapID = (WORD)m_dwMapID;

#ifdef _SYNC_CREATE
		if(GetRecordInMonsterTable(_uID, &pNewMonster->GetUserData()->stAttrib))
		{
			pNewMonster->EncryptObject();

			//	Evolution
			bool bCanEvolute = true;

			if(pNewMonster->GetObject_ID() == 14 ||
				pNewMonster->GetObject_ID() == 62 ||
				pNewMonster->GetObject_ID() == 82 ||
				pNewMonster->GetObject_ID() == 29)
			{
				bCanEvolute = false;
			}

			if(bCanEvolute)
			{
				//int nRandom = rand() % MONS_EVOLUTE_PROB;
				int nRandom = 1;

				if(nRandom <= 2)
				{
					if(nRandom == 0)
					{
						//	Leader
						UINT uFlag = pNewMonster->GetObject_MaxExpr();
						SET_FLAG(uFlag, MAXEXPR_MASK_LEADER);
						pNewMonster->SetObject_MaxExpr(uFlag);

						pNewMonster->SetObject_MaxHP(pNewMonster->GetObject_MaxHP() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_HP(pNewMonster->GetObject_HP() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_DC(pNewMonster->GetObject_DC() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_MaxDC(pNewMonster->GetObject_MaxDC() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_Expr(pNewMonster->GetObject_Expr() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_MC(pNewMonster->GetObject_MC() / MONS_LEADER_MULTI);
						pNewMonster->SetObject_MaxMC(pNewMonster->GetObject_MaxMC() / MONS_LEADER_MULTI);
						pNewMonster->SetObject_Accuracy(pNewMonster->GetObject_Accuracy() * MONS_LEADER_MULTI);
					}
					else
					{
						//	Elite
						UINT uFlag = pNewMonster->GetObject_MaxExpr();
						SET_FLAG(uFlag, MAXEXPR_MASK_ELITE);
						pNewMonster->SetObject_MaxExpr(uFlag);

						pNewMonster->SetObject_MaxHP(pNewMonster->GetObject_MaxHP() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_HP(pNewMonster->GetObject_HP() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_DC(pNewMonster->GetObject_DC() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_MaxDC(pNewMonster->GetObject_MaxDC() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_Expr(pNewMonster->GetObject_Expr() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_MC(pNewMonster->GetObject_MC() / MONS_ELITE_MULTI);
						pNewMonster->SetObject_MaxMC(pNewMonster->GetObject_MaxMC() / MONS_ELITE_MULTI);
						pNewMonster->SetObject_Accuracy(pNewMonster->GetObject_Accuracy() * MONS_ELITE_MULTI);
					}
				}
			}

			if(InsertNPC(pNewMonster))
			{
#ifdef _DEBUG
				LOG(INFO) << "地图[" << GetMapID() << "]生成Monster[" << pNewMonster->GetObject_ID() << "]成功";
#endif
				OnMonsterCreated(_uID, pNewMonster);
			}
			else
			{
				SAFE_DELETE(pNewMonster);
			}
		}
		else
		{
			SAFE_DELETE(pNewMonster);
		}
#else
		DBOperationParam* pParam = new DBOperationParam;
		pParam->dwOperation = DO_QUERY_MONSATTRIB;
		pParam->dwParam[0] = (DWORD)pNewMonster;
		pParam->dwParam[1] = pNewMonster->GetUserData()->stAttrib.id;
		DBThread::GetInstance()->AsynExecute(pParam);
#endif
	}
#ifdef _DEBUG_NEW_
	LOG(INFO) << "new operate on CreateMonster:" << _uID;
#endif
	if(pNewMonster)
	{
		return pNewMonster->GetID();
	}
	else
	{
		return 0;
	}
}
//////////////////////////////////////////////////////////////////////////
int GameScene::CreateMonster(unsigned int _uID, unsigned short _uX, unsigned short _uY)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		LOG(ERROR) << "No data have loaded!CreateMonster error!";
		return 0;
	}
	MonsterObject* pNewMonster = NULL;
	pNewMonster = NewMonsterByID(_uID);

	if(pNewMonster)
	{
		pNewMonster->SetID(GameWorld::GetInstance().GenerateObjectID());
		pNewMonster->GetUserData()->stAttrib.id = _uID;
		pNewMonster->GetUserData()->wCoordX = _uX;
		pNewMonster->GetUserData()->wCoordY = _uY;
		pNewMonster->GetUserData()->wMapID = (WORD)m_dwMapID;

#ifdef _SYNC_CREATE
		if(GetRecordInMonsterTable(_uID, &pNewMonster->GetUserData()->stAttrib))
		{
			pNewMonster->EncryptObject();

			//	Evolution
			bool bCanEvolute = true;

			if(pNewMonster->GetObject_ID() == 14 ||
				pNewMonster->GetObject_ID() == 62 ||
				pNewMonster->GetObject_ID() == 82 ||
				pNewMonster->GetObject_ID() == 29)
			{
				bCanEvolute = false;
			}

#ifdef _DEBUG
			/*if(GameWorld::GetInstance().GetOnlinePlayerCount() <= 1)
			{
				bCanEvolute = false;
			}*/
#else
			//if(SettingLoader::GetInstance()->GetIntValue("GENELITEMONS") == 0)
			if(!GameWorld::GetInstance().IsGenElitMons())
			{
				bCanEvolute = false;
			}
#endif

			if(bCanEvolute)
			{
				int nRandom = rand() % MONS_EVOLUTE_PROB;

				if(nRandom <= 2)
				{
					if(nRandom == 0)
					{
						//	Leader
						UINT uFlag = pNewMonster->GetObject_MaxExpr();
						SET_FLAG(uFlag, MAXEXPR_MASK_LEADER);
						pNewMonster->SetObject_MaxExpr(uFlag);

						pNewMonster->SetObject_MaxHP(pNewMonster->GetObject_MaxHP() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_HP(pNewMonster->GetObject_HP() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_DC(pNewMonster->GetObject_DC() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_MaxDC(pNewMonster->GetObject_MaxDC() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_Expr(pNewMonster->GetObject_Expr() * MONS_LEADER_MULTI);
						pNewMonster->SetObject_MC(pNewMonster->GetObject_MC() / MONS_LEADER_MULTI);
						pNewMonster->SetObject_MaxMC(pNewMonster->GetObject_MaxMC() / MONS_LEADER_MULTI);
						pNewMonster->SetObject_Accuracy(pNewMonster->GetObject_Accuracy() * MONS_LEADER_MULTI);
					}
					else
					{
						//	Elite
						UINT uFlag = pNewMonster->GetObject_MaxExpr();
						SET_FLAG(uFlag, MAXEXPR_MASK_ELITE);
						pNewMonster->SetObject_MaxExpr(uFlag);

						pNewMonster->SetObject_MaxHP(pNewMonster->GetObject_MaxHP() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_HP(pNewMonster->GetObject_HP() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_DC(pNewMonster->GetObject_DC() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_MaxDC(pNewMonster->GetObject_MaxDC() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_Expr(pNewMonster->GetObject_Expr() * MONS_ELITE_MULTI);
						pNewMonster->SetObject_MC(pNewMonster->GetObject_MC() / MONS_ELITE_MULTI);
						pNewMonster->SetObject_MaxMC(pNewMonster->GetObject_MaxMC() / MONS_ELITE_MULTI);
						pNewMonster->SetObject_Accuracy(pNewMonster->GetObject_Accuracy() * MONS_ELITE_MULTI);
					}
				}
			}

			if(InsertNPC(pNewMonster))
			{
#ifdef _DEBUG
				LOG(INFO) << "地图[" << GetMapID() << "]生成Monster[" << pNewMonster->GetObject_ID() << "]成功";
#endif
				OnMonsterCreated(_uID, pNewMonster);
			}
			else
			{
				SAFE_DELETE(pNewMonster);
			}
		}
		else
		{
			SAFE_DELETE(pNewMonster);
		}
#else
		DBOperationParam* pParam = new DBOperationParam;
		pParam->dwOperation = DO_QUERY_MONSATTRIB;
		pParam->dwParam[0] = (DWORD)pNewMonster;
		pParam->dwParam[1] = pNewMonster->GetUserData()->stAttrib.id;
		DBThread::GetInstance()->AsynExecute(pParam);
#endif
	}
#ifdef _DEBUG_NEW_
	LOG(INFO) << "new operate on CreateMonster:" << _uID;
#endif
	if(pNewMonster)
	{
		//LOG(INFO) << "G" << pNewMonster->GetObject_ID() << "|" << (void*)pNewMonster;
		return pNewMonster->GetID();
	}
	else
	{
		return 0;
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::OnMonsterCreated(unsigned int _uID, MonsterObject* _pMonster)
{
	RECORD_FUNCNAME_WORLD;

	//	设置一些属性
	/*if(_pMonster->GetAttribID() == 128)
	{
		//	蛾子
		_pMonster->SetStoneProb(12);
		_pMonster->SetStoneTime(2500);
	}
	else if(_pMonster->GetAttribID() == 153)
	{
		//	幻之蛾子
		_pMonster->SetStoneProb(20);
		_pMonster->SetStoneTime(2500);
	}*/

	_pMonster->UpdateCostTime();
	_pMonster->GetValidCheck()->UpdateAllAttrib();

	bool bCanEvolute = true;

	if(_pMonster->GetMaster() != NULL)
	{
		bCanEvolute = false;
	}

	if(_pMonster->GetMaster() != NULL)
	{
		PkgPlayerUpdateAttribNtf ntf;
		ntf.uTargetId = _pMonster->GetID();
		ntf.bType = UPDATE_MASTER;
		ntf.dwParam = _pMonster->GetMaster()->GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ntf;
		BroadcastPacket(&g_xThreadBuffer);

		if(_pMonster->GetObject_ID() == MONSTER_SHENSHOU ||
			_pMonster->GetObject_ID() == 62 ||
			_pMonster->GetObject_ID() == 99)
		{
			int nSC = 0;

			if(_pMonster->GetMaster()->GetType() == SOT_HERO)
			{
				HeroObject* pHero = static_cast<HeroObject*>(_pMonster->GetMaster());
				{
					int nLevel = 0;
					if(_pMonster->GetObject_ID() == MONSTER_SHENSHOU)
					{
						nLevel = pHero->GetUserMagic(MEFF_SUMMON)->bLevel;
					}
					else if(_pMonster->GetObject_ID() == 62)
					{
						nLevel = pHero->GetUserMagic(MEFF_SUPERSUMMON)->bLevel;
					}
					else if(_pMonster->GetObject_ID() == 99)
					{
						nLevel = pHero->GetUserMagic(MEFF_SUMMONTIGER)->bLevel;
					}
					nSC = pHero->GetRandomAbility(AT_SC);

					if(nLevel > 3)
					{
						nLevel = 3;
					}

					if(nLevel > 0)
					{
						_pMonster->SetInitLevel(nLevel);
						_pMonster->SetUpgradeLevel(nLevel);

						for(int i = 0; i < nLevel; ++i)
						{
							_pMonster->SlaveLevelUp();
						}

						ntf.bType = UPDATE_SLAVELV;
						ntf.dwParam = _pMonster->GetObject_MaxAC();
						ntf.uTargetId = _pMonster->GetID();
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						BroadcastPacket(&g_xThreadBuffer);
					}
				}
			}
		}
		else if(_pMonster->GetObject_ID() == MONSTER_KULOU)
		{
			HeroObject* pHero = static_cast<HeroObject*>(_pMonster->GetMaster());
			{
				int nLevel = pHero->GetUserMagic(MEFF_KULOU)->bLevel;
				if(nLevel > 0)
				{
					_pMonster->SetInitLevel(nLevel);
					_pMonster->SetUpgradeLevel(nLevel);

					ntf.bType = UPDATE_SLAVELV;
					ntf.dwParam = _pMonster->GetObject_MaxAC();
					ntf.uTargetId = _pMonster->GetID();
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					_pMonster->GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
				}
			}
		}
	}

	if(bCanEvolute)
	{
		int nRandom = rand() % 40;

		if(nRandom <= 2)
		{
			if(nRandom == 0)
			{
				//	Leader
			}
			else
			{
				//	Elite
			}
		}
	}

	if(_pMonster->GetMaster() == NULL)
	{
		if(GetMonsterAttackMulti() != 0.0f)
		{
			_pMonster->SetObject_DC(_pMonster->GetObject_DC() * GetMonsterAttackMulti());
			_pMonster->SetObject_MaxDC(_pMonster->GetObject_MaxDC() * GetMonsterAttackMulti());
		}
		if(GetMonsterDefenceMulti() != 0.0f)
		{
			_pMonster->SetObject_MaxAC(_pMonster->GetObject_MaxAC() * GetMonsterDefenceMulti());
			_pMonster->SetObject_MaxMAC(_pMonster->GetObject_MaxMAC() * GetMonsterDefenceMulti());
		}
	}
}
//////////////////////////////////////////////////////////////////////////
int GameScene::CreateMonster(unsigned int _uID)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		LOG(ERROR) << "No data have loaded!CreateMonster error!";
		return 0;
	}

	DWORD dwPos = 0;
	if(GetRandomPosition(&dwPos))
	{
		return CreateMonster(_uID, LOWORD(dwPos), HIWORD(dwPos));
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////
int GameScene::CreateNormalMonster(unsigned int _uID)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		LOG(ERROR) << "No data have loaded!CreateMonster error!";
		return 0;
	}

	DWORD dwPos = 0;
	if(GetRandomPosition(&dwPos))
	{
		return CreateNormalMonster(_uID, LOWORD(dwPos), HIWORD(dwPos));
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////
int GameScene::CreateEliteMonster(unsigned int _uID)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		LOG(ERROR) << "No data have loaded!CreateMonster error!";
		return 0;
	}

	DWORD dwPos = 0;
	if(GetRandomPosition(&dwPos))
	{
		return CreateEliteMonster(_uID, LOWORD(dwPos), HIWORD(dwPos));
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////
int GameScene::CreateLeaderMonster(unsigned int _uID)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		LOG(ERROR) << "No data have loaded!CreateMonster error!";
		return 0;
	}

	DWORD dwPos = 0;
	if(GetRandomPosition(&dwPos))
	{
		return CreateLeaderMonster(_uID, LOWORD(dwPos), HIWORD(dwPos));
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateMonster(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval)
{
	RECORD_FUNCNAME_WORLD;

	if(_ux >= m_stMapInfo.nCol ||
		_uy >= m_stMapInfo.nRow)
	{
		return;
	}

	MonsterGenerateInfo* pInfo = new MonsterGenerateInfo;
	memset(pInfo, 0, sizeof(MonsterGenerateInfo));

	pInfo->bNumber = _number;
	pInfo->bOft = _oft;
	pInfo->dwInterval = _uinterval;
	pInfo->dwMonsID = _uID;
	pInfo->bMonsGenerateType = kMonsGenerateDefault;
	pInfo->dwPos = MAKELONG(_ux, _uy);
	m_xMonsterGenerator.push_back(pInfo);
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateEliteMonster(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval)
{
	RECORD_FUNCNAME_WORLD;

	if(_ux >= m_stMapInfo.nCol ||
		_uy >= m_stMapInfo.nRow)
	{
		return;
	}

	MonsterGenerateInfo* pInfo = new MonsterGenerateInfo;
	memset(pInfo, 0, sizeof(MonsterGenerateInfo));

	pInfo->bNumber = _number;
	pInfo->bOft = _oft;
	pInfo->dwInterval = _uinterval;
	pInfo->dwMonsID = _uID;
	pInfo->bMonsGenerateType = kMonsGenerateElite;
	pInfo->dwPos = MAKELONG(_ux, _uy);
	m_xMonsterGenerator.push_back(pInfo);
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateLeaderMonster(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval)
{
	RECORD_FUNCNAME_WORLD;

	if(_ux >= m_stMapInfo.nCol ||
		_uy >= m_stMapInfo.nRow)
	{
		return;
	}

	MonsterGenerateInfo* pInfo = new MonsterGenerateInfo;
	memset(pInfo, 0, sizeof(MonsterGenerateInfo));

	pInfo->bNumber = _number;
	pInfo->bOft = _oft;
	pInfo->dwInterval = _uinterval;
	pInfo->dwMonsID = _uID;
	pInfo->bMonsGenerateType = kMonsGenerateLeader;
	pInfo->dwPos = MAKELONG(_ux, _uy);
	m_xMonsterGenerator.push_back(pInfo);
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateMonsterLater(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval)
{
	RECORD_FUNCNAME_WORLD;

	if(_ux >= m_stMapInfo.nCol ||
		_uy >= m_stMapInfo.nRow)
	{
		return;
	}

	MonsterGenerateInfo* pInfo = new MonsterGenerateInfo;
	memset(pInfo, 0, sizeof(MonsterGenerateInfo));

	pInfo->bNumber = _number;
	pInfo->bOft = _oft;
	pInfo->dwInterval = _uinterval;
	pInfo->dwMonsID = _uID;
	pInfo->dwPos = MAKELONG(_ux, _uy);
	pInfo->dwLastExecuteTime = GetTickCount();
	pInfo->bMonsGenerateType = kMonsGenerateDefault;
	m_xMonsterGenerator.push_back(pInfo);
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateEliteMonsterLater(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval)
{
	RECORD_FUNCNAME_WORLD;

	if(_ux >= m_stMapInfo.nCol ||
		_uy >= m_stMapInfo.nRow)
	{
		return;
	}

	MonsterGenerateInfo* pInfo = new MonsterGenerateInfo;
	memset(pInfo, 0, sizeof(MonsterGenerateInfo));

	pInfo->bNumber = _number;
	pInfo->bOft = _oft;
	pInfo->dwInterval = _uinterval;
	pInfo->dwMonsID = _uID;
	pInfo->dwPos = MAKELONG(_ux, _uy);
	pInfo->dwLastExecuteTime = GetTickCount();
	pInfo->bMonsGenerateType = kMonsGenerateElite;
	m_xMonsterGenerator.push_back(pInfo);
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateLeaderMonsterLater(unsigned int _uID, unsigned short _ux, unsigned short _uy, unsigned short _oft, unsigned short _number, unsigned int _uinterval)
{
	RECORD_FUNCNAME_WORLD;

	if(_ux >= m_stMapInfo.nCol ||
		_uy >= m_stMapInfo.nRow)
	{
		return;
	}

	MonsterGenerateInfo* pInfo = new MonsterGenerateInfo;
	memset(pInfo, 0, sizeof(MonsterGenerateInfo));

	pInfo->bNumber = _number;
	pInfo->bOft = _oft;
	pInfo->dwInterval = _uinterval;
	pInfo->dwMonsID = _uID;
	pInfo->dwPos = MAKELONG(_ux, _uy);
	pInfo->dwLastExecuteTime = GetTickCount();
	pInfo->bMonsGenerateType = kMonsGenerateLeader;
	m_xMonsterGenerator.push_back(pInfo);
}
//////////////////////////////////////////////////////////////////////////
void GameScene::SetCityRange(int _x, int _y, int _width, int _height, int _centerx, int _centery)
{
	RECORD_FUNCNAME_WORLD;

	m_stCityRect.left = _x;
	m_stCityRect.right = _x + _width;
	m_stCityRect.top = _y;
	m_stCityRect.bottom = _y + _height;
	m_dwCityCenter = MAKELONG(_centerx, _centery);
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::IsInCity(WORD _wPosX, WORD _wPosY)
{
	RECORD_FUNCNAME_WORLD;

	if(m_stCityRect.left == m_stCityRect.right)
	{
		return false;
	}
	if(m_dwCityCenter == 0)
	{
		return false;
	}

	POINT ptTest = {_wPosX, _wPosY};
	return (PtInRect(&m_stCityRect, ptTest)) ? true : false;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::AutoGenerateMonster()
{
	RECORD_FUNCNAME_WORLD;
	ExecuteTimer xExeTimer;
	xExeTimer.Start();

	if(m_xMonsterGenerator.empty())
	{
		return;
	}
	if(m_xPlayers.empty())
	{
		return;
	}

	DWORD dwCurTick = GetTickCount();

	int nCanGenerate = 0;
	int nLoopGuard = 0;
	MonsterGenerateInfo* pInfo = NULL;
	MONSTERGENERATEINFOLIST::iterator endIter = m_xMonsterGenerator.end();
	MONSTERGENERATEINFOLIST::iterator gbegiter = m_xMonsterGenerator.begin();
	std::map<DWORD, GameObject*>::const_iterator mbegiter = m_xNPCs.begin();
	for(gbegiter; gbegiter != endIter;
		++gbegiter)
	{
		pInfo = *gbegiter;
		if(dwCurTick - pInfo->dwLastExecuteTime < pInfo->dwInterval)
		{
			continue;
		}
		else
		{
			pInfo->dwLastExecuteTime = dwCurTick;
		}

		nCanGenerate = pInfo->bNumber;
		/*nCanGenerate = 0;
		MONSTERGENERATEINFOLIST::iterator begIter = m_xMonsterGenerator.begin();
		for(begIter;
			begIter != endIter;
			++begIter)
		{
			if((*begIter)->dwMonsID == pInfo->dwMonsID)
			{
				nCanGenerate += (*begIter)->bNumber;
			}
		}*/

		mbegiter = m_xNPCs.begin();
		for(mbegiter; mbegiter != m_xNPCs.end(); ++mbegiter)
		{
			if(mbegiter->second->GetObject_ID() == pInfo->dwMonsID &&
				mbegiter->second->GetUserData()->eGameState != OS_DEAD)
			{
				if(mbegiter->second->GetType() == SOT_MONSTER)
				{
					MonsterObject* pMonster = static_cast<MonsterObject*>(mbegiter->second);
					if(pMonster->GetMaster() == NULL)
					{
						--nCanGenerate;
					}
				}
				else
				{
					--nCanGenerate;
				}
			}
		}
		if(nCanGenerate > 0)
		{
			WORD wBaseX = LOWORD(pInfo->dwPos);
			WORD wBaseY = HIWORD(pInfo->dwPos);
			RECT rcGenerate;
			rcGenerate.left = (int)wBaseX - (int)pInfo->bOft;
			if(rcGenerate.left < 0)
			{
				rcGenerate.left = 0;
			}
			rcGenerate.right = wBaseX + pInfo->bOft;
			rcGenerate.top = (int)wBaseY - (int)pInfo->bOft;
			if(rcGenerate.top < 0)
			{
				rcGenerate.top = 0;
			}
			rcGenerate.bottom = wBaseY + pInfo->bOft;
			int nGeneX = 0;
			int nGeneY = 0;
			nLoopGuard = 0;

			if(rcGenerate.left == rcGenerate.right &&
				rcGenerate.top == rcGenerate.bottom)
			{
				if(CanThrough(rcGenerate.left, rcGenerate.top))
				{
					if(pInfo->bMonsGenerateType == kMonsGenerateDefault)
					{
						CreateMonster(pInfo->dwMonsID, rcGenerate.left, rcGenerate.top);
					}
					else if(pInfo->bMonsGenerateType == kMonsGenerateNormal)
					{
						CreateNormalMonster(pInfo->dwMonsID, rcGenerate.left, rcGenerate.top);
					}
					else if(pInfo->bMonsGenerateType == kMonsGenerateElite)
					{
						CreateEliteMonster(pInfo->dwMonsID, rcGenerate.left, rcGenerate.top);
					}
					else if(pInfo->bMonsGenerateType == kMonsGenerateLeader)
					{
						CreateLeaderMonster(pInfo->dwMonsID, rcGenerate.left, rcGenerate.top);
					}
				}
			}
			else
			{
				for(int i = 0; i < nCanGenerate; ++i)
				{
					nGeneX = rand() % (rcGenerate.right - rcGenerate.left) + rcGenerate.left;
					nGeneY = rand() % (rcGenerate.bottom - rcGenerate.top) + rcGenerate.top;
					++nLoopGuard;
					if(CanThrough(nGeneX, nGeneY))
					{
						if(pInfo->bMonsGenerateType == kMonsGenerateDefault)
						{
							CreateMonster(pInfo->dwMonsID, nGeneX, nGeneY);
						}
						else if(pInfo->bMonsGenerateType == kMonsGenerateNormal)
						{
							CreateNormalMonster(pInfo->dwMonsID, nGeneX, nGeneY);
						}
						else if(pInfo->bMonsGenerateType == kMonsGenerateElite)
						{
							CreateEliteMonster(pInfo->dwMonsID, nGeneX, nGeneY);
						}
						else if(pInfo->bMonsGenerateType == kMonsGenerateLeader)
						{
							CreateLeaderMonster(pInfo->dwMonsID, nGeneX, nGeneY);
						}
					}
					if(nLoopGuard > nCanGenerate + 10)
					{
						break;
					}
				}
			}
		}
	}

	if(GetMapID() == 30)
	{
		DWORD dwExeTime = xExeTimer.GetExecuteTime();
		dwExeTime += 1;
	}
}
//////////////////////////////////////////////////////////////////////////
GameObject* GameScene::GetOnePlayerInRange(RECT& _rc, bool _bCanSeeHide /* = false */)
{
	RECORD_FUNCNAME_WORLD;

	if(m_xPlayers.empty())
	{
		return NULL;
	}

	POINT ptPlayer;
	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		ptPlayer.x = iter->second->GetUserData()->wCoordX;
		ptPlayer.y = iter->second->GetUserData()->wCoordY;
		if(PtInRect(&_rc, ptPlayer))
		{
			if(iter->second->GetObject_HP() != 0 &&
				iter->second->GetUserData()->eGameState != OS_DEAD)
			{
				if(_bCanSeeHide)
				{
					return iter->second;
				}
				else
				{
					if(!iter->second->IsHide())
					{
						return iter->second;
					}
				}
			}
		}
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////////
GameObject* GameScene::MonsterGetOneTargetInRange(const RECT& _rc, bool _bCanSeeHide /* = false */)
{
	RECORD_FUNCNAME_WORLD;

	MapCellInfo* pCell = NULL;
	CELLDATALIST* pObjs = NULL;

	for(int i = _rc.left; i <= _rc.right; ++i)
	{
		for(int j = _rc.top; j < _rc.bottom; ++j)
		{
			pCell = GetMapData(i, j);
			if(NULL != pCell)
			{
				pObjs = pCell->pCellObjects;
				if(NULL != pObjs)
				{
					CELLDATALIST::const_iterator begiter = pObjs->begin();
					for(begiter;
						begiter != pObjs->end();
						++begiter)
					{
						if((*begiter)->bType == CELL_MOVEOBJECT)
						{
							GameObject* pObj = (GameObject*)(*begiter)->pData;
							if(pObj->GetType() == SOT_MONSTER)
							{
								MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
								if(pMonster->GetMaster() != NULL)
								{
									if(pMonster->GetUserData()->eGameState != OS_DEAD &&
										pMonster->GetObject_HP() != 0)
									{
										return pMonster;
									}
								}
							}
							else if(pObj->GetType() == SOT_HERO)
							{
								HeroObject* pHero = static_cast<HeroObject*>(pObj);
								if(pHero->GetUserData()->eGameState != OS_DEAD &&
									pHero->GetObject_HP() != 0)
								{
									if(!pHero->IsGmHide())
									{
										if(pHero->IsHide())
										{
											if(_bCanSeeHide)
											{
												return pHero;
											}
										}
										else
										{
											return pHero;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////////
GameObject* GameScene::SlaveGetOneTargetInRange(const RECT& _rc, bool _bCanSeeHide /* = false */)
{
	RECORD_FUNCNAME_WORLD;

	MapCellInfo* pCell = NULL;
	CELLDATALIST* pObjs = NULL;

	for(int i = _rc.left; i <= _rc.right; ++i)
	{
		for(int j = _rc.top; j < _rc.bottom; ++j)
		{
			pCell = GetMapData(i, j);
			if(NULL != pCell)
			{
				pObjs = pCell->pCellObjects;
				if(NULL != pObjs)
				{
					CELLDATALIST::const_iterator begiter = pObjs->begin();
					for(begiter;
						begiter != pObjs->end();
						++begiter)
					{
						if((*begiter)->bType == CELL_MOVEOBJECT)
						{
							GameObject* pObj = (GameObject*)(*begiter)->pData;
							if(pObj->GetType() == SOT_MONSTER)
							{
								MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
								if(pMonster->GetMaster() == NULL)
								{
									if(pMonster->GetUserData()->eGameState != OS_DEAD &&
										pMonster->GetObject_HP() != 0)
									{
										if(pMonster->GetViewRange() != 0)
										{
											return pMonster;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::EraseTarget(GameObject* _pObj)
{
	RECORD_FUNCNAME_WORLD;

	MonsterObject* pMons = NULL;
	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xNPCs.begin();
		iter != m_xNPCs.end();
		++iter)
	{
		if(iter->second->GetType() == SOT_MONSTER)
		{
			pMons = (MonsterObject*)iter->second;
			if(pMons->GetTarget() == _pObj)
			{
				pMons->SetTarget(NULL);
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::AddMappedObject(int _x, int _y,BYTE _type, void* _pData)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		return false;
	}

	MapCellInfo* pCell = GetMapData(_x, _y);
	if(pCell == NULL)
	{
		return NULL;
	}

	if(NULL == pCell->pCellObjects)
	{
		pCell->pCellObjects = new CELLDATALIST;
	}
	CellData* pData = new CellData;
	pData->bType = _type;
	pData->dwInfo = 0;
	pData->pData = _pData;
	pCell->pCellObjects->push_back(pData);

	if(pData->bType == CELL_MOVEOBJECT)
	{
		GameObject* pObj = (GameObject*)_pData;
		pObj->SetValidPositon(_x, _y);
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::RemoveMappedObject(int _x, int _y, void *_pData)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		return false;
	}

	MapCellInfo* pCell = GetMapData(_x, _y);
	if(pCell == NULL)
	{
		return false;
	}

	if(NULL == pCell->pCellObjects)
	{
		//DEBUG_BREAK;
		//LOG(ERROR) << "Cell data empty???!!!";
		return false;
	}

	bool bRet = false;

	CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
	CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();
	for(begiter; begiter != enditer; ++begiter)
	{
		CellData* pData = (*begiter);
		if(pData->pData == _pData)
		{
			delete pData;
			pCell->pCellObjects->erase(begiter);
			bRet = true;
			break;
		}
	}

	if(!bRet)
	{
		//LOG(ERROR) << "Cell data empty?";
	}

	return bRet;
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::MoveMappedObject(int _sx, int _sy, int _dx, int _dy, void *_pData)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pCellData == NULL)
	{
		return false;
	}

	MapCellInfo* pCell = GetMapData(_sx, _sy);
	if(pCell == NULL)
	{
		return false;
	}

	if(NULL == pCell->pCellObjects)
	{
		DEBUG_BREAK;
		return false;
	}

	CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
	CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();
	CellData* pSrcObjData = NULL;
	for(begiter; begiter != enditer; ++begiter)
	{
		CellData* pData = (*begiter);
		if(pData->pData == _pData)
		{
			pSrcObjData = pData;
			pCell->pCellObjects->erase(begiter);
			break;
		}
	}

	if(pSrcObjData != NULL)
	{
		MapCellInfo* pTargetCell = GetMapData(_dx, _dy);
		if(pTargetCell == NULL)
		{
			delete pSrcObjData;
			return false;
		}

		if(NULL == pTargetCell->pCellObjects)
		{
			pTargetCell->pCellObjects = new CELLDATALIST;
#ifdef _DEBUG_NEW_
			LOG(INFO) << "new operate on MoveMappedObject:" << _dx << "," << _dy;
#endif
		}
		pTargetCell->pCellObjects->push_back(pSrcObjData);
		if(pSrcObjData->bType == CELL_MOVEOBJECT)
		{
			GameObject* pObj = (GameObject*)pSrcObjData->pData;
			pObj->SetValidPositon(_dx, _dy);
		}
		return true;
	}
	else
	{
		DEBUG_BREAK;
	}
	delete pSrcObjData;
	return false;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::NewGroundItem(int _id, int _x, int _y)
{
	RECORD_FUNCNAME_WORLD;

	m_xNewGroundItem.push_back(_id);
	m_xNewGroundItem.push_back(_x);
	m_xNewGroundItem.push_back(_y);
}
//////////////////////////////////////////////////////////////////////////
void GameScene::HandleNewItem()
{
	RECORD_FUNCNAME_WORLD;

	if(m_xNewGroundItem.empty())
	{
		return;
	}
	size_t nSize = m_xNewGroundItem.size();
	if(nSize % 3 != 0)
	{
		LOG(ERROR) << "Wrong item params sum";
		m_xNewGroundItem.clear();
	}

	std::list<int>::const_iterator begiter = m_xNewGroundItem.begin();
	std::list<int>::const_iterator enditer = m_xNewGroundItem.end();

	int nId = 0;
	int nPosX = 0;
	int nPosY = 0;
	int nCounter = 0;
	for(begiter; begiter != enditer;)
	{
		nId = *begiter;
		++begiter;
		nPosX = *begiter;
		++begiter;
		nPosY = *begiter;
		++begiter;

		CreateGroundItem(nId, nPosX, nPosY);
	}

	m_xNewGroundItem.clear();
}
//////////////////////////////////////////////////////////////////////////
bool GameScene::PushStaticMagic(const StaticMagic* _pMgc)
{
	RECORD_FUNCNAME_WORLD;

	//	5 position
	static const int s_nFirewallOft[] = {0, 0, 0, 1, 0, -1, 1, 0, -1, 0};
	static const int s_nBigPoisonOft[] = {0, 0,
											-1, 1,
											-1, 0,
											-1, -1,
											0, -1,
											1, -1,
											1, 0,
											1, 1};
	static const int s_n5RadiusOft[] = {
		-2,2,
		-2,1,
		-2,0,
		-2,-1,
		-2,-2,

		-1,2,
		-1,1,
		-1,0,
		-1,-1,
		-1,-2,

		0,2,
		0,1,
		0,0,
		0,-1,
		0,-2,

		1,2,
		1,1,
		1,0,
		1,-1,
		1,-2,

		2,2,
		2,1,
		2,0,
		2,-1,
		2,-2
	};
	bool bSamePos = false;

	if(_pMgc->wMgcID == MEFF_FIREWALL)
	{
		for(int i = 0; i < sizeof(s_nFirewallOft) / sizeof(s_nFirewallOft[0]) / 2; ++i)
		{
			bSamePos = false;
			MapCellInfo* pCell = GetMapData(_pMgc->sPosX + s_nFirewallOft[i * 2], _pMgc->sPoxY + s_nFirewallOft[i * 2 + 1]);

			if(pCell != NULL)
			{
				if(pCell->pCellObjects == NULL)
				{
					pCell->pCellObjects = new CELLDATALIST;
				}
			}
			else
			{
				continue;
			}

			CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
			CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();
			CellData* pData = NULL;
			for(begiter; begiter != enditer; ++begiter)
			{
				pData = *begiter;
				if(pData->bType == CELL_MAGIC)
				{
					StaticMagic* pMgc = (StaticMagic*)pData->pData;
					if(pMgc->wMgcID == _pMgc->wMgcID/* &&
													pMgc->dwFire == _pMgc->dwFire*/)
					{
						pMgc->dwExpire = _pMgc->dwExpire;
						pMgc->dwFire = _pMgc->dwFire;
						//pMgc->pFire = _pMgc->pFire;
						bSamePos = true;
						break;
					}
				}
			}

			if(!bSamePos)
			{
				StaticMagic* pNewMgc = new StaticMagic;
				memcpy(pNewMgc, _pMgc, sizeof(StaticMagic));
				pNewMgc->dwEffectActiveCount = 0;
				pNewMgc->sPosX += s_nFirewallOft[i * 2];
				pNewMgc->sPoxY += s_nFirewallOft[i * 2 + 1];
				m_xStaticMagic.push_back(pNewMgc);

#ifdef _DEBUG
				LOG(INFO) << "new operation on PushStaticMagic";
#endif
				CellData* pData = new CellData;
				pData->bType = CELL_MAGIC;
				pData->dwInfo = 0;
				pData->pData = (void*)pNewMgc;
				pCell->pCellObjects->push_back(pData);
				//return true;
			}
		}
	}
	else if(_pMgc->wMgcID == MEFF_BIGPOISON)
	{
		for(int i = 0; i < sizeof(s_nBigPoisonOft) / sizeof(s_nBigPoisonOft[0]) / 2; ++i)
		{
			bSamePos = false;
			MapCellInfo* pCell = GetMapData(_pMgc->sPosX + s_nBigPoisonOft[i * 2], _pMgc->sPoxY + s_nBigPoisonOft[i * 2 + 1]);

			if(pCell != NULL)
			{
				if(pCell->pCellObjects == NULL)
				{
					pCell->pCellObjects = new CELLDATALIST;
				}
			}
			else
			{
				continue;
			}

			CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
			CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();
			CellData* pData = NULL;
			for(begiter; begiter != enditer; ++begiter)
			{
				pData = *begiter;
				if(pData->bType == CELL_MAGIC)
				{
					StaticMagic* pMgc = (StaticMagic*)pData->pData;
					if(pMgc->wMgcID == _pMgc->wMgcID/* &&
													pMgc->dwFire == _pMgc->dwFire*/)
					{
						pMgc->dwExpire = _pMgc->dwExpire;
						pMgc->dwFire = _pMgc->dwFire;
						//pMgc->pFire = _pMgc->pFire;
						bSamePos = true;
						break;
					}
				}
			}

			if(!bSamePos)
			{
				StaticMagic* pNewMgc = new StaticMagic;
				memcpy(pNewMgc, _pMgc, sizeof(StaticMagic));
				pNewMgc->dwEffectActiveCount = 0;
				pNewMgc->sPosX += s_nBigPoisonOft[i * 2];
				pNewMgc->sPoxY += s_nBigPoisonOft[i * 2 + 1];
				m_xStaticMagic.push_back(pNewMgc);

#ifdef _DEBUG
				LOG(INFO) << "new operation on PushStaticMagic";
#endif
				CellData* pData = new CellData;
				pData->bType = CELL_MAGIC;
				pData->dwInfo = 0;
				pData->pData = (void*)pNewMgc;
				pCell->pCellObjects->push_back(pData);
				//return true;
			}
		}
	}
	else if(_pMgc->wMgcID == MEFF_FIREDRAGON_FIREWALL)
	{
		for(int i = 0; i < sizeof(s_n5RadiusOft) / sizeof(s_n5RadiusOft[0]) / 2; ++i)
		{
			bSamePos = false;
			MapCellInfo* pCell = GetMapData(_pMgc->sPosX + s_n5RadiusOft[i * 2], _pMgc->sPoxY + s_n5RadiusOft[i * 2 + 1]);

			if(pCell != NULL)
			{
				if(pCell->pCellObjects == NULL)
				{
					pCell->pCellObjects = new CELLDATALIST;
				}
			}
			else
			{
				continue;
			}

			CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
			CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();
			CellData* pData = NULL;
			for(begiter; begiter != enditer; ++begiter)
			{
				pData = *begiter;
				if(pData->bType == CELL_MAGIC)
				{
					StaticMagic* pMgc = (StaticMagic*)pData->pData;
					if(pMgc->wMgcID == _pMgc->wMgcID/* &&
													pMgc->dwFire == _pMgc->dwFire*/)
					{
						pMgc->dwExpire = _pMgc->dwExpire;
						pMgc->dwFire = _pMgc->dwFire;
						//pMgc->pFire = _pMgc->pFire;
						bSamePos = true;
						break;
					}
				}
			}

			if(!bSamePos)
			{
				StaticMagic* pNewMgc = new StaticMagic;
				memcpy(pNewMgc, _pMgc, sizeof(StaticMagic));
				pNewMgc->dwEffectActiveCount = 0;
				pNewMgc->sPosX += s_n5RadiusOft[i * 2];
				pNewMgc->sPoxY += s_n5RadiusOft[i * 2 + 1];
				m_xStaticMagic.push_back(pNewMgc);

#ifdef _DEBUG
				LOG(INFO) << "new operation on PushStaticMagic";
#endif
				CellData* pData = new CellData;
				pData->bType = CELL_MAGIC;
				pData->dwInfo = 0;
				pData->pData = (void*)pNewMgc;
				pCell->pCellObjects->push_back(pData);
				//return true;
			}
		}
	}
	else if(_pMgc->wMgcID == MEFF_DRAGONBLUSTER)
	{
		for(int i = 0; i < sizeof(s_n5RadiusOft) / sizeof(s_n5RadiusOft[0]) / 2; ++i)
		{
			bSamePos = false;
			MapCellInfo* pCell = GetMapData(_pMgc->sPosX + s_n5RadiusOft[i * 2], _pMgc->sPoxY + s_n5RadiusOft[i * 2 + 1]);

			if(pCell != NULL)
			{
				if(pCell->pCellObjects == NULL)
				{
					pCell->pCellObjects = new CELLDATALIST;
				}
			}
			else
			{
				continue;
			}

			CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
			CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();
			CellData* pData = NULL;
			for(begiter; begiter != enditer; ++begiter)
			{
				pData = *begiter;
				if(pData->bType == CELL_MAGIC)
				{
					StaticMagic* pMgc = (StaticMagic*)pData->pData;
					if(pMgc->wMgcID == _pMgc->wMgcID/* &&
													pMgc->dwFire == _pMgc->dwFire*/)
					{
						pMgc->dwExpire = _pMgc->dwExpire;
						pMgc->dwFire = _pMgc->dwFire;
						//pMgc->pFire = _pMgc->pFire;
						bSamePos = true;
						break;
					}
				}
			}

			if(!bSamePos)
			{
				StaticMagic* pNewMgc = new StaticMagic;
				memcpy(pNewMgc, _pMgc, sizeof(StaticMagic));
				pNewMgc->dwEffectActiveCount = 0;
				pNewMgc->sPosX += s_n5RadiusOft[i * 2];
				pNewMgc->sPoxY += s_n5RadiusOft[i * 2 + 1];
				m_xStaticMagic.push_back(pNewMgc);

#ifdef _DEBUG
				LOG(INFO) << "new operation on PushStaticMagic";
#endif
				CellData* pData = new CellData;
				pData->bType = CELL_MAGIC;
				pData->dwInfo = 0;
				pData->pData = (void*)pNewMgc;
				pCell->pCellObjects->push_back(pData);
				//return true;
			}
		}
	}
	

	return true;
}
//////////////////////////////////////////////////////////////////////////
void GameScene::ParseStaticMagic(const StaticMagic* _pMgc)
{
	const StaticMagic* pMgc = _pMgc;
	int nPosX = 0;
	int nPosY = 0;
	int nDamage = 0;
	DWORD dwCurTick = GetTickCount();
	static const int s_nAtkOft[] =
	{
		0,0,
		0,-1,
		0,1,
		-1,0,
		1,0
	};

	if(pMgc->wMgcID == MEFF_FIREWALL)
	{
		if(NULL == _pMgc->pFire)
		{
			return;
		}
		if(_pMgc->pFire->GetType() != SOT_HERO)
		{
			return;
		}
		HeroObject* pHero = (HeroObject*)_pMgc->pFire;
		const UserMagic* pUserMagic = pHero->GetUserMagic(MEFF_FIREWALL);
		if(NULL == pUserMagic ||
			0 == pUserMagic->bLevel)
		{
			return;
		}

		nPosX = pMgc->sPosX;
		nPosY = pMgc->sPoxY;
		GameObjectList xObjs;
		GetMappedObjects(nPosX, nPosY, 0, 0, xObjs, MAPPEDOBJECT_ALL_ALIVE);
		GameObjectList::const_iterator objbegiter = xObjs.begin();
		GameObjectList::const_iterator objenditer  = xObjs.end();

		for(objbegiter; objbegiter != objenditer; ++objbegiter)
		{
			GameObject* pObj = (GameObject*)*objbegiter;
			nDamage = pHero->GetMagicDamageNoDefence(pUserMagic);

			if(pObj->GetType() == SOT_MONSTER)
			{
				if(static_cast<MonsterObject*>(pObj)->CanDefFire())
				{
					nDamage /= 1.5;
				}
				if(static_cast<MonsterObject*>(pObj)->IsFrozen())
				{
					nDamage *= 1.5f;
				}
			}

			if(nDamage > 0)
			{
				pObj->ReceiveDamage(pHero, true, nDamage);
			}
		}
	}
	else if(pMgc->wMgcID == MEFF_BIGPOISON)
	{
		if(NULL == _pMgc->pFire)
		{
			return;
		}
		if(_pMgc->pFire->GetType() != SOT_HERO)
		{
			return;
		}
		HeroObject* pHero = (HeroObject*)_pMgc->pFire;
		const UserMagic* pUserMagic = pHero->GetUserMagic(MEFF_BIGPOISON);
		if(NULL == pUserMagic ||
			0 == pUserMagic->bLevel)
		{
			return;
		}

		nPosX = pMgc->sPosX;
		nPosY = pMgc->sPoxY;
		GameObjectList xObjs;
		GetMappedObjects(nPosX, nPosY, 0, 0, xObjs, MAPPEDOBJECT_ALL_ALIVE);
		GameObjectList::const_iterator objbegiter = xObjs.begin();
		GameObjectList::const_iterator objenditer  = xObjs.end();

		for(objbegiter; objbegiter != objenditer; ++objbegiter)
		{
			GameObject* pObj = (GameObject*)*objbegiter;
			nDamage = pHero->GetMagicDamageNoDefence(pUserMagic);

			if(pUserMagic->bLevel == 1)
			{
				nDamage /= 8;
			}
			else if(pUserMagic->bLevel == 2)
			{
				nDamage /= 6;
			}
			else if(pUserMagic->bLevel == 3)
			{
				nDamage /= 4;
			}
			else
			{
				nDamage = 0;
			}

			if(nDamage > 0)
			{
				if(0 != pObj->ReceiveDamage(pHero, true, nDamage))
				{
					int nPoison = 0;
					nPoison = rand() % (8 - pUserMagic->bLevel);

					if(0 == nPoison)
					{
						int nTime = pHero->GetRandomAbility(AT_SC) * 200 + 5000;
						int nPoisonDamage = pHero->GetRandomAbility(AT_SC) / 4;
						pObj->SetEffStatus(MMASK_LVDU, nTime, nPoisonDamage);

						PkgPlayerSetEffectAck ack;
						ack.uTargetId = pObj->GetID();
						ack.dwMgcID = MMASK_LVDU;
						ack.bShow = true;
						ack.dwTime = nTime;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ack;
						pObj->GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
					}
				}
			}
		}
	}
	else if(pMgc->wMgcID == MEFF_FIREDRAGON_FIREWALL)
	{
		if(NULL == _pMgc->pFire)
		{
			return;
		}
		if(_pMgc->pFire->GetType() != SOT_HERO)
		{
			return;
		}

		nPosX = pMgc->sPosX;
		nPosY = pMgc->sPoxY;
		GameObjectList xObjs;
		GetMappedObjects(nPosX, nPosY, 0, 0, xObjs, MAPPEDOBJECT_ALL_ALIVE);
		GameObjectList::const_iterator objbegiter = xObjs.begin();
		GameObjectList::const_iterator objenditer  = xObjs.end();

		for(objbegiter; objbegiter != objenditer; ++objbegiter)
		{
			GameObject* pObj = (GameObject*)*objbegiter;
			int nDCDelta = pMgc->wMaxDC - pMgc->wMinDC;
			if(nDCDelta == 0)
			{
				nDCDelta = 1;
			}
			nDamage = rand() % nDCDelta + pMgc->wMinDC;

			if(nDamage > 0)
			{
				pObj->ReceiveDamage(_pMgc->pFire, true, nDamage, 200);
			}
		}
	}
	else if(pMgc->wMgcID == MEFF_DRAGONBLUSTER)
	{
		if(NULL == _pMgc->pFire)
		{
			return;
		}
		if(_pMgc->pFire->GetType() != SOT_HERO)
		{
			return;
		}
		HeroObject* pHero = (HeroObject*)_pMgc->pFire;
		const UserMagic* pUserMagic = pHero->GetUserMagic(MEFF_DRAGONBLUSTER);
		if(NULL == pUserMagic ||
			0 == pUserMagic->bLevel)
		{
			return;
		}

		nPosX = pMgc->sPosX;
		nPosY = pMgc->sPoxY;
		GameObjectList xObjs;
		GetMappedObjects(nPosX, nPosY, 0, 0, xObjs, MAPPEDOBJECT_ALL_ALIVE);
		GameObjectList::const_iterator objbegiter = xObjs.begin();
		GameObjectList::const_iterator objenditer  = xObjs.end();

		for(objbegiter; objbegiter != objenditer; ++objbegiter)
		{
			GameObject* pObj = (GameObject*)*objbegiter;
			nDamage = pHero->GetMagicDamageNoDefence(pUserMagic);
			ReceiveDamageInfo info = {0};
			info.nMagicId = MEFF_DRAGONBLUSTER;
			info.bMagicLevel = pUserMagic->bLevel;

			if(pObj->GetType() == SOT_MONSTER)
			{
				if(static_cast<MonsterObject*>(pObj)->CanDefFire())
				{
					nDamage /= 1.3;
				}
				if(static_cast<MonsterObject*>(pObj)->IsFrozen())
				{
					nDamage *= 1.5f;
				}
			}

			if(nDamage > 0)
			{
				pObj->ReceiveDamage(pHero, true, nDamage, 500, &info);
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::HandleStaticMagic()
{
	RECORD_FUNCNAME_WORLD;

	if(m_xStaticMagic.empty())
	{
		return;
	}
	DWORD dwCurTick = GetTickCount();
	StaticMagic* pMgc = NULL;
	int nDamage = 0;

	size_t nSize = m_xStaticMagic.size();
	STATICMAGICLIST::const_iterator begiter = m_xStaticMagic.begin();
	STATICMAGICLIST::const_iterator enditer = m_xStaticMagic.end();
	int nPosX = 0;
	int nPosY = 0;

	for(begiter; begiter != enditer;
		)
	{
		pMgc = (*begiter);
		if(dwCurTick > pMgc->dwExpire)
		{
			MapCellInfo* pCell = GetMapData(pMgc->sPosX, pMgc->sPoxY);
			bool bFind = false;
			if(pCell != NULL)
			{
				if(pCell->pCellObjects == NULL)
				{
					//pCell->pCellObjects = new CELLDATALIST;
					DEBUG_BREAK;
				}
				CELLDATALIST::const_iterator begciter = pCell->pCellObjects->begin();
				CELLDATALIST::const_iterator endciter = pCell->pCellObjects->end();
				CellData* pData = NULL;
				for(begciter; begciter != endciter;)
				{
					pData = *begciter;
					if(pData->bType == CELL_MAGIC)
					{
						StaticMagic* pGetMgc = (StaticMagic*)pData->pData;
						if(pGetMgc == pMgc)
						{
							//	Remove the node
							delete pData;
							pCell->pCellObjects->erase(begciter);
							bFind = true;
							break;
						}
					}
					++begciter;
				}

				if(!bFind)
				{
					DEBUG_BREAK;
				}
			}
			delete pMgc;
			begiter = m_xStaticMagic.erase(begiter);
		}
		else if(pMgc->dwEnableTime != 0 &&
			GetTickCount() < pMgc->dwEnableTime)
		{
			//	魔法设定了生效时间
			++begiter;
		}
		else
		{
			ParseStaticMagic(pMgc);

			++begiter;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::RemoveStaticMagicByFire(GameObject* _pFire)
{
	RECORD_FUNCNAME_WORLD;

	if(m_xStaticMagic.empty())
	{
		return;
	}
	StaticMagic* pMgc = NULL;

	size_t nSize = m_xStaticMagic.size();
	STATICMAGICLIST::const_iterator begiter = m_xStaticMagic.begin();
	STATICMAGICLIST::const_iterator enditer = m_xStaticMagic.end();

	for(begiter; begiter != enditer; )
	{
		pMgc = (*begiter);
		
		if(pMgc->pFire != NULL &&
			pMgc->pFire == _pFire)
		{
			pMgc->pFire = NULL;
			delete *begiter;
			begiter = m_xStaticMagic.erase(begiter);
		}
		else
		{
			++begiter;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::RestoreStaticMagicByFire(GameObject* _pFire)
{
	RECORD_FUNCNAME_WORLD;
	return;

	if(m_xStaticMagic.empty())
	{
		return;
	}
	StaticMagic* pMgc = NULL;

	size_t nSize = m_xStaticMagic.size();
	STATICMAGICLIST::const_iterator begiter = m_xStaticMagic.begin();
	STATICMAGICLIST::const_iterator enditer = m_xStaticMagic.end();

	for(begiter; begiter != enditer; ++begiter)
	{
		pMgc = (*begiter);

		if(pMgc->dwFire == _pFire->GetID())
		{
			pMgc->pFire = _pFire;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateDoorEvent(unsigned int _uMapID, unsigned int _ux, unsigned int _uy, unsigned int _nextx, unsigned int _nexty)
{
	RECORD_FUNCNAME_WORLD;

	MapCellInfo* pCell = GetMapData(_ux, _uy);
	if(pCell)
	{
		if(pCell->pCellObjects == NULL)
		{
			pCell->pCellObjects = new CELLDATALIST;
		}
		CellData* pCellData = new CellData;
		pCellData->bType = CELL_DOOR;
		pCellData->dwInfo = 0;
		DoorEvent* pDoorEvt = new DoorEvent;
		pCellData->pData = pDoorEvt;
		pDoorEvt->wMapID = _uMapID;
		pDoorEvt->wPosX = _nextx;
		pDoorEvt->wPoxY = _nexty;
		pDoorEvt->dwTime = 0;
		pCell->pCellObjects->push_back(pCellData);

		//m_xDoorEvts.push_back(pDoorEvt);
		m_xDoorPos.push_back(MAKELONG(_ux, _uy));
	}
}

void GameScene::CreateDoorEvent(unsigned int _uMapID, unsigned int _ux, unsigned int _uy, unsigned int _nextx, unsigned int _nexty, unsigned int _utime)
{
	RECORD_FUNCNAME_WORLD;

	MapCellInfo* pCell = GetMapData(_ux, _uy);
	if(pCell)
	{
		if(pCell->pCellObjects == NULL)
		{
			pCell->pCellObjects = new CELLDATALIST;
		}
		CellData* pCellData = new CellData;
		pCellData->bType = CELL_DOOR;
		pCellData->dwInfo = 0;
		DoorEvent* pDoorEvt = new DoorEvent;
		pCellData->pData = pDoorEvt;
		pDoorEvt->wMapID = _uMapID;
		pDoorEvt->wPosX = _nextx;
		pDoorEvt->wPoxY = _nexty;
		pDoorEvt->wOX = _ux;
		pDoorEvt->wOY = _uy;
		pDoorEvt->dwTime = GetTickCount() + _utime;
		pCell->pCellObjects->push_back(pCellData);

		//m_xDoorEvts.push_back(pDoorEvt);
		m_xDoorEvts.push_back(pDoorEvt);
		m_xDoorPos.push_back(MAKELONG(_ux, _uy));
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::CreateDoorEventWithNotification(unsigned int _uMapID, unsigned int _ux, unsigned int _uy, unsigned int _nextx, unsigned int _nexty, unsigned int _utime)
{
	CreateDoorEvent(_uMapID, _ux, _uy, _nextx, _nexty, _utime);

	PkgPlayerShowDoorAniAck dack;
	dack.wMgcID = MEFF_DOOR;
	dack.uTargetId = 0;
	dack.uUserId = _utime;
	dack.wPosX = _ux;
	dack.wPosY = _uy;
	//BroadcastPacket(dack);

	HeroObject* pObj = NULL;

	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		pObj = (HeroObject*)iter->second;

		if(pObj)
		{
			dack.uTargetId = pObj->GetID();

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << dack;

			pObj->SendPlayerBuffer(g_xThreadBuffer);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::UpdateDoorEvent()
{
	RECORD_FUNCNAME_WORLD;

	if(!m_xDoorEvts.empty())
	{
		DOOREVENTLIST::iterator begiter = m_xDoorEvts.begin();
		DWORD dwCurTick = GetTickCount();
		DoorEvent* pDoorEvt = NULL;

		for(begiter;
			begiter != m_xDoorEvts.end();
			)
		{
			pDoorEvt = *begiter;
			if(pDoorEvt->dwTime != 0)
			{
				if(dwCurTick > pDoorEvt->dwTime)
				{
					//	Remove it
					RemoveMappedObject(pDoorEvt->wOX, pDoorEvt->wOY, pDoorEvt);

					DOORPOSITIONLIST::iterator positer = m_xDoorPos.begin();
					for(positer;
						positer != m_xDoorPos.end();
						++positer)
					{
						if(*positer == MAKELONG(pDoorEvt->wOX, pDoorEvt->wOY))
						{
							m_xDoorPos.erase(positer);
							break;
						}
					}
					delete pDoorEvt;
					begiter = m_xDoorEvts.erase(begiter);
				}
				else
				{
					++begiter;
				}
			}
			else
			{
				++begiter;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::AllMonsterHPToFull()
{
	RECORD_FUNCNAME_WORLD;

	GameObject* pObj = NULL;

	if(m_bAutoReset)
	{
		for(std::map<DWORD, GameObject*>::const_iterator begiter = m_xNPCs.begin();
			begiter != m_xNPCs.end();
			++begiter)
		{
			pObj = begiter->second;
			if(pObj->GetType() == SOT_MONSTER &&
				pObj->GetUserData()->eGameState != OS_DEAD &&
				pObj->GetObject_HP() != 0 &&
				GameWorld::GetInstance().GetAutoReset())
			{
				MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
				if(pMonster->GetMaster() == NULL)
				{
					pMonster->SetObject_HP(pMonster->GetObject_MaxHP());
					pMonster->GetValidCheck()->SetHP(pMonster->GetObject_MaxHP());
					PkgPlayerUpdateAttribNtf ntf;
					ntf.uTargetId = pMonster->GetID();
					ntf.bType = UPDATE_HP;
					ntf.dwParam = pMonster->GetObject_HP();
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					BroadcastPacket(&g_xThreadBuffer);
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameScene::GetMapObjects(int _x, int _y, int _oft, GameObjectList& _xObjList)
{
	RECT rcAttackRage;
	rcAttackRage.left = _x;
	rcAttackRage.right = _x;
	rcAttackRage.top = rcAttackRage.bottom = _y;
	rcAttackRage.left -= _oft;
	rcAttackRage.right += _oft;
	rcAttackRage.top -= _oft;
	rcAttackRage.bottom += _oft;

	if(rcAttackRage.left < 0)
	{
		rcAttackRage.right = 0;
	}
	if(rcAttackRage.right >= m_stMapInfo.nCol)
	{
		rcAttackRage.right = m_stMapInfo.nCol - 1;
	}
	if(rcAttackRage.top < 0)
	{
		rcAttackRage.top = 0;
	}
	if(rcAttackRage.bottom >= m_stMapInfo.nRow)
	{
		rcAttackRage.bottom = m_stMapInfo.nRow - 1;
	}


	MapCellInfo* pCell = NULL;
	CELLDATALIST* pList = NULL;
	GameObject* pObj = NULL;

	for(int x = rcAttackRage.left; x <= rcAttackRage.right; ++x)
	{
		for(int y = rcAttackRage.top; y <= rcAttackRage.bottom; ++y)
		{
			pCell = GetMapData(x, y);

			if(pCell)
			{
				if(pCell->pCellObjects)
				{
					pList = pCell->pCellObjects;

					CELLDATALIST::const_iterator begiter = pList->begin();
					for(begiter;
						begiter != pList->end();
						++begiter)
					{
						//bReqStruck = true;
						//bMissed = false;

						if((*begiter)->bType == CELL_MOVEOBJECT)
						{
							pObj = (GameObject*)(*begiter)->pData;
							
							_xObjList.push_back(pObj);
						}
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int GameScene::GetMappedObjects(int _x, int _y, int _ofx, int _ofy, GameObjectList& _refList, int _nTypeFlag /* = MAPPEDOBJECT_ALL */)
{
	RECT rcAttackRage;
	rcAttackRage.left = _x;
	rcAttackRage.right = _x;
	rcAttackRage.top = rcAttackRage.bottom = _y;
	rcAttackRage.left -= _ofx;
	rcAttackRage.right += _ofx;
	rcAttackRage.top -= _ofy;
	rcAttackRage.bottom += _ofy;

	if(rcAttackRage.left < 0)
	{
		rcAttackRage.right = 0;
	}
	if(rcAttackRage.right >= m_stMapInfo.nCol)
	{
		rcAttackRage.right = m_stMapInfo.nCol - 1;
	}
	if(rcAttackRage.top < 0)
	{
		rcAttackRage.top = 0;
	}
	if(rcAttackRage.bottom >= m_stMapInfo.nRow)
	{
		rcAttackRage.bottom = m_stMapInfo.nRow - 1;
	}


	MapCellInfo* pCell = NULL;
	CELLDATALIST* pList = NULL;
	GameObject* pObj = NULL;
	int nCount = 0;
	bool bIgnoreDead = false;

	//	死亡的不放入链表
	if(TEST_FLAG_BOOL(_nTypeFlag, MAPPEDOBJECT_STATUE_ALIVE))
	{
		bIgnoreDead = true;
	}

	for(int x = rcAttackRage.left; x <= rcAttackRage.right; ++x)
	{
		for(int y = rcAttackRage.top; y <= rcAttackRage.bottom; ++y)
		{
			pCell = GetMapData(x, y);

			if(pCell)
			{
				if(pCell->pCellObjects)
				{
					pList = pCell->pCellObjects;

					CELLDATALIST::const_iterator begiter = pList->begin();
					for(begiter;
						begiter != pList->end();
						++begiter)
					{
						if((*begiter)->bType == CELL_MOVEOBJECT)
						{
							bool bNeedPush = false;

							pObj = (GameObject*)(*begiter)->pData;

							if(pObj->GetType() == SOT_HERO)
							{
								if(TEST_FLAG_BOOL(_nTypeFlag, MAPPEDOBJECT_PLAYER))
								{
									bNeedPush = true;
								}
							}
							else if(pObj->GetType() == SOT_MONSTER)
							{
								MonsterObject* pMonster = (MonsterObject*)pObj;

								if(pMonster->GetMaster() != NULL)
								{
									if(TEST_FLAG_BOOL(_nTypeFlag, MAPPEDOBJECT_SLAVE))
									{
										bNeedPush = true;
									}
								}
								else
								{
									if(TEST_FLAG_BOOL(_nTypeFlag, MAPPEDOBJECT_MONSTER))
									{
										bNeedPush = true;
									}
								}
							}

							if(bIgnoreDead)
							{
								if(pObj->GetUserData()->eGameState == OS_DEAD ||
									pObj->GetObject_HP() == 0)
								{
									bNeedPush = false;
								}
							}

							if(bNeedPush)
							{
								_refList.push_back(pObj);
								++nCount;
							}
						}
					}
				}
			}
		}
	}

	return nCount;
}

void GameScene::RegisterCallback(SceneEvent _eType, const char* _pszFuncName, int _nInterval)
{
	m_xSceneEventExecutor.AddSceneEvent(_eType, _pszFuncName, _nInterval);
}

void GameScene::ChallengeResetScene(int _nClgID)
{
	if(0 == GetPlayerSum())
	{
		DeleteAllItem();
		DeleteAllMonster();

		lua_getglobal(GetLuaState(), "clg_reset");
		if(lua_isfunction(GetLuaState(), -1))
		{
			int nRet = lua_pcall(GetLuaState(), 0, 0, 0);
			if(nRet != 0)
			{
				LOG(WARNING) << lua_tostring(GetLuaState(), -1);
				lua_pop(GetLuaState(), 1);
			}
		}

		m_nChallengeID = _nClgID;
	}
}

void GameScene::ChallengeSuccess(int _nItemID)
{
	HeroObject* pOneHero = NULL;
	HeroObject* pTeamLeader = NULL;

	if(0 != GetPlayerSum())
	{
		HeroObject* pHero = NULL;
		for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
			iter != m_xPlayers.end();
			++iter)
		{
			pHero = (HeroObject*)iter->second;
			pOneHero = pHero;
			
			if(pHero->IsTeamLeader())
			{
				pTeamLeader = pHero;
			}
		}
	}

	if(NULL == pTeamLeader)
	{
		pTeamLeader = pOneHero;
	}

	if(NULL == pTeamLeader)
	{
		return;
	}

	pTeamLeader->AddItemNoBind(_nItemID);

	char szMsg[MAX_PATH] = {0};
	char szName[20] = {0};
	ItemAttrib item;
	if(GetRecordInItemTable(_nItemID, &item))
	{
		ObjectValid::GetItemName(&pTeamLeader->GetUserData()->stAttrib, szName);
		sprintf(szMsg, "由[%s]领导的队伍挑战成功，成功获得[%s]",
			szName, item.name);
		//GameSceneManager::GetInstance()->SendSystemMessageAllScene(szMsg);
		GameSceneManager::GetInstance()->SendSystemNotifyAllScene(szMsg);
	}
}

void GameScene::AllHeroFlyToHome()
{
	//	Players
	HeroObject* pHero = NULL;
	for(std::map<DWORD, GameObject*>::const_iterator iter = m_xPlayers.begin();
		iter != m_xPlayers.end();
		++iter)
	{
		pHero = (HeroObject*)iter->second;
		pHero->FlyToHome();
	}
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* Class GameSceneManager
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
//	static variable
static GameSceneManager* s_pGameSceneManager = NULL;
//////////////////////////////////////////////////////////////////////////
GameSceneManager::GameSceneManager()
{
	for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
	{
		m_pScenes[i] = NULL;
	}
	m_dwLoadedMap = 0;

	//GetRunMapData(m_xRunMap);
	if(!GetRunMapDataEx(m_xRunMapData))
	{
		LOG(ERROR) << "Can't load the run map information.";
	}
	if(!GetInstanceMapDataEx(m_xInsMapData))
	{
		LOG(ERROR) << "Can't load the instance map information.";
	}

	m_dwLastNotifyPlayerCount = 0;
}

GameSceneManager::~GameSceneManager()
{
	ReleaseAllScene();
}

GameSceneManager* GameSceneManager::GetInstance()
{
	if(NULL == s_pGameSceneManager)
	{
		s_pGameSceneManager = new GameSceneManager;
	}
	return s_pGameSceneManager;
}

void GameSceneManager::DestroyInstance()
{
	if(NULL != s_pGameSceneManager)
	{
		delete s_pGameSceneManager;
		s_pGameSceneManager = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
bool GameSceneManager::CreateAllScene()
{
	/*char szCfgFile[MAX_PATH];
	GetModuleFileName(NULL, szCfgFile, sizeof(szCfgFile));
	PathRemoveFileSpec(szCfgFile);
	strcat(szCfgFile, "\\cfg.ini");*/
	if(m_xRunMapData.empty())
	{
		return false;
	}

	DWORD dwTotal = 0;
	bool bSuc = true;
	//dwTotal = GetPrivateProfileInt("LOADMAP", "TOTAL", 0, szCfgFile);
	dwTotal = m_xRunMapData.size();

	if(dwTotal > 0)
	{
		for(DWORD i = 0; i < dwTotal; ++i)
		{
			m_pScenes[i] = new GameScene;
			bSuc = bSuc && m_pScenes[i]->Initialize(i);
			if(!bSuc)
			{
				break;
			}
			++m_dwLoadedMap;
		}
	}
	LOG(INFO) << "载入场景数量[" << m_dwLoadedMap << "]个";

	//	读入地图名配置
	char szPath[MAX_PATH];
	sprintf(szPath, "%s\\Config\\map.ini",
		GetRootPath());
	m_xIniMapName.LoadFile(szPath);

	return bSuc;
}
//////////////////////////////////////////////////////////////////////////
void GameSceneManager::ReleaseAllScene()
{
	for(DWORD i = 0; i < m_dwLoadedMap; ++i)
	{
		m_pScenes[i]->Release();
		m_pScenes[i] = NULL;
	}
	m_dwLoadedMap = 0;
}
//////////////////////////////////////////////////////////////////////////
bool GameSceneManager::InsertPlayer(GameObject* _pObj)
{
	DWORD wMapID = _pObj->GetMapID();
	if(wMapID >= MAX_SCENE_NUMBER)
	{
		return false;
	}
	if(m_pScenes[wMapID])
	{
		return m_pScenes[wMapID]->InsertPlayer(_pObj);
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GameSceneManager::InsertNPC(GameObject* _pObj)
{
	DWORD wMapID = _pObj->GetMapID();
	if(wMapID >= MAX_SCENE_NUMBER)
	{
		return false;
	}
	if(m_pScenes[wMapID])
	{
		return m_pScenes[wMapID]->InsertNPC(_pObj);
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GameSceneManager::RemovePlayer(DWORD _dwID)
{
	for(DWORD i = 0; i < m_dwLoadedMap; ++i)
	{
		if(m_pScenes[i])
		{
			if(m_pScenes[i]->RemovePlayer(_dwID))
			{
				return true;
			}
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
GameObject* GameSceneManager::GetPlayer(WORD _wMapID, DWORD _dwID)
{
	if(_wMapID >= MAX_SCENE_NUMBER)
	{
		return NULL;
	}
	if(m_pScenes[_wMapID])
	{
		std::map<DWORD, GameObject*>::iterator iter = m_pScenes[_wMapID]->m_xPlayers.find(_dwID);
		if(iter != m_pScenes[_wMapID]->m_xPlayers.end())
		{
			return iter->second;
		}
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
void GameSceneManager::GetPlayerByUid(int _nUid, GameObjectList& _refList)
{
	for(DWORD i = 0; i < m_dwLoadedMap; ++i)
	{
		if(i >= MAX_SCENE_NUMBER)
		{
			break;
		}

		GameScene* pScene = m_pScenes[i];

		if(pScene)
		{
			std::map<DWORD, GameObject*>& refPlayers = pScene->m_xPlayers;
			std::map<DWORD, GameObject*>::iterator iter = refPlayers.begin();

			for(iter;
				iter != refPlayers.end();
				++iter)
			{
				GameObject* pObj = iter->second;
				if(NULL == pObj)
				{
					continue;
				}
				if(pObj->GetType() != SOT_HERO)
				{
					continue;
				}

				HeroObject* pHero = (HeroObject*)pObj;
				if(pHero->GetUID() == _nUid)
				{
					_refList.push_back(pHero);
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
GameObject* GameSceneManager::GetPlayer(DWORD _dwID)
{
	GameObject* pObj = NULL;

	for(DWORD i = 0; i < m_dwLoadedMap; ++i)
	{
		pObj = GetPlayer(i, _dwID);
		if(pObj)
		{
			return pObj;
		}
	}
	return pObj;
}
//////////////////////////////////////////////////////////////////////////
void GameSceneManager::ReloadScript()
{
	for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
	{
		if(m_pScenes[i])
		{
			if(m_pScenes[i]->ReloadScript())
			{
				LOG(INFO) << "重新载入脚本[scene" << m_pScenes[i]->GetMapID() << ".lua]成功";
			}
		}
		else
		{
			break;
		}
	}

	if(!m_xInsMaps.empty())
	{
		INSTANCEMAPLIST::const_iterator begIter = m_xInsMaps.begin();
		INSTANCEMAPLIST::const_iterator endIter = m_xInsMaps.end();

		for(begIter;
			begIter != endIter;
			++begIter)
		{
			if((*begIter)->ReloadScript())
			{
				LOG(INFO) << "重新载入脚本[scene" << (*begIter)->GetMapID() << ".lua]成功";
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void GameSceneManager::Update(DWORD _dwTick)
{
	for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
	{
		if(m_pScenes[i])
		{
			m_pScenes[i]->Update(_dwTick);
		}
		else
		{
			break;
		}
	}

	for(int i = 0; i < MAX_SCENE_NUMBER; ++i)
	{
		if(m_pScenes[i])
		{
			m_pScenes[i]->ProcessWaitInsert();
			m_pScenes[i]->ProcessWaitRemove();
			m_pScenes[i]->ProcessWaitDelete();
		}
		else
		{
			break;
		}
	}

	if(!m_xInsMaps.empty())
	{
		INSTANCEMAPLIST::const_iterator begIter = m_xInsMaps.begin();
		INSTANCEMAPLIST::const_iterator endIter = m_xInsMaps.end();

		for(begIter;
			begIter != endIter;
			++begIter)
		{
			(*begIter)->Update(_dwTick);
		}
	}

	if(GetTickCount() - m_dwLastNotifyPlayerCount > UPDATEPLAYERCOUNT_INTERVAL)
	{
		m_dwLastNotifyPlayerCount = GetTickCount();

		int nCounter = CountPlayer();
		int nMonsterCounter = CountMonster();

		PostMessage(g_hServerDlg, WM_PLAYERCOUNT, nCounter, nMonsterCounter);
	}
}
//////////////////////////////////////////////////////////////////////////
GameInstanceScene* GameSceneManager::GetFreeInstanceScene(int _id)
{
	if(_id < 100)
	{
		return NULL;
	}

	GameInstanceScene* pInsScene = NULL;

	if(!m_xInsMaps.empty())
	{
		INSTANCEMAPLIST::const_iterator begIter = m_xInsMaps.begin();
		INSTANCEMAPLIST::const_iterator endIter = m_xInsMaps.end();

		for(begIter;
			begIter != endIter;
			++begIter)
		{
			pInsScene = *begIter;
			if(pInsScene->GetMapID() == _id &&
				pInsScene->IsFree())
			{
				return pInsScene;
			}
		}
	}

	if(pInsScene == NULL)
	{
		pInsScene = new GameInstanceScene;
		if(pInsScene->Initialize(_id))
		{
			m_xInsMaps.push_back(pInsScene);
		}
		else
		{
			LOG(ERROR) << "Initialize failed when loading the instance map[" << _id << "]";
			SAFE_DELETE(pInsScene);
		}
	}

	return pInsScene;
}

const char* GameSceneManager::GetMapChName(int _id)
{
	const char* pszRunmapName = GetRunMap(_id);
	if(NULL == pszRunmapName)
	{
		return "未知地图";
	}
	else
	{
		const char* pszChMapName = m_xIniMapName.GetValue("MapNameInfo", pszRunmapName);
		if(NULL == pszChMapName)
		{
			return "未知地图";
		}
		if(strlen(pszChMapName) == 0)
		{
			return "未知地图";
		}
		return pszChMapName;
	}
}