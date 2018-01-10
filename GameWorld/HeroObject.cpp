#include "../stdafx.h"
#include "ObjectEngine.h"
#include "GameWorld.h"
#include "GameSceneManager.h"
#include "MonsterObject.h"
#include "DBThread.h"
#include "ObjectValid.h"
#include "TeammateControl.h"
#include <math.h>
#include <zlib.h>
#include <algorithm>
#include "ObjectStatus.h"
#include "ExceptionHandler.h"
#include "../CMainServer/CMainServer.h"
#include "../../CommonModule/SettingLoader.h"
#include "../../CommonModule/StatusDefine.h"
#include "../../CommonModule/ItemGrowUtils.h"
#include "../../CommonModule/HideAttribHelper.h"
#include "../../CommonModule/PotentialAttribHelper.h"
#include "../../CommonModule/loginsvr.pb.h"
#include "../../CommonModule/ProtoType.h"
#include "../runarg.h"
#include "GlobalAllocRecord.h"
//////////////////////////////////////////////////////////////////////////
static const int s_nMoveOft[] =
{
	0,-1,
	1,-1,
	1,0,
	1,1,
	0,1,
	-1,1,
	-1,0,
	-1,-1
};
//////////////////////////////////////////////////////////////////////////
extern HWND g_hServerDlg;

PkgPlayerInteractiveDialogContentNot HeroObject::s_xInteractiveDialogItemPkg;
//////////////////////////////////////////////////////////////////////////
HeroObject::HeroObject(DWORD _dwID) : m_xMagics(USER_MAGIC_NUM),
	/*,m_xSendBuffer(OBJECT_BUFFER_SIZE)*/
	m_xBag(HERO_BAG_SIZE_CUR)
{
	m_eType = SOT_HERO;
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		//m_xBag[i].id = INVALID_ITEM_ID;
		//m_xBag[i].type = ITEM_NO;
		//m_xBag[i].tag = 0;
		ZeroMemory(&m_xBag[i], sizeof(ItemAttrib));
	}
	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		//m_stEquip[i].id = 0;
		//m_stEquip[i].type = ITEM_NO;
		//m_stEquip[i].tag = 0;
		ZeroMemory(&m_stEquip[i], sizeof(ItemAttrib));
	}
	for(int i = 0; i < HP_SUPPLY_NUMBER + MP_SUPPLY_NUMBER; ++i)
	{
		m_dwSupply[i] = 0;
	}
	ZeroMemory(m_dwHumEffTime, sizeof(m_dwHumEffTime));
	m_dwDefence = 0;

	m_dwLastIncHPTime = m_dwLastIncMPTime = 0;
	m_bClientLoaded = false;
	m_nAssistItemSum = HERO_ASSISTBAG_SIZE_CUR;
	m_dwIndex = _dwID;
	m_dwHumEffFlag = 0;
	m_nMoney = 0;

	m_stData.bJob = 0;
	m_dwLastCityMap = 0;
	ZeroMemory(m_pSlaves, sizeof(m_pSlaves));
	

	ZeroMemory(m_stStore, sizeof(m_stStore));
	ZeroMemory(m_stBigStore, sizeof(m_stBigStore));
	m_dwLastSpellTime = 0;

	m_dwTimeOut = 0;
	m_bIsNewHero = false;
	m_dwLastReviveTime = 0;
	m_dwLastCheckTime = GetTickCount();
	m_uVersion = 0;
	m_nGainedExp = 0;

	m_dwLastUseMoveRingTime = 0;

	m_nTeamID = 0;
	m_dwDoubleDropExpireTime = 0;
	m_dwReviveRingUseTime = 0;

	m_dwLSIndex = 0;

	m_bGmHide = false;

	m_dwLastPrivateChatTime = m_dwLastTeamChatTime = 0;

	m_bUnUse = false;

	m_dwLastReqOffSellItems = 0;
	m_dwLastReqOffTakeBack = 0;

	m_dwJinGangExpireTime = 0;

	m_dwLoginTimeTick = 0;
	m_bCheckSoldItems = false;
	m_cAutoSaveCounter = 0;
	m_nDonateMoney = 0;
	m_nDonateLeft = 0;

	m_dwServerNetDelaySec = 0xffffffff;
	m_dwLastCheckServerNetDelay = 0;
	m_nServerNetDelaySeq = 0;

	//	initialize extend attribute
	ZeroMemory(&m_stExtAttrib, sizeof(m_stExtAttrib));

	m_bLastAttackCritical = false;

	m_nTimeLimitID = 0;
	m_nEnterTimeLimitMapTime = 0;

	m_bNeedTransAni = false;
	m_byteDifficultyLevel = 0;
	m_dwHPRecoverInterval = 15 * 1000;

	m_ePkType = kHeroPkType_None;
	//m_ePkType = kHeroPkType_All;

	m_bSmallQuit = false;

	m_nExtraSuitType = 0;

	//	NOTE : 这里注意下，没有收到大仓库的数据后，假如人物退出，会造成大仓库数据丢失
	m_bBigStoreReceived = false;

	m_nInvalidMagicAttackTimes = 0;

	m_bPushLSLogoutEvent = true;
	m_bLSLoginPushed = false;
}

HeroObject::~HeroObject()
{

}

//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* virtual void DoWork(unsigned int _dwTick)
/************************************************************************/
void HeroObject::DoWork(unsigned int _dwTick)
{
	RECORD_FUNCNAME_WORLD;

	m_dwTotalExeTime += _dwTick;
	m_dwCurrentTime = GetTickCount();
	//	process delay actions
	ProcessDelayAction();
	//	process buffered message
	DoMsgQueue(_dwTick);
	//	execute AI
	DoAction(_dwTick);
	//	send all buffered packets that generated in DoPacket and DoAction process
	//DoSendDelayedPackets();
	if(m_dwTimeOut > TIME_KICK)
	{
		m_dwTimeOut = 0;
		//PkgPlayerUserDataReq req;
		//DoPacket(req);
		//GameWorld::GetInstance().Stop();
		GameWorld::GetInstance().Stop(STOP_HEROSPDERR);

		//FlyToPrison();
#ifdef _DEBUG
		LOG(ERROR) << "SPEED ERROR";
#endif
	}

	if(GetObject_HP() == 0 &&
		GetUserData()->eGameState != OS_DEAD)
	{
		GameWorld::GetInstance().Stop(STOP_HPZERONOTDEAD);
		FlyToPrison();
#ifdef _DEBUG
		LOG(ERROR) << "HP zero";
#endif
	}

	if(_dwTick - m_dwLastCheckTime > 30 * 1000)
	{
		m_dwLastCheckTime = _dwTick;

		ItemAttrib preAttrib = m_stData.stAttrib;
		if(false == DBThread::GetInstance()->VerifyHeroItemSimple(this))
		{
			//GameWorld::GetInstance().Stop();
			GameWorld::GetInstance().Stop(STOP_HEROATTRIBERR);
			FlyToPrison();
#ifdef _DEBUG
			LOG(ERROR) << "Item err";
#endif
		}

		if(!m_pValid->TestValid())
		{
			FlyToPrison();
		}

		if(GetObject_HP() > GetObject_MaxHP())
		{
			PostMessage(g_hServerDlg, WM_CLOSECONNECTION, GetUserIndex(), 0);
		}

		//	Check standing on the door
		//if(GetUserData()->eGameState == OS_STAND)
		{
			if(_dwTick - m_dwLastWalkTime > 20 * 1000 &&
				_dwTick - m_dwLastRunTime > 20 * 1000 &&
				m_bClientLoaded)
			{
				if(GetLocateScene()->GetDoorEvent(GetUserData()->wCoordX, GetUserData()->wCoordY) != NULL)
				{
					bool bKick = true;
					if(SettingLoader::GetInstance()->GetIntValue("AUTOKICKDOOROBSTACLE") == 0)
					{
						bKick = false;
					}
					if(bKick)
					{
						PostMessage(g_hServerDlg, WM_CLOSECONNECTION, GetUserIndex(), 0);
					}
				}
			}
		}

		{
			if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
			{
				if(_dwTick - m_dwLastWalkTime > 120 * 1000 &&
					_dwTick - m_dwLastRunTime > 120 * 1000 &&
					m_bClientLoaded)
				{
					GameObject* pSlave = NULL;

					for(int i = 0; i < 3; ++i)
					{
						pSlave = GetSlave(i);
						if(pSlave &&
							pSlave->GetUserData()->eGameState != OS_STOP)
						{
							pSlave->GetUserData()->eGameState = OS_STOP;
						}
					}
				}
			}
		}
	}

	//	update net delay
	CheckServerNetDelay();

	//	check time limit
	GameScene* pLocateScene = GetLocateScene();
	if(pLocateScene->GetTimeLimitID() == 0 ||
		pLocateScene->GetTimeLimitID() != m_nTimeLimitID)
	{
		m_nEnterTimeLimitMapTime = 0;
		m_nTimeLimitID = 0;
	}
	else
	{
		//	相同的限制ID
		if(GetTickCount() - m_nEnterTimeLimitMapTime > pLocateScene->GetTimeLimitSec() * 1000)
		{
			//	回城
			m_nEnterTimeLimitMapTime = 0;
			m_nTimeLimitID = 0;
			FlyToHome();

			g_xConsole.CPrint("Time limit scene timeout: limit id:%d limitsec:%d entertime:%d nowtime:%d",
				pLocateScene->GetTimeLimitID(),
				pLocateScene->GetTimeLimitSec(),
				m_nEnterTimeLimitMapTime,
				GetTickCount());
		}
	}
}

/************************************************************************/
/* virtual void DoAction(unsigned int _dwTick)
/************************************************************************/
void HeroObject::DoAction(unsigned int _dwTick)
{
	RECORD_FUNCNAME_WORLD;
	//	Check drug state
	DWORD dwCurTime = GetTickCount();
	UpdateStatus(dwCurTime);

	//	Offline?
	if(GetTickCount() - GetLastRecvDataTime() > OFFLINE_TIME)
	{
		if(CMainServer::GetInstance()->GetServerMode() == GM_NORMAL)
		{
			if(m_bClientLoaded)
			{
				//	Close connection
				char szName[20];
				ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
				LOG(INFO) << "Connection cut [" << GetID() << "] " << szName;
				PostMessage(g_hServerDlg, WM_CLOSECONNECTION, GetUserIndex(), 0);

				//	Don't show the msg again
				SetLastRecvDataTime(GetTickCount());
			}
			else
			{
				SetLastRecvDataTime(GetTickCount());
			}
		}
		else
		{
			//	战网不踢人
		}
	}

	if(!m_bCheckSoldItems &&
		m_bClientLoaded &&
		GetTickCount() - m_dwLoginTimeTick > 5000)
	{
		if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN &&
			GameWorld::GetInstance().IsEnableOfflineSell())
		{
			SoldItemList xSoldItems;
			OfflineSellSystem::GetInstance()->QuerySoldItem(GetUID(), xSoldItems);

			if(!xSoldItems.empty())
			{
				SendSystemMessage("您有售出的物品，请点击[查看已售]查看详情");
			}

			ExpireItemList xExpireItems;
			OfflineSellSystem::GetInstance()->QueryExpireItem(GetUID(), xExpireItems);

			if(!xExpireItems.empty()){
				SendSystemMessage("您有已过期的物品，请点击[查看已售]进行收回");
			}
		}

		m_bCheckSoldItems = true;
	}
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::UpdateStatus(DWORD _dwCurTick)
{
	RECORD_FUNCNAME_WORLD;
	//	Check status
	/*if(GetTickCount() - m_dwLastCheckValidTime > 10 * 1000)
	{
		m_dwLastCheckValidTime = GetTickCount();
		if(!m_pValid->TestValid())
		{
			DEBUG_BREAK;
			//LOG(INFO) << "CHEATER";
			//PostQuitMessage(0);
			//_endthreadex(10);
			GameWorld::GetInstance().Stop(10);
			FlyToPrison();
		}
	}*/

	__super::UpdateStatus(_dwCurTick);

	PkgPlayerEnableSkillNot ppesn;
	ppesn.uTargetId = GetID();

	//	reset sheild
	if(TEST_FLAG(m_dwHumEffFlag, MMASK_SHIELD))
	{
		if(_dwCurTick >= m_dwHumEffTime[MMASK_SHIELD_INDEX])
		{
			ProcessSheild(99999);
		}
	}

	//	reset energy shield
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_ENERGYSHIELD))
	{
		if(_dwCurTick >= m_dwHumEffTime[MMASK_ENERGYSHIELD_INDEX])
		{
			RESET_FLAG(m_dwHumEffFlag, MMASK_ENERGYSHIELD);
			m_dwHumEffTime[MMASK_ENERGYSHIELD_INDEX] = 0;
		}
	}

	//	Reset fire sword
	if(TEST_FLAG(m_dwHumEffFlag, MMASK_LIEHUO))
	{
		if(_dwCurTick - m_dwHumEffTime[MMASK_LIEHUO_INDEX] > 6000)
		{
			RESET_FLAG(m_dwHumEffFlag, MMASK_LIEHUO);
			SendSystemMessage("烈火精灵消失...");
			ppesn.nSkillId = MEFF_LIEHUO;
			ppesn.bEnabled = false;
			SendPacket(ppesn);
		}
	}

	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_SLIEHUO))
	{
		if(_dwCurTick - m_dwHumEffTime[MMASK_SLIEHUO_INDEX] > 10000)
		{
			RESET_FLAG(m_dwHumEffFlag, MMASK_SLIEHUO);
			SendSystemMessage("炙炎之力散失...");
			ppesn.nSkillId = MEFF_SLIEHUO;
			ppesn.bEnabled = false;
			SendPacket(ppesn);
		}
	}

	if(m_dwDoubleDropExpireTime != 0)
	{
		if(_dwCurTick > m_dwDoubleDropExpireTime)
		{
			m_dwDoubleDropExpireTime = 0;
			SendSystemMessage("双倍爆率已过期");
		}
	}

	//	金刚不坏
	if(m_dwJinGangExpireTime != 0)
	{
		if(GetTickCount() > m_dwJinGangExpireTime)
		{
			m_dwJinGangExpireTime = 0;
		}
	}
}

/************************************************************************/
/* virtual void DoSendDelayedPackets()
/************************************************************************/
void HeroObject::DoSendDelayedPackets()
{
	RECORD_FUNCNAME_WORLD;
// 	while(m_xSendBuffer.GetLength())
// 	{
// 		//	auto write a 4 bytes net based number of length to send
// 		DWORD dwLength = *(unsigned int*)m_xSendBuffer.GetHead();
// 		SendMsgToUser(m_xSendBuffer.GetHead() + sizeof(unsigned int),
// 			dwLength - 4);
// 		m_xSendBuffer.SetHeadOffset(dwLength);
// 	}
// 	m_xSendBuffer.Reset();
}

/************************************************************************/
/* unsigned int PushMessage(const void* _pData, unsigned int _uLen)
/************************************************************************/
unsigned int HeroObject::PushMessage(const void* _pData, unsigned int _uLen)
{
	//	merge the data because we lost the header's length
	unsigned int uWrite = 0;

	Lock();
	m_xMsgQueue << _uLen + sizeof(unsigned int);
	uWrite = m_xMsgQueue.Write(_pData, _uLen);
	if(uWrite == 0)
	{
		LOG(FATAL) << "Fatal error has occured on writing data to receive buffer";
	}
	Unlock();

	return uWrite;
}

unsigned int HeroObject::PushMessage(ByteBuffer* _xBuf)
{
	//	merge the data because we lost the header's length
	unsigned int uWrite = 0;

	Lock();
	try
	{
		uWrite = m_xMsgQueue.Write(_xBuf->GetBuffer(), _xBuf->GetLength());
	}
	catch (std::exception* e)
	{
		LOG(ERROR) << "Fatal error has occured on writing data to receive buffer, Player:" << GetName();
		CMainServer::GetInstance()->ForceCloseConnection(GetUserIndex());
	}
	if(uWrite == 0)
	{
		LOG(ERROR) << "Fatal error has occured on writing data to receive buffer";
	}
	Unlock();

	return uWrite;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::ReceiveDamage(GameObject* _pAttacker, bool _bMgcAtk, int _oriDC /* = 0 */, int _nDelay /* = 500 */, const ReceiveDamageInfo* _pInfo /* = NULL */)
{
	RECORD_FUNCNAME_WORLD;
	//	检测是否能处理伤害
	if(NULL == _pAttacker)
	{
		return 0;
	}
	if(_pAttacker == this)
	{
		return 0;
	}
	if(_pAttacker->GetType() == SOT_HERO)
	{
		HeroObject* pHero = (HeroObject*)_pAttacker;
		//	被其它玩家攻击
		if(GetLocateScene()->GetMapPkType() == kMapPkType_None)
		{
			return 0;
		}
		else
		{
			if(pHero->GetPkType() == kHeroPkType_None)
			{
				return 0;
			}
			else if(pHero->GetPkType() == kHeroPkType_All)
			{

			}
			else if(pHero->GetPkType() == kHeroPkType_Team)
			{
				if(pHero->GetTeamID() == GetTeamID() &&
					0 != GetTeamID())
				{
					//	同组
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
	}
	else if(_pAttacker->GetType() == SOT_MONSTER)
	{
		//	被怪物攻击
		MonsterObject* pMonster = (MonsterObject*)_pAttacker;
		HeroObject* pMaster = (HeroObject*)pMonster->GetMaster();

		if(pMaster == this)
		{
			return 0;
		}

		if(NULL == pMaster)
		{
			//	被普通怪物攻击
		}
		else
		{
			//	被宝宝攻击
			if(GetLocateScene()->GetMapPkType() == kMapPkType_None)
			{
				return 0;
			}
			else
			{
				if(pMaster->GetPkType() == kHeroPkType_None)
				{
					return 0;
				}
				else if(pMaster->GetPkType() == kHeroPkType_All)
				{

				}
				else if(pMaster->GetPkType() == kHeroPkType_Team)
				{
					if(pMaster->GetTeamID() == GetTeamID() &&
						0 != GetTeamID())
					{
						//	同组
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
		}
	}

	//	ignore damage
	if(_bMgcAtk)
	{
		int nHideValue = m_xStates.GetStateValue(EAID_IGNOREMCDAM) + m_xStates.GetStateValue(EAID_MAGICHIDE);
		if(0 != nHideValue)
		{
			if(nHideValue > rand() % 100)
			{
				return 0;
			}
		}
	}
	else
	{
		int nHideValue = m_xStates.GetStateValue(EAID_IGNOREDCDAM) + m_xStates.GetStateValue(EAID_DCHIDE);
		if(0 != nHideValue)
		{
			if(nHideValue > rand() % 100)
			{
				return 0;
			}
		}
	}

	int nAC = 0;
	int nDC = _pAttacker->GetRandomAbility(AT_DC);

	if(_oriDC != 0)
	{
		nDC = _oriDC;
	}

	if(m_stData.eGameState == OS_DEAD)
	{
		return 0;
	}

	if(!_bMgcAtk)
	{
		if(_pAttacker->IsMissed(this))
		{
			return 0;
		}
	}

	if(_bMgcAtk)
	{
		nAC = GetRandomAbility(AT_MAC);
		nAC += m_xStates.GetStateValue(EAID_MAGICDEFENSE);
	}
	else
	{
		nAC = GetRandomAbility(AT_AC);
		nAC += m_xStates.GetStateValue(EAID_DCDEFENSE);
	}

	if(NULL != _pInfo)
	{
		if(_pInfo->nMagicId == MEFF_DRAGONBLUSTER &&
			0 != _pInfo->bMagicLevel)
		{
			nAC /= _pInfo->bMagicLevel;
		}
		else if(_pInfo->nMagicId == MEFF_FIREDRAGON_FIREWALL)
		{
			nAC /= 2;
		}
	}

	if(_pAttacker->GetType() == SOT_MONSTER)
	{
		if(TEST_FLAG_BOOL(_pAttacker->GetObject_Weight(), WEIGHT_MASK_IGNOREAC))
		{
			nAC = 0;
		}
	}

	if(NULL != _pInfo &&
		_pInfo->bIgnoreAC)
	{
		nAC = 0;
	}

	int nDamage = nDC - nAC;

	//	If no damage, return
	if(nDamage <= 0)
	{
		return 0;
	}

	//	Set slave target
	SetSlaveTarget(_pAttacker);

	//int nPreHP = m_stData.stAttrib.HP;
	//
#define DEFENCE_MC_MULTI	1.0f
#define DEFENCE_SC_MULTI	1.5f
#define DEFENCE_DC_MULTI	3.0f;
	if(m_xStates.GetHurtTransAddition() != 0)
	{
		//int nMpDamage = nDamage * DEFENCE_MULTI;
		int nMpDamage = nDamage;
		if(GetHeroJob() == 0)
		{
			nMpDamage *= DEFENCE_DC_MULTI;
		}
		else if(GetHeroJob() == 1)
		{
			nMpDamage *= DEFENCE_MC_MULTI;
		}
		else if(GetHeroJob() == 2)
		{
			nMpDamage *= DEFENCE_SC_MULTI;
		}

		if(GetObject_MP() >= nMpDamage)
		{
			DecMP(nMpDamage);
			m_pValid->DecMP(nMpDamage);
			PkgPlayerUpdateAttribNtf ntf;
			ntf.bType = UPDATE_MP;
			ntf.dwParam = GetObject_MP();
			ntf.uTargetId = GetID();
			SendPacket(ntf);

			//
			nDamage = 1;
		}
		else
		{
			nMpDamage -= GetObject_MP();
			SetObject_MP(0);
			m_pValid->SetMP(0);
			PkgPlayerUpdateAttribNtf ntf;
			ntf.bType = UPDATE_MP;
			ntf.dwParam = 0;
			ntf.uTargetId = GetID();
			SendPacket(ntf);

			if(nMpDamage > 0)
			{
				//nMpDamage /= DEFENCE_MULTI;
				if(GetHeroJob() == 0)
				{
					nMpDamage /= DEFENCE_DC_MULTI;
				}
				else if(GetHeroJob() == 1)
				{
					nMpDamage /= DEFENCE_MC_MULTI;
				}
				else if(GetHeroJob() == 2)
				{
					nMpDamage /= DEFENCE_SC_MULTI;
				}

				if(nMpDamage <= 0)
				{
					//return;
					nDamage = 1;
				}
				else
				{
					nDamage = nMpDamage;
				}
			}
		}
	}

	//	First detect if the hero have magic shield effect
	if(NULL != _pInfo &&
		_pInfo->bIgnoreAC)
	{

	}
	else
	{
		if(IsEffectExist(MEFF_SHIELD))
		{
			int nDefence = GetSheildDefence(nDamage);
			if(NULL != _pInfo &&
				_pInfo->bIgnoreMagicShield)
			{

			}
			else
			{
				nDamage -= nDefence;
				if(nDamage < 0)
				{
					nDamage = 1;
				}
			}
			//ProcessSheild(nDamage);
			ProcessSheild(nDefence);
		}
	}

	//	special ring?
	/*bool bHaveSpecialRing = false;
	if(GETITEMATB(GetEquip(PLAYER_ITEM_RING1), Type) != ITEM_NO &&
		GETITEMATB(GetEquip(PLAYER_ITEM_RING1), ID) == 176)
	{
		bHaveSpecialRing = true;
	}
	if(!bHaveSpecialRing)
	{
		if(GETITEMATB(GetEquip(PLAYER_ITEM_RING2), Type) != ITEM_NO &&
			GETITEMATB(GetEquip(PLAYER_ITEM_RING2), ID) == 176)
		{
			bHaveSpecialRing = true;
		}
	}
	if(bHaveSpecialRing)
	{
		int nMpDamage = nDamage * 1.2f;
		if(GetObject_MP() >= nMpDamage)
		{
			DecMP(nMpDamage);
			m_pValid->DecMP(nMpDamage);
			PkgPlayerUpdateAttribNtf ntf;
			ntf.bType = UPDATE_MP;
			ntf.dwParam = GetObject_MP();
			ntf.uTargetId = GetID();
			SendPacket(ntf);

			//
			nDamage = 1;
		}
		else
		{
			nMpDamage -= GetObject_MP();
			SetObject_MP(0);
			m_pValid->SetMP(0);
			PkgPlayerUpdateAttribNtf ntf;
			ntf.bType = UPDATE_MP;
			ntf.dwParam = 0;
			ntf.uTargetId = GetID();
			SendPacket(ntf);

			if(nMpDamage > 0)
			{
				nMpDamage /= 1.2f;
				if(nMpDamage <= 0)
				{
					//return;
					nDamage = 1;
				}
				else
				{
					nDamage = nMpDamage;
				}
			}
		}
	}*/

	int nSuckDamagePercent = m_xStates.GetStateValue(EAID_SUCKDAMAGE);
	if(nSuckDamagePercent != 0)
	{
		float fPercent = 1.0f - (float)nSuckDamagePercent / 100;
		nDamage *= fPercent;
	}

	//	是否来自于玩家或者宠物的伤害
	if(_pAttacker->GetType() == SOT_HERO)
	{
		if(!_bMgcAtk)
		{
			nDamage /= PVP_FACTOR_NORMAL;
		}
		else
		{
			nDamage /= PVP_FACTOR_MAGIC;
		}
	}
	else if(_pAttacker->GetType() == SOT_MONSTER)
	{
		MonsterObject* pMonster = (MonsterObject*)_pAttacker;

		if(NULL != pMonster->GetMaster())
		{
			if(!_bMgcAtk)
			{
				nDamage /= PVP_FACTOR_NORMAL;
			}
			else
			{
				nDamage /= PVP_FACTOR_MAGIC;
			}

			nDamage /= 2;
		}
	}

	if(nDamage <= 0)
	{
		return 0;
	}

	const UserMagic* pMgcJinGang = GetUserMagic(MEFF_JINGANG);
	if(pMgcJinGang &&
		pMgcJinGang->bLevel >= 1 &&
		m_dwJinGangExpireTime != 0)
	{
		float fPercent = 1.0f;
		if(pMgcJinGang->bLevel == 1)
		{
			fPercent = 0.85f;
		}
		else if(pMgcJinGang->bLevel == 2)
		{
			fPercent = 0.80f;
		}
		else if(pMgcJinGang->bLevel == 3)
		{
			fPercent = 0.70f;
		}

		int nRandom = rand() % 3;
		if(pMgcJinGang->bLevel > nRandom)
		{
			nDamage *= fPercent;
		}
	}

	//	Super shield, level > 3
	if(IsEffectExist(MEFF_SHIELD))
	{
		int nMaxHP = GetObject_MaxHP();
		const UserMagic* pInfo = GetUserMagic(MEFF_SHIELD);

		if(pInfo != NULL &&
			pInfo->bLevel > 3)
		{
			g_xConsole.CPrint("处理护体神盾");

			int nSuperLevel = pInfo->bLevel - 3;
			float fMaxDamage = 0.55f - nSuperLevel * 0.1;

			int nMaxDamage = nMaxHP * fMaxDamage;
			if(nDamage > nMaxDamage)
			{
				g_xConsole.CPrint("修正最大伤害为[%d] 初始伤害[%d]", nMaxDamage, nDamage);
				nDamage = nMaxDamage;
			}

			//	无视伤害
			int nIgnoreProb = nSuperLevel * 10;
			int nRandom = rand() % 120;

			if(nIgnoreProb > nRandom)
			{
				if(float(GetObject_HP()) / float(GetObject_MaxHP()) < 0.4f)
				{
					g_xConsole.CPrint("无视伤害");
					nDamage = 0;
				}
			}
		}
	}

	//	游戏难度
	if(CMainServer::GetInstance()->GetServerMode() == GM_NORMAL &&
		_pAttacker->GetType() != SOT_HERO)
	{
		int nDifficultyLevel = GameWorld::GetInstance().GetDifficultyLevel();
		if(kDifficultyEasy == nDifficultyLevel)
		{
			nDamage *= 0.7f;
		}
		else if(kDifficultyNovice == nDifficultyLevel)
		{
			nDamage *= 0.45f;
		}
	}

	if(nDamage <= 0)
	{
		return 0;
	}

	bool bIsDead = false;
	bool bReqStruck = false;

	//if(_pAttacker->GetType() == SOT_MONSTER)
	{
		//MonsterObject* pMonster = static_cast<MonsterObject*>(_pAttacker);

		/*nPreHP -= nDamage;
		if(nPreHP < 0)
		{
			nPreHP = 0;
		}*/
		if(pMgcJinGang &&
			pMgcJinGang->bLevel >= 1 &&
			m_dwJinGangExpireTime != 0)
		{
			float fDamage = 0;

			if(pMgcJinGang->bLevel == 1)
			{
				fDamage = 0.45f;
			}
			else if(pMgcJinGang->bLevel == 2)
			{
				fDamage = 0.55f;
			}
			else if(pMgcJinGang->bLevel == 3)
			{
				fDamage = 0.65f;
			}

			int nRandom = rand() % 12;
			if(fDamage > 0 &&
				pMgcJinGang->bLevel > nRandom)
			{
				int nMonsDamage = nDC * fDamage;

				_pAttacker->ReceiveDamage(this, false, nMonsDamage);
			}
		}

		if(DecHP(nDamage))
		{
			m_pValid->DecHP(nDamage);
			//
			if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_ENERGYSHIELD))
			{
				const UserMagic* pMgc = GetUserMagic(MEFF_ENERGYSHIELD);
				if(NULL != pMgc)
				{
					if(pMgc->bLevel > 0)
					{
						int nAddHP = 0;
						nAddHP = pMgc->bLevel * nDamage / 10;
						//IncHP(nAddHP);
						//m_pValid->IncHP(nAddHP);
						int nStep = 4;
						if(pMgc->bLevel > 3 &&
							pMgc->bLevel <=5)
						{
							nStep = 3;
						}
						else if(pMgc->bLevel >= 6)
						{
							nStep = 2;
						}
						AddEnergyShieldState(nAddHP, nStep);
						g_xConsole.CPrint("先天气功恢复体力：%d", nAddHP);
					}
				}
			}

			PkgPlayerUpdateAttribNtf ntf;
			ntf.bType = UPDATE_HP;
			ntf.dwParam = GetObject_HP();
			ntf.uTargetId = GetID();
			SendPacket(ntf);
			bReqStruck = true;
		}
		else
		{
			//	Dead
			bool bUseReviveRing = false;
			if(m_xStates.GetStateValue(EAID_REVIVE) != 0)
			{
#ifdef _DEBUG
				if (true)
#else
				if(GetTickCount() - m_dwReviveRingUseTime > 10*60*1000)
#endif
				{
					bUseReviveRing = true;
					m_dwReviveRingUseTime = GetTickCount();
					SendSystemMessage("由于复活戒指的力量复活，间隔10分钟");
					SetObject_HP(GetObject_MaxHP());
					m_pValid->SetHP(GetObject_MaxHP());
					ResetSupply();

					PkgPlayerUpdateAttribNtf ntf;
					ntf.bType = UPDATE_HP;
					ntf.dwParam = GetObject_HP();
					ntf.uTargetId = GetID();
					SendPacket(ntf);
					bReqStruck = true;
				}
			}

			if (bUseReviveRing)
			{
				// Here check the durability of item
				ItemAttrib* pItem = GetEquip(PLAYER_ITEM_RING1);
				if (GETITEMATB(pItem, ID) != 175)
				{
					pItem = NULL;
				}
				if (NULL == pItem)
				{
					pItem = GetEquip(PLAYER_ITEM_RING2);
					if (GETITEMATB(pItem, ID) != 175)
					{
						pItem = NULL;
					}
				}

				if (NULL != pItem)
				{
					WORD wMaxDura = LOWORD(GETITEMATB(pItem, MaxHP));
					WORD wCurDura = HIWORD(GETITEMATB(pItem, MaxHP));

					WORD wDuraCost = 1000;
					if (wDuraCost >= wMaxDura)
					{
						// clean item
						int nTag = pItem->tag;
						ZeroMemory(pItem, sizeof(ItemAttrib));
						ObjectValid::EncryptAttrib(pItem);

						PkgPlayerLostItemAck pplia;
						pplia.uTargetId = GetID();
						pplia.dwTag = nTag;
						SendPacket(pplia);

						RefleshAttrib();
					}
					else
					{
						WORD wLeftDura = wMaxDura - wDuraCost;
						if (wCurDura > wLeftDura)
						{
							wCurDura = wLeftDura;
						}
						SETITEMATB(pItem, MaxHP, MAKELONG(wLeftDura, wCurDura));

						PkgPlayerUpdateItemNtf ppuin;
						ppuin.uTargetId = GetID();
						memcpy(&ppuin.stItem, pItem, sizeof(ItemAttrib));
						ObjectValid::DecryptAttrib(&ppuin.stItem);
						SendPacket(ppuin);
					}
				}
			}
			else
			{
				PkgObjectActionNot notdead;
				notdead.uAction = ACTION_DEAD;
				notdead.uTargetId = GetID();
				notdead.uParam0 = MAKE_POSITION_DWORD(this);
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << notdead;
				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
				bIsDead = true;

				m_stData.eGameState = OS_DEAD;
				ResetSupply();
				KillAllSlave();

				m_pValid->DecHP(nDamage);
			}
		}
		//m_pValid->DecHP(nDamage);

		if(!bIsDead)
		{
			if(_pAttacker->GetType() == SOT_MONSTER)
			{
				MonsterObject* pMonster = (MonsterObject*)_pAttacker;
				int nRandom = 0;
				if(pMonster->IsStoneAttack())
				{
					nRandom = rand() % 100;
					if(nRandom <= pMonster->GetStoneProb())
					{
						DWORD dwStoneTime = pMonster->GetStoneTime();
						SetEffStatus(MMASK_STONE, dwStoneTime, 0);
						SetStoneRestore(false);
						PkgPlayerSetEffectAck ack;
						ack.uTargetId = GetID();
						ack.dwMgcID = MMASK_STONE;
						ack.bShow = true;
						ack.dwTime = dwStoneTime + 250;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ack;
						GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

						ForceGotoValidPosition();
						bReqStruck = false;
					}
				}

				if(pMonster->IsPoisonAttack())
				{
					nRandom = rand() % 10;
					if(nRandom == 0)
					{
						SetEffStatus(MMASK_LVDU, 8000, 8);

						PkgPlayerSetEffectAck ack;
						ack.uTargetId = GetID();
						ack.dwMgcID = MMASK_LVDU;
						ack.bShow = true;
						ack.dwTime = 8 * 1000;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ack;
						GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
						//SendStatusInfo();
					}
				}
			}

			if(bReqStruck)
			{
				if(CanStruck(nDamage))
				{
					/*DelaySendInfo info;
					info.uDelayTime = m_dwCurrentTime + 800;
					info.uOp = DELAY_HEROSTRUCK;
					info.uParam[0] = GetID();
					GetLocateScene()->PushDelayBuf(info);*/
					/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
					pMsg->dwDelayTime = m_dwCurrentTime + 800;
					pMsg->uOp = DELAY_HEROSTRUCK;
					pMsg->uParam[0] = GetID();
					GetLocateScene()->PushDelayBuf(pMsg);*/
					DelayActionStruck* pAction = new DelayActionStruck;
					pAction->dwActiveTime = GetTickCount() + 800;
					if(_bMgcAtk)
					{
						pAction->dwActiveTime += 300;
					}
					AddDelayAction(pAction);
				}
			}
		}
		else
		{
			ClearDelayAction();
			GetLocateScene()->EraseTarget(this);

			if(_pAttacker->GetType() == SOT_HERO)
			{
				char szDeadTip[128];
				sprintf(szDeadTip, "您被[%s]杀死了!!!", _pAttacker->GetName());
				SendSystemMessage(szDeadTip);
			}
			else
			{
				MonsterObject* pMonster = (MonsterObject*)_pAttacker;

				if(pMonster->GetMaster() != NULL)
				{
					char szDeadTip[128];
					sprintf(szDeadTip, "您被[%s]的宝宝杀死了!!!", pMonster->GetMaster()->GetName());
					SendSystemMessage(szDeadTip);
				}
			}
		}
	}
	/*else if(_pAttacker->GetType() == SOT_HERO)
	{
		HeroObject* pMonster = static_cast<HeroObject*>(_pAttacker);

		if(pMgcJinGang &&
			pMgcJinGang->bLevel >= 1 &&
			m_dwJinGangExpireTime != 0)
		{
			float fDamage = 0;

			if(pMgcJinGang->bLevel == 1)
			{
				fDamage = 0.45f;
			}
			else if(pMgcJinGang->bLevel == 2)
			{
				fDamage = 0.55f;
			}
			else if(pMgcJinGang->bLevel == 3)
			{
				fDamage = 0.65f;
			}

			int nRandom = rand() % 12;
			if(fDamage > 0 &&
				pMgcJinGang->bLevel > nRandom)
			{
				int nMonsDamage = nDC * fDamage;

				pMonster->ReceiveDamage(this, false, nMonsDamage);
			}
		}

		if(DecHP(nDamage))
		{
			m_pValid->DecHP(nDamage);
			//
			if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_ENERGYSHIELD))
			{
				const UserMagic* pMgc = GetUserMagic(MEFF_ENERGYSHIELD);
				if(NULL != pMgc)
				{
					if(pMgc->bLevel > 0)
					{
						int nAddHP = 0;
						nAddHP = pMgc->bLevel * nDamage / 10;
						//IncHP(nAddHP);
						//m_pValid->IncHP(nAddHP);
						int nStep = 5;
						if(pMgc->bLevel > 3 &&
							pMgc->bLevel <=5)
						{
							nStep = 4;
						}
						else if(pMgc->bLevel >= 6)
						{
							nStep = 3;
						}
						AddEnergyShieldState(nAddHP, nStep);
						g_xConsole.CPrint("先天气功恢复体力：%d", nAddHP);
					}
				}
			}

			PkgPlayerUpdateAttribNtf ntf;
			ntf.bType = UPDATE_HP;
			ntf.dwParam = GetObject_HP();
			ntf.uTargetId = GetID();
			SendPacket(ntf);
			bReqStruck = true;
		}
		else
		{
			//	Dead
			bool bUseReviveRing = false;
			if(m_xStates.GetStateValue(EAID_REVIVE) != 0)
			{
				if(GetTickCount() - m_dwReviveRingUseTime > 10*60*1000)
				{
					bUseReviveRing = true;
					m_dwReviveRingUseTime = GetTickCount();
					SendSystemMessage("由于复活戒指的力量复活，间隔10分钟");
					SetObject_HP(GetObject_MaxHP());
					m_pValid->SetHP(GetObject_MaxHP());
					ResetSupply();

					PkgPlayerUpdateAttribNtf ntf;
					ntf.bType = UPDATE_HP;
					ntf.dwParam = GetObject_HP();
					ntf.uTargetId = GetID();
					SendPacket(ntf);
					bReqStruck = true;
				}
			}

			if(!bUseReviveRing)
			{
				PkgObjectActionNot notdead;
				notdead.uAction = ACTION_DEAD;
				notdead.uTargetId = GetID();
				notdead.uParam0 = MAKE_POSITION_DWORD(this);
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << notdead;
				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
				bIsDead = true;

				m_stData.eGameState = OS_DEAD;
				ResetSupply();
				KillAllSlave();

				m_pValid->DecHP(nDamage);
			}
		}
		//m_pValid->DecHP(nDamage);

		if(!bIsDead)
		{
			int nRandom = 0;

			if(bReqStruck)
			{
				if(CanStruck(nDamage))
				{
					DelayActionStruck* pAction = new DelayActionStruck;
					pAction->dwActiveTime = GetTickCount() + 800;
					AddDelayAction(pAction);
				}
			}
		}
		else
		{
			ClearDelayAction();
			GetLocateScene()->EraseTarget(this);

			char szDeadTip[128];
			sprintf(szDeadTip, "您被[%s]杀死了!!!", pMonster->GetName());
			SendSystemMessage(szDeadTip);
		}
	}*/

	return nDamage;
}

/************************************************************************/
/* unsigned int AppendPacket(const PacketHeader* _pPkt)
/************************************************************************/
unsigned int HeroObject::AppendPacket(const PacketHeader* _pPkt)
{
	/*unsigned int uWrite = m_xSendBuffer.Write(_pPkt, _pPkt->uLen);
	if(0 == uWrite)
	{
		LOG(FATAL) << "Fatal error on writing to send buffer";
	}
	return uWrite;*/
	return 0;
}

/************************************************************************/
/* unsigned int AppendBuffer(ByteBuffer* _pBuf)
/************************************************************************/
unsigned int HeroObject::AppendBuffer(ByteBuffer* _pBuf)
{
	/*unsigned char* pBuf = const_cast<unsigned char*>(_pBuf->GetBuffer());
	*(DWORD*)pBuf = _pBuf->GetLength();
	unsigned int uWrite = m_xSendBuffer.Write(_pBuf->GetBuffer(), _pBuf->GetLength());
	return uWrite;*/
	return 0;
}

/************************************************************************/
/* virtual unsigned int SendMsgToUser(const void* _pData, unsigned int _uLen)
/************************************************************************/
unsigned int HeroObject::SendMsgToUser(const void* _pData, unsigned int _uLen)
{
	//	the packet header's uLen is a net based number
// 	extern CNetbase* g_pNet;
// 
// 	if(m_nConnectionIndex < 0)
// 	{
// 		return 0;
// 	}
// 	g_pNet->SendToUser((DWORD)m_nConnectionIndex, (char*)_pData, _uLen, 0);
	return _uLen;
}

bool HeroObject::SendPlayerBuffer(ByteBuffer& _xBuf)
{
	RECORD_FUNCNAME_WORLD;

	if(!m_bClientLoaded)
	{
		return false;
	}
	if(0 == SendBuffer(GetUserIndex(), &_xBuf))
	{
		return false;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::EncryptObject()
{
	__super::EncryptObject();

	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		//if(m_xBag[i].type != ITEM_NO)
		{
			ObjectValid::EncryptAttrib(&m_xBag[i]);
		}
	}
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		//if(m_stStore[i].type != ITEM_NO)
		{
			ObjectValid::EncryptAttrib(&m_stStore[i]);
		}
	}
	for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
	{
		{
			ObjectValid::EncryptAttrib(&m_stBigStore[i]);
		}
	}
	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		//if(m_stEquip[i].type != ITEM_NO)
		{
			ObjectValid::EncryptAttrib(&m_stEquip[i]);
		}
	}

	//	Encrypt money
	m_nMoney = EncryptValue(ATB_MONEY_MASK, m_nMoney);
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AcceptLogin(bool _bNew)
{
	m_pValid->UpdateAllAttrib();

	SetLastRecvDataTime(GetTickCount());
	//	Check version
	/*PkgPlayerVersionVerifyNtf ppvvn;
	ppvvn.uTargetId = GetID();
	ppvvn.xVersion = BACKMIR_CURVERSION;
	//SendPacket(ppvvn);
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << ppvvn;
	SendBuffer(GetUserIndex(), &g_xThreadBuffer);*/

	//	Send data
	ItemAttrib* pAttrib = &GetUserData()->stAttrib;
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << pAttrib->name;
#ifdef _DEBUG
	/*if(_bNew)
	{
		pAttrib->level = 35;
	}*/
#endif
	g_xThreadBuffer << (USHORT)pAttrib->level;
	//	current map
	GameScene* pScene = GetLocateScene();
	g_xThreadBuffer << (USHORT)pScene->GetMapResID();
	//g_xThreadBuffer << (USHORT)GetMapID();
	//	current pos
	g_xThreadBuffer << (WORD)GetUserData()->wCoordX;
	g_xThreadBuffer << (WORD)GetUserData()->wCoordY;
	//	HP MP
	g_xThreadBuffer << (DWORD)MAKELONG(pAttrib->HP, pAttrib->maxHP);
	g_xThreadBuffer << (DWORD)MAKELONG(pAttrib->MP, pAttrib->maxMP);
	//	Exp
	g_xThreadBuffer << (DWORD)pAttrib->EXPR;
	g_xThreadBuffer << (DWORD)pAttrib->maxEXPR;
	//	Money
#ifdef _DEBUG
	if(_bNew)
	{
		SetMoney(9999999);
		g_xThreadBuffer << GetMoney();
	}
	else
	{
		g_xThreadBuffer << (DWORD)GetMoney();
	}
#else
	g_xThreadBuffer << (DWORD)GetMoney();
#endif

	if(_bNew)
	{
		//	new player
		//	bag items and items on body are all zero
		g_xThreadBuffer << (BYTE)0;
		g_xThreadBuffer << (BYTE)0;
		SetNewPlayer();
	}
	else
	{
		//	items;
		int nCounter = 0;
		for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
		{
			if(GetItemByIndex(i)->type != ITEM_NO)
			{
				GetItemByIndex(i)->tag = GameWorld::GetInstance().GenerateItemTag();
				++nCounter;
			}
		}
		g_xThreadBuffer << (BYTE)nCounter;
		for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
		{
			if(GetItemByIndex(i)->type != ITEM_NO)
			{
				g_xThreadBuffer << *GetItemByIndex(i);
			}
		}
		//	body items
		nCounter = 0;
		for(int i = PLAYER_ITEM_WEAPON; i < PLAYER_ITEM_TOTAL; ++i)
		{
			if(GetEquip((PLAYER_ITEM_TYPE)i)->type != ITEM_NO)
			{
				GetEquip((PLAYER_ITEM_TYPE)i)->tag = GameWorld::GetInstance().GenerateItemTag();
				++nCounter;
			}
		}
		g_xThreadBuffer << (BYTE)nCounter;
		for(int i = PLAYER_ITEM_WEAPON; i < PLAYER_ITEM_TOTAL; ++i)
		{
			if(GetEquip((PLAYER_ITEM_TYPE)i)->type != ITEM_NO)
			{
				g_xThreadBuffer << (BYTE)i;
				g_xThreadBuffer << *GetEquip((PLAYER_ITEM_TYPE)i);
			}
		}
		//	magic
		nCounter = 0;
		for(int i = 0; i < USER_MAGIC_NUM; ++i)
		{
			if(GetUserMagicByIndex(i)->bLevel != 0)
			{
				++nCounter;
			}
		}
		g_xThreadBuffer << (BYTE)nCounter;
		const UserMagic* pMagic = NULL;
		for(int i = 0; i < USER_MAGIC_NUM; ++i)
		{
			pMagic = GetUserMagicByIndex(i);
			if(pMagic->bLevel != 0)
			{
				if(GetHeroJob() == 0)
				{
					g_xThreadBuffer << (WORD)(i + MEFF_DC_BEGIN + 1) << (BYTE)pMagic->bLevel;
				}
				else if(GetHeroJob() == 1)
				{
					g_xThreadBuffer << (WORD)(i + MEFF_MC_BEGIN + 1) << (BYTE)pMagic->bLevel;
				}
				else if(GetHeroJob() == 2)
				{
					g_xThreadBuffer << (WORD)(i + MEFF_SC_BEGIN + 1) << (BYTE)pMagic->bLevel;
				}
			}
		}
	}

	//	Set storage tag
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		if(GetStoreItemByIndex(i)->type != ITEM_NO)
		{
			GetStoreItemByIndex(i)->tag = GameWorld::GetInstance().GenerateItemTag();
		}
	}

	EncryptObject();

	/*bool bValid = DBThread::GetInstance()->VerifyHeroItem(this);
	if(!bValid)
	{
		return false;
	}*/

#define MAX_DATA_SIZE 20480
	static char* s_pData = new char[MAX_DATA_SIZE];
	GlobalAllocRecord::GetInstance()->RecordArray(s_pData);
	uLongf srcsize = g_xThreadBuffer.GetLength();
	uLongf cmpsize = MAX_DATA_SIZE;
	PkgUserLoginAck ack;
	ack.uVersion = BACKMIR_VERSION;

	int nRet = compress((Bytef*)s_pData, &cmpsize, (const Bytef*)g_xThreadBuffer.GetBuffer(), srcsize);
	if(nRet == Z_OK)
	{
		//	OK
		PostMessage(g_hServerDlg, WM_INSERTMAPKEY, (WPARAM)GetUserIndex(), (LPARAM)GetID());

		ack.xMsg.resize(cmpsize);
		/*for(int i = 0; i < cmpsize; ++i)
		{
			ack.xMsg[i] = pData[i];
		}*/
		memcpy((char*)(&ack.xMsg[0]), s_pData, cmpsize);
		ack.bOk = true;
		ack.nId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		SendBuffer(GetUserIndex(), &g_xThreadBuffer);

		if(_bNew)
		{
			//	Add new item
			AddItem(30);
			//if(GetHeroSex() == 1)
			if(GetObject_Sex() == 1)
			{
				AddItem(60);
			}
			else
			{
				AddItem(80);
			}
			AddItem(394);
			AddItem(398);
			AddItem(404);
			AddItem(404);
			AddItem(404);
			AddItem(404);
			AddItem(404);
			AddItem(404);
		}

		RefleshAttrib();
	}
	else
	{
		LOG(ERROR) << "Compress error!";
		return false;
	}
	//delete[] pData;

	OnPlayerLogin();

	m_dwLoginTimeTick = GetTickCount();
	return true;
}

void HeroObject::VerifyLogin()
{

}

//////////////////////////////////////////////////////////////////////////
ItemAttrib* HeroObject::GetItemByTag(DWORD _dwTag)
{
	RECORD_FUNCNAME_WORLD;

	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(m_xBag[i].tag == _dwTag)
		{
			return &m_xBag[i];
		}
	}
	return NULL;
}

ItemAttrib* HeroObject::Lua_GetItemByAttribID(int _nId)
{
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(GETITEMATB(&m_xBag[i], ID) == _nId)
		{
			return &m_xBag[i];
		}
	}
	return NULL;
}

ItemAttrib* HeroObject::GetItemByIndex(DWORD _dwIndex)
{
	RECORD_FUNCNAME_WORLD;

	if(_dwIndex >= HERO_BAG_SIZE_CUR)
	{
		return NULL;
	}
	return &m_xBag[_dwIndex];
}

bool HeroObject::AddBagItem(const ItemAttrib* _pItem)
{
	RECORD_FUNCNAME_WORLD;

	ItemAttrib* pItem = NULL;
	ItemAttrib* pReadItem = const_cast<ItemAttrib*>(_pItem);

	if(GETITEMATB(pReadItem, Type) == ITEM_NO)
	{
		return false;
	}

	/*if(GETITEMATB(pReadItem, Type) == ITEM_COST)
	{
		for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
		{
			if(GETITEMATB(&m_xBag[i], Type) == ITEM_COST &&
				GETITEMATB(&m_xBag[i], ID) == GETITEMATB(pReadItem, ID))
			{
				if(GETITEMATB(&m_xBag[i], AtkSpeed) < 20)
				{
					pItem = &m_xBag[i];
					break;
				}
			}
		}

		if(NULL == pItem)
		{
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
					m_xBag[i].tag != ITEMTAG_INQUERY)
				{
					pItem = &m_xBag[i];
					break;
				}
			}
		}

		if(NULL != pItem)
		{
			if(GETITEMATB(pItem, Type) == ITEM_COST)
			{
				int nSum = GETITEMATB(pItem, AtkSpeed) + 1;
				SETITEMATB(pItem, AtkSpeed, nSum);
			}
			else if(GETITEMATB(pItem, Type) == ITEM_NO)
			{
				memcpy(pItem, _pItem, sizeof(ItemAttrib));
				SETITEMATB(pItem, AtkSpeed, 1);
			}
			return true;
		}
	}
	else*/
	{
		for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
		{
			if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
				m_xBag[i].tag != ITEMTAG_INQUERY)
			{
				pItem = &m_xBag[i];
				break;
			}
		}

		if(NULL == pItem)
		{
			return false;
		}
		else
		{
			memcpy(pItem, _pItem, sizeof(ItemAttrib));
			pItem->price = g_nItemPrice[GETITEMATB(pItem, ID)];
			return true;
		}
	}

	return false;
}

bool HeroObject::AddBagItem(DWORD _dwIndex, const ItemAttrib* _pItem)
{
	RECORD_FUNCNAME_WORLD;

	ItemAttrib* pItem = NULL;
	if(_dwIndex >= HERO_BAG_SIZE_CUR)
	{
		return false;
	}
	
	pItem = &m_xBag[_dwIndex];
	if(GETITEMATB(pItem, Type) != ITEM_NO)
	{
		return false;
	}
	if(pItem->tag == ITEMTAG_INQUERY)
	{
		return false;
	}

	memcpy(pItem, _pItem, sizeof(ItemAttrib));
	return true;
}

BYTE HeroObject::CheckItemCanDress(ItemAttrib* _pItem)
{
	RECORD_FUNCNAME_WORLD;

	BYTE bRet = 0;

	switch(GETITEMATB(_pItem, ReqType))
	{
	case REQ_LEVEL:
		{
			if(GetObject_Level() < GETITEMATB(_pItem, ReqValue))
			{
				bRet = REQ_ERR_LEVEL;
			}
		}break;
	case REQ_DC:
		{
			if(GetObject_MaxDC() < GETITEMATB(_pItem, ReqValue))
			{
				bRet = REQ_ERR_DC;
			}
		}break;
	case REQ_MC:
		{
			if(GetObject_MaxMC() < GETITEMATB(_pItem, ReqValue))
			{
				bRet = REQ_ERR_MC;
			}
		}break;
	case REQ_SC:
		{
			if(GetObject_MaxSC() < GETITEMATB(_pItem, ReqValue))
			{
				bRet = REQ_ERR_SC;
			}
		}break;
	}

	if(GETITEMATB(_pItem, Sex) != 0)
	{
		if(GETITEMATB(_pItem, Sex) != GetObject_Sex())
		{
			bRet = REQ_ERR_SEX;
		}
	}

	return bRet;
}

bool HeroObject::IsCostItem(DWORD _dwType)
{
	/*if(_dwType == ITEM_YAO_SPE ||
		_dwType == ITEM_YAP ||
		_dwType == ITEM_SCROLL)*/
	if(_dwType == ITEM_COST)
	{
		return true;
	}

	return false;
}

bool HeroObject::IsEquipItem(DWORD _dwType)
{
	if(_dwType == ITEM_WEAPON ||
		_dwType == ITEM_CLOTH ||
		_dwType == ITEM_NECKLACE ||
		_dwType == ITEM_RING ||
		_dwType == ITEM_BRACELAT ||
		_dwType == ITEM_HELMET ||
		_dwType == ITEM_MEDAL ||
		_dwType == ITEM_SHOE ||
		_dwType == ITEM_GEM ||
		_dwType == ITEM_CHARM ||
		_dwType == ITEM_BELT)
	{
		return true;
	}

	return false;
}

bool HeroObject::IsJewelryItem(DWORD _dwType)
{
	if(_dwType == ITEM_NECKLACE ||
		_dwType == ITEM_RING ||
		_dwType == ITEM_BRACELAT ||
		_dwType == ITEM_GEM)
	{
		return true;
	}

	return false;
}

bool HeroObject::IsDefenceItem(DWORD _dwType)
{
	if(_dwType == ITEM_CLOTH ||
		_dwType == ITEM_HELMET ||
		_dwType == ITEM_MEDAL ||
		_dwType == ITEM_SHOE ||
		_dwType == ITEM_BELT)
	{
		return true;
	}

	return false;
}

bool HeroObject::IsAttackItem(DWORD _dwType)
{
	if(_dwType == ITEM_WEAPON)
	{
		return true;
	}

	return false;
}

void HeroObject::GetHeroAttrib(ItemAttrib* _pItem)
{
	ItemAttrib item;
	ZeroMemory(&item, sizeof(ItemAttrib));

	for(int i = PLAYER_ITEM_WEAPON; i < PLAYER_ITEM_TOTAL; ++i)
	{
		if(GETITEMATB(&m_stEquip[i], Type) == ITEM_NO)
		{
			continue;
		}
		else if(GETITEMATB(&m_stEquip[i], Type) == ITEM_WEAPON)
		{
			//item.AC += m_equip[i].AC;
			//item.maxAC += m_equip[i].maxAC;
			item.DC += m_stEquip[i].DC;
			item.maxDC += m_stEquip[i].maxDC;
			item.SC += m_stEquip[i].SC;
			item.maxSC += m_stEquip[i].maxSC;
			item.MC += m_stEquip[i].MC;
			item.maxMC += m_stEquip[i].maxMC;
			//item.MAC += m_equip[i].MAC;
			//item.maxMAC += m_equip[i].maxMAC;
			//item.moveSpeed += m_equip[i].moveSpeed;
			//item.atkPalsy += m_equip[i].atkPalsy;
			//item.atkPois += m_equip[i].atkPois;
			//item.atkSpeed += m_equip[i].atkSpeed;
			item.lucky += m_stEquip[i].lucky;
			item.accuracy += m_stEquip[i].accuracy;
			//item.hide += m_equip[i].hide;
		}
		else if(m_stEquip[i].type == ITEM_HELMET)
		{
			item.AC += m_stEquip[i].AC;
			item.maxAC += m_stEquip[i].maxAC;
			item.DC += m_stEquip[i].DC;
			item.maxDC += m_stEquip[i].maxDC;
			item.SC += m_stEquip[i].SC;
			item.maxSC += m_stEquip[i].maxSC;
			item.MC += m_stEquip[i].MC;
			item.maxMC += m_stEquip[i].maxMC;
			item.MAC += m_stEquip[i].MAC;
			item.maxMAC += m_stEquip[i].maxMAC;
			//item.moveSpeed += m_equip[i].moveSpeed;
			//item.atkPalsy += m_equip[i].atkPalsy;
			//item.atkPois += m_equip[i].atkPois;
			//item.atkSpeed += m_equip[i].atkSpeed;
			//item.lucky += m_equip[i].lucky;
			//item.accuracy += m_equip[i].accuracy;
			//item.hide += m_equip[i].hide;
		}
		else if(m_stEquip[i].type == ITEM_NECKLACE)
		{
			//item.AC += m_equip[i].AC;
			//item.maxAC += m_equip[i].maxAC;
			item.DC += m_stEquip[i].DC;
			item.maxDC += m_stEquip[i].maxDC;
			item.SC += m_stEquip[i].SC;
			item.maxSC += m_stEquip[i].maxSC;
			item.MC += m_stEquip[i].MC;
			item.maxMC += m_stEquip[i].maxMC;
			//item.MAC += m_equip[i].MAC;
			//item.maxMAC += m_equip[i].maxMAC;
			//item.moveSpeed += m_equip[i].moveSpeed;
			//item.atkPalsy += m_equip[i].atkPalsy;
			//item.atkPois += m_equip[i].atkPois;
			item.atkSpeed += m_stEquip[i].atkSpeed;
			item.lucky += m_stEquip[i].lucky;
			item.accuracy += m_stEquip[i].accuracy;
			item.hide += m_stEquip[i].hide;
		}
		else if(m_stEquip[i].type == ITEM_CLOTH)
		{
			item.AC += m_stEquip[i].AC;
			item.maxAC += m_stEquip[i].maxAC;
			item.DC += m_stEquip[i].DC;
			item.maxDC += m_stEquip[i].maxDC;
			item.SC += m_stEquip[i].SC;
			item.maxSC += m_stEquip[i].maxSC;
			item.MC += m_stEquip[i].MC;
			item.maxMC += m_stEquip[i].maxMC;
			item.MAC += m_stEquip[i].MAC;
			item.maxMAC += m_stEquip[i].maxMAC;
			//item.moveSpeed += m_equip[i].moveSpeed;
			//item.atkPalsy += m_equip[i].atkPalsy;
			//item.atkPois += m_equip[i].atkPois;
			//item.atkSpeed += m_equip[i].atkSpeed;
			//item.lucky += m_equip[i].lucky;
			//item.accuracy += m_equip[i].accuracy;
			//item.hide += m_equip[i].hide;
		}
		else if(m_stEquip[i].type == ITEM_MEDAL)
		{
			item.AC += m_stEquip[i].AC;
			item.maxAC += m_stEquip[i].maxAC;
			item.DC += m_stEquip[i].DC;
			item.maxDC += m_stEquip[i].maxDC;
			item.SC += m_stEquip[i].SC;
			item.maxSC += m_stEquip[i].maxSC;
			item.MC += m_stEquip[i].MC;
			item.maxMC += m_stEquip[i].maxMC;
			item.MAC += m_stEquip[i].MAC;
			item.maxMAC += m_stEquip[i].maxMAC;
			//item.moveSpeed += m_equip[i].moveSpeed;
			//item.atkPalsy += m_equip[i].atkPalsy;
			//item.atkPois += m_equip[i].atkPois;
			//item.atkSpeed += m_equip[i].atkSpeed;
			//item.lucky += m_equip[i].lucky;
			//item.accuracy += m_equip[i].accuracy;
			//item.hide += m_equip[i].hide;
		}
		else if(m_stEquip[i].type == ITEM_BRACELAT)
		{
			item.AC += m_stEquip[i].AC;
			item.maxAC += m_stEquip[i].maxAC;
			item.DC += m_stEquip[i].DC;
			item.maxDC += m_stEquip[i].maxDC;
			item.SC += m_stEquip[i].SC;
			item.maxSC += m_stEquip[i].maxSC;
			item.MC += m_stEquip[i].MC;
			item.maxMC += m_stEquip[i].maxMC;
			item.MAC += m_stEquip[i].MAC;
			item.maxMAC += m_stEquip[i].maxMAC;
			//item.moveSpeed += m_equip[i].moveSpeed;
			//item.atkPalsy += m_equip[i].atkPalsy;
			//item.atkPois += m_equip[i].atkPois;
			//item.atkSpeed += m_equip[i].atkSpeed;
			//item.lucky += m_equip[i].lucky;
			item.accuracy += m_stEquip[i].accuracy;
			item.hide += m_stEquip[i].hide;
		}
		else if(m_stEquip[i].type == ITEM_RING)
		{
			item.AC += m_stEquip[i].AC;
			item.maxAC += m_stEquip[i].maxAC;
			item.DC += m_stEquip[i].DC;
			item.maxDC += m_stEquip[i].maxDC;
			item.SC += m_stEquip[i].SC;
			item.maxSC += m_stEquip[i].maxSC;
			item.MC += m_stEquip[i].MC;
			item.maxMC += m_stEquip[i].maxMC;
			item.MAC += m_stEquip[i].MAC;
			item.maxMAC += m_stEquip[i].maxMAC;
			item.moveSpeed += m_stEquip[i].moveSpeed;
			//item.atkPalsy += m_stEquip[i].atkPalsy;
			//item.atkPois += m_equip[i].atkPois;
			item.atkSpeed += m_stEquip[i].atkSpeed;
			//item.lucky += m_equip[i].lucky;
			//item.accuracy += m_equip[i].accuracy;
			//item.hide += m_equip[i].hide;
		}
		/*item.AC += m_stEquip[i].AC;
		item.maxAC += m_stEquip[i].maxAC;
		item.DC += m_stEquip[i].DC;
		item.maxDC += m_stEquip[i].maxDC;
		item.SC += m_stEquip[i].SC;
		item.maxSC += m_stEquip[i].maxSC;
		item.MC += m_stEquip[i].MC;
		item.maxMC += m_stEquip[i].maxMC;
		item.MAC += m_stEquip[i].MAC;
		item.maxMAC += m_stEquip[i].maxMAC;
		item.moveSpeed += m_stEquip[i].moveSpeed;
		item.atkPalsy += m_stEquip[i].atkPalsy;
		item.atkPois += m_stEquip[i].atkPois;
		item.atkSpeed += m_stEquip[i].atkSpeed;
		item.lucky += m_stEquip[i].lucky;
		item.accuracy += m_stEquip[i].accuracy;
		item.hide += m_stEquip[i].hide;*/
	}

	item.HP = m_stData.stAttrib.HP;
	item.maxHP = m_stData.stAttrib.maxHP;
	item.MP = m_stData.stAttrib.MP;
	item.maxMP = m_stData.stAttrib.maxMP;
	item.EXPR = m_stData.stAttrib.EXPR;
	item.maxEXPR = m_stData.stAttrib.maxEXPR;
	item.level = m_stData.stAttrib.level;
	item.sex = m_stData.stAttrib.sex;
	item.id = m_stData.stAttrib.id;
	//strcpy(item.name, m_stData.stAttrib.name);
	memcpy(item.name, m_stData.stAttrib.name, sizeof(item.name));

	int n7MinAdd = m_stData.stAttrib.level / 7;
	int n7MaxAdd = n7MinAdd + 1;
	int n14MinAdd = m_stData.stAttrib.level / 14;
	int n14MaxAdd = n14MinAdd + 1;

	if(m_stData.bJob == 0)
	{
		item.DC += n7MinAdd;
		item.maxDC += n7MaxAdd;
		item.AC += n7MinAdd;
		item.maxAC += n7MaxAdd;
	}
	else if(m_stData.bJob == 1)
	{
		item.MC += n7MinAdd;
		item.maxMC += n7MaxAdd;
		item.MAC += n7MinAdd;
		item.maxMAC += n7MaxAdd;
	}
	else if(m_stData.bJob == 2)
	{
		item.SC += n7MinAdd;
		item.maxSC += n7MaxAdd;
		item.DC += n14MinAdd;
		item.maxDC += n14MaxAdd;
		item.AC += n14MinAdd;
		item.maxAC += n14MaxAdd;
		item.MAC += n14MinAdd;
		item.maxMAC += n14MaxAdd;
	}

	//m_stData.stAttrib = item;

	if(m_stData.bJob == 0)
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_BASESWORD);
		if(pMgc->bLevel == 1)
		{
			item.accuracy += 1;
		}
		else if(pMgc->bLevel == 2)
		{
			item.accuracy += 3;
		}
		else if(pMgc->bLevel == 3)
		{
			item.accuracy += 6;
		}

		pMgc = GetUserMagic(MEFF_BERSERKER);
		if(pMgc->bLevel == 1)
		{
			item.atkSpeed += 1;
			item.DC *= 1.1f;
			item.maxDC *= 1.3f;
			item.maxAC += 4;
			item.accuracy += 2;
		}
		else if(pMgc->bLevel == 2)
		{
			item.atkSpeed += 2;
			item.DC *= 1.2f;
			item.maxDC *= 1.4f;
			item.maxAC += 7;
			item.accuracy += 3;
		}
		else if(pMgc->bLevel == 3)
		{
			item.atkSpeed += 3;
			item.DC *= 1.3f;
			item.maxDC *= 1.5f;
			item.maxAC += 10;
			item.accuracy += 4;
		}

		//m_stData.stAttrib.atkSpeed += 1;
		//m_stData.stAttrib.atkSpeed += 4;
		//m_stData.stAttrib.atkSpeed += (m_stData.stAttrib.level / 10);
	}
	else if(m_stData.bJob == 2)
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_SPIRIT);
		if(pMgc->bLevel == 1)
		{
			item.accuracy += 1;
		}
		else if(pMgc->bLevel == 2)
		{
			item.accuracy += 2;
			item.hide += 1;
		}
		else if(pMgc->bLevel == 3)
		{
			item.accuracy += 3;
			item.hide += 2;
		}
	}

	*_pItem = item;
}

void HeroObject::RefleshAttrib()
{
	ItemAttrib item;
	ItemAttrib itemdecrypt;
	ZeroMemory(&item, sizeof(ItemAttrib));
	ZeroMemory(&itemdecrypt, sizeof(ItemAttrib));
	m_xStates.ClearForever();

	bool bEncrypt = TEST_FLAG_BOOL(m_stData.stAttrib.extra, EXTRA_MASK_ENCRYPT);

	int nStealHPPercent = 0;
	int nNearPalsyPercent = 0;

	for(int i = PLAYER_ITEM_WEAPON; i < PLAYER_ITEM_TOTAL; ++i)
	{
		if(GETITEMATB(&m_stEquip[i], Type) == ITEM_NO)
		{
			continue;
		}
		else
		{
			itemdecrypt = m_stEquip[i];
			if(bEncrypt)
			{
				ObjectValid::DecryptAttrib(&itemdecrypt);
			}

			int nPotentialIndex = PotentialAttribHelper::GetPotentialIndex(itemdecrypt.MP);
			const StoveAttribInfo* pStoveAttribInfo = StoveManager::GetInstance()->GetStoveAttrib(nPotentialIndex);

			float fDefenceMulti = 1.0f;
			float fAttackMulti = 1.0f;

			if(NULL != pStoveAttribInfo)
			{
				//	find enhance defence and attack attrib
				for(int i = 0; i < pStoveAttribInfo->nAttribCount; ++i)
				{
					int nAttribId = LOWORD(pStoveAttribInfo->dwAttribs[i]);
					int nAttribValue = HIWORD(pStoveAttribInfo->dwAttribs[i]);

					if(nAttribId == kStoveAttrib_EnhanceDefence)
					{
						fDefenceMulti += float(nAttribValue) / 100;
					}
					else if(nAttribId == kStoveAttrib_EnhanceAttack)
					{
						fAttackMulti += float(nAttribValue) / 100;
					}
				}
			}

			if(itemdecrypt.type == ITEM_WEAPON)
			{
				//item.AC += itemdecrypt.AC;
				//item.maxAC += itemdecrypt.maxAC;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				//item.MAC += itemdecrypt.MAC;
				//item.maxMAC += itemdecrypt.maxMAC;
				//item.moveSpeed += itemdecrypt.moveSpeed;
				//item.atkPalsy += itemdecrypt.atkPalsy;
				//item.atkPois += itemdecrypt.atkPois;
				//item.atkSpeed += itemdecrypt.atkSpeed;
				item.lucky += itemdecrypt.lucky;
				item.accuracy += itemdecrypt.accuracy;
				//item.hide += m_equip[i].hide;
			}
			else if(itemdecrypt.type == ITEM_HELMET)
			{
				item.AC += itemdecrypt.AC;
				item.maxAC += itemdecrypt.maxAC * fDefenceMulti;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				item.MAC += itemdecrypt.MAC;
				item.maxMAC += itemdecrypt.maxMAC * fDefenceMulti;
				//item.moveSpeed += m_equip[i].moveSpeed;
				//item.atkPalsy += m_equip[i].atkPalsy;
				//item.atkPois += m_equip[i].atkPois;
				//item.atkSpeed += m_equip[i].atkSpeed;
				//item.lucky += m_equip[i].lucky;
				//item.accuracy += m_equip[i].accuracy;
				//item.hide += m_equip[i].hide;
			}
			else if(itemdecrypt.type == ITEM_NECKLACE)
			{
				//item.AC += m_equip[i].AC;
				//item.maxAC += m_equip[i].maxAC;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				//item.MAC += m_equip[i].MAC;
				//item.maxMAC += m_equip[i].maxMAC;
				//item.moveSpeed += m_equip[i].moveSpeed;
				//item.atkPalsy += m_equip[i].atkPalsy;
				//item.atkPois += m_equip[i].atkPois;
				item.atkSpeed += itemdecrypt.atkSpeed;
				item.lucky += itemdecrypt.lucky;
				item.accuracy += itemdecrypt.accuracy;
				item.hide += itemdecrypt.hide;
			}
			else if(itemdecrypt.type == ITEM_CLOTH)
			{
				item.AC += itemdecrypt.AC;
				item.maxAC += itemdecrypt.maxAC * fDefenceMulti;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				item.MAC += itemdecrypt.MAC;
				item.maxMAC += itemdecrypt.maxMAC * fDefenceMulti;
				//item.moveSpeed += m_equip[i].moveSpeed;
				//item.atkPalsy += m_equip[i].atkPalsy;
				//item.atkPois += m_equip[i].atkPois;
				//item.atkSpeed += m_equip[i].atkSpeed;
				//item.lucky += m_equip[i].lucky;
				//item.accuracy += m_equip[i].accuracy;
				//item.hide += m_equip[i].hide;
			}
			else if(itemdecrypt.type == ITEM_MEDAL)
			{
				item.AC += itemdecrypt.AC;
				item.maxAC += itemdecrypt.maxAC * fDefenceMulti;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				item.MAC += itemdecrypt.MAC;
				item.maxMAC += itemdecrypt.maxMAC * fDefenceMulti;
				//item.moveSpeed += m_equip[i].moveSpeed;
				//item.atkPalsy += m_equip[i].atkPalsy;
				//item.atkPois += m_equip[i].atkPois;
				//item.atkSpeed += m_equip[i].atkSpeed;
				//item.lucky += m_equip[i].lucky;
				//item.accuracy += m_equip[i].accuracy;
				//item.hide += m_equip[i].hide;
			}
			else if(itemdecrypt.type == ITEM_BRACELAT)
			{
				item.AC += itemdecrypt.AC;
				item.maxAC += itemdecrypt.maxAC * fDefenceMulti;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				item.MAC += itemdecrypt.MAC;
				item.maxMAC += itemdecrypt.maxMAC * fDefenceMulti;
				//item.moveSpeed += m_equip[i].moveSpeed;
				//item.atkPalsy += m_equip[i].atkPalsy;
				//item.atkPois += m_equip[i].atkPois;
				//item.atkSpeed += m_equip[i].atkSpeed;
				//item.lucky += m_equip[i].lucky;
				item.accuracy += itemdecrypt.accuracy;
				item.hide += itemdecrypt.hide;
			}
			else if(itemdecrypt.type == ITEM_RING)
			{
				item.AC += itemdecrypt.AC;
				item.maxAC += itemdecrypt.maxAC * fDefenceMulti;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				item.MAC += itemdecrypt.MAC;
				item.maxMAC += itemdecrypt.maxMAC * fDefenceMulti;
				item.moveSpeed += itemdecrypt.moveSpeed;
				//item.atkPalsy += itemdecrypt.atkPalsy;
				//item.atkPois += m_equip[i].atkPois;
				item.atkSpeed += itemdecrypt.atkSpeed;
				//item.lucky += m_equip[i].lucky;
				//item.accuracy += m_equip[i].accuracy;
				//item.hide += m_equip[i].hide;
			}
			else if(itemdecrypt.type == ITEM_SHOE)
			{
				item.AC += itemdecrypt.AC;
				item.maxAC += itemdecrypt.maxAC * fDefenceMulti;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				item.MAC += itemdecrypt.MAC;
				item.maxMAC += itemdecrypt.maxMAC * fDefenceMulti;
				item.moveSpeed += itemdecrypt.moveSpeed;
				//item.atkPalsy += itemdecrypt.atkPalsy;
			}
			else if(itemdecrypt.type == ITEM_BELT)
			{
				item.AC += itemdecrypt.AC;
				item.maxAC += itemdecrypt.maxAC * fDefenceMulti;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				item.MAC += itemdecrypt.MAC;
				item.maxMAC += itemdecrypt.maxMAC * fDefenceMulti;
				//item.atkPalsy += itemdecrypt.atkPalsy;
			}
			else if(itemdecrypt.type == ITEM_GEM)
			{
				item.AC += itemdecrypt.AC;
				item.maxAC += itemdecrypt.maxAC * fDefenceMulti;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				item.MAC += itemdecrypt.MAC;
				item.maxMAC += itemdecrypt.maxMAC * fDefenceMulti;
				//item.atkPalsy += itemdecrypt.atkPalsy;
			}
			else if(itemdecrypt.type == ITEM_CHARM)
			{
				item.AC += itemdecrypt.AC;
				item.maxAC += itemdecrypt.maxAC * fDefenceMulti;
				item.DC += itemdecrypt.DC;
				item.maxDC += itemdecrypt.maxDC * fAttackMulti;
				item.SC += itemdecrypt.SC;
				item.maxSC += itemdecrypt.maxSC * fAttackMulti;
				item.MC += itemdecrypt.MC;
				item.maxMC += itemdecrypt.maxMC * fAttackMulti;
				item.MAC += itemdecrypt.MAC;
				item.maxMAC += itemdecrypt.maxMAC * fDefenceMulti;
			}

			if(itemdecrypt.id == 176)
			{
				//	护身戒指
				m_xStates.PushItem(EAID_HURTTRANS, STATE_FOREVER, 1);
			}
			else if(itemdecrypt.id == 168)
			{
				//	麻痹戒指
				//m_xStates.PushItem(EAID_NEARPALSY, STATE_FOREVER, 1);
				nNearPalsyPercent += 12;
			}
			else if(itemdecrypt.id == 172)
			{
				//	传送戒指
				m_xStates.PushItem(EAID_MOVEPOS, STATE_FOREVER, 1);
			}
			else if(itemdecrypt.id == 175)
			{
				//	复活戒指
				m_xStates.PushItem(EAID_REVIVE, STATE_FOREVER, 1);
			}

			if(itemdecrypt.id == 433 ||
				itemdecrypt.id == 432 ||
				itemdecrypt.id == 434)
			{
				nStealHPPercent += 5;
			}

			//	隐藏属性
			if(itemdecrypt.maxMP != 0)
			{
				int nActiveAttribSum = HideAttribHelper::GetActiveAttribCount(itemdecrypt.maxMP);

				for(int i = 0; i < nActiveAttribSum; ++i)
				{
					int nActiveAttribType = HideAttribHelper::GetActiveAttribType(i, itemdecrypt.maxMP);
					int nActiveAttribValue = HideAttribHelper::GetActiveAttribValue(i, itemdecrypt.maxMP);

					if(HideAttrib_AC == nActiveAttribType)
					{
						//item.maxAC += nActiveAttribValue;
						m_xStates.AddItemValueIfExistsForever(EAID_DCDEFENSE, nActiveAttribValue);
					}
					else if(HideAttrib_MAC == nActiveAttribType)
					{
						//item.maxMAC += nActiveAttribValue;
						m_xStates.AddItemValueIfExistsForever(EAID_MAGICDEFENSE, nActiveAttribValue);
					}
					else if(HideAttrib_DC == nActiveAttribType)
					{
						//item.maxDC += nActiveAttribValue;
						m_xStates.AddItemValueIfExistsForever(EAID_DCDAMAGE, nActiveAttribValue);
					}
					else if(HideAttrib_MC == nActiveAttribType)
					{
						//item.maxMC += nActiveAttribValue;
						m_xStates.AddItemValueIfExistsForever(EAID_MCDAMAGE, nActiveAttribValue);
					}
					else if(HideAttrib_SC == nActiveAttribType)
					{
						//item.maxSC += nActiveAttribValue;
						m_xStates.AddItemValueIfExistsForever(EAID_SCDAMAGE, nActiveAttribValue);
					}
					else if(HideAttrib_Lucky == nActiveAttribType)
					{
						item.lucky += nActiveAttribValue;
					}
					else if(HideAttrib_DcHide == nActiveAttribType)
					{
						//item.hide += nActiveAttribValue;
						m_xStates.AddItemValueIfExistsForever(EAID_DCHIDE, nActiveAttribValue);
					}
					else if(HideAttrib_MagicHide == nActiveAttribType)
					{
						//item.accuracy += nActiveAttribValue;
						m_xStates.AddItemValueIfExistsForever(EAID_MAGICHIDE, nActiveAttribValue);
					}
					else if(HideAttrib_MagicDrop == nActiveAttribType)
					{
						m_xStates.PushItem(EAID_MAGICITEM, STATE_FOREVER, nActiveAttribValue);
					}
				}
			}

			//	stove attrib
			if(NULL != pStoveAttribInfo)
			{
				for(int i = 0; i < pStoveAttribInfo->nAttribCount; ++i)
				{
					int nAttribId = LOWORD(pStoveAttribInfo->dwAttribs[i]);
					int nAttribValue = HIWORD(pStoveAttribInfo->dwAttribs[i]);

					if(0 == nAttribId ||
						0 == nAttribValue)
					{
						continue;
					}

					switch(nAttribId)
					{
					case kStoveAttrib_CriticalAttack:
						{
							int nPrevValue = m_xStates.GetStateValue(EAID_CRITICALPEC);
							m_xStates.PushItem(EAID_CRITICALPEC, STATE_FOREVER, nAttribValue + nPrevValue);
						}break;
					case kStoveAttrib_CriticalLimit:
						{
							int nPrevValue = m_xStates.GetStateValue(EAID_CRITICALLIMIT);
							m_xStates.PushItem(EAID_CRITICALLIMIT, STATE_FOREVER, nAttribValue + nPrevValue);
						}break;
					case kStoveAttrib_NormalAttackSpeed:
						{
							item.atkSpeed += nAttribValue;
						}break;
					case kStoveAttrib_SuckHP:
						{
							int nPrevValue = m_xStates.GetStateValue(EAID_STEALHP);
							m_xStates.PushItem(EAID_STEALHP, STATE_FOREVER, nAttribValue + nPrevValue);
						}break;
					case kStoveAttrib_SummonWhiteTiger:
						{
							int nPrevValue = m_xStates.GetStateValue(EAID_SUMMONWHITETIGER);
							m_xStates.PushItem(EAID_SUMMONWHITETIGER, STATE_FOREVER, nAttribValue + nPrevValue);
						}break;
					case kStoveAttrib_NearPalsy:
						{
							m_xStates.AddItemValueIfExistsForever(EAID_NEARPALSY, nAttribValue);
						}break;
					case kStoveAttrib_Mountain:
						{
							m_xStates.AddItemValueIfExistsForever(EAID_MOUNTAIN, nAttribValue);
						}break;
					case kStoveAttrib_PoisRecover:
						{
							m_xStates.AddItemValueIfExistsForever(EAID_POISRECOVER, nAttribValue);
						}break;
					case kStoveAttrib_StoneRecover:
						{
							m_xStates.AddItemValueIfExistsForever(EAID_STONERECOVER, nAttribValue);
						}break;
					case kStoveAttrib_AddHPSecond:
						{
							m_xStates.AddItemValueIfExistsForever(EAID_ADDHPSECOND, nAttribValue);
						}break;
					case kStoveAttrib_AddMPSecond:
						{
							m_xStates.AddItemValueIfExistsForever(EAID_ADDMPSECOND, nAttribValue);
						}break;
					}
				}
			}
		}
	}

	if(nStealHPPercent != 0)
	{
		m_xStates.PushItem(EAID_STEALHP, STATE_FOREVER, nStealHPPercent);
	}
	if(nNearPalsyPercent != 0)
	{
		m_xStates.PushItem(EAID_NEARPALSY, STATE_FOREVER, nNearPalsyPercent);
	}

	item.HP = GetObject_HP();
	item.maxHP = GetObject_MaxHP();
	item.MP = GetObject_MP();
	item.maxMP = GetObject_MaxMP();
	item.EXPR = GetObject_Expr();
	item.maxEXPR = GetObject_MaxExpr();
	item.level = GetObject_Level();
	item.sex = GetObject_Sex();
	item.id = GetObject_ID();
	//strcpy(item.name, m_stData.stAttrib.name);
	ObjectValid::GetItemName(&m_stData.stAttrib, item.name);

	int n7MinAdd = GetObject_Level() / 5;
	int n7MaxAdd = n7MinAdd + 1;
	int n14MinAdd = GetObject_Level() / 10;
	int n14MaxAdd = n14MinAdd + 1;

	if(m_stData.bJob == 0)
	{
		item.DC += n7MinAdd;
		item.maxDC += n7MaxAdd;
		item.AC += n7MinAdd;
		item.maxAC += n7MaxAdd;
	}
	else if(m_stData.bJob == 1)
	{
		item.MC += n7MinAdd;
		item.maxMC += n7MaxAdd;
		item.MAC += n7MinAdd;
		item.maxMAC += n7MaxAdd;
	}
	else if(m_stData.bJob == 2)
	{
		item.SC += n7MinAdd;
		item.maxSC += n7MaxAdd;
		item.DC += n14MinAdd;
		item.maxDC += n14MaxAdd;
		item.AC += n14MinAdd;
		item.maxAC += n14MaxAdd;
		item.MAC += n14MinAdd;
		item.maxMAC += n14MaxAdd;
	}

	if(m_stData.bJob == 0)
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_BASESWORD);
		if(pMgc->bLevel == 1)
		{
			item.accuracy += 1;
		}
		else if(pMgc->bLevel == 2)
		{
			item.accuracy += 3;
		}
		else if(pMgc->bLevel == 3)
		{
			item.accuracy += 6;
		}

		pMgc = GetUserMagic(MEFF_BERSERKER);
		if(pMgc->bLevel == 1)
		{
			item.atkSpeed += 1;
			item.DC *= 1.1f;
			item.maxDC *= 1.3f;
			item.maxAC += 4;
			item.accuracy += 2;
		}
		else if(pMgc->bLevel == 2)
		{
			item.atkSpeed += 2;
			item.DC *= 1.2f;
			item.maxDC *= 1.4f;
			item.maxAC += 7;
			item.accuracy += 3;
		}
		else if(pMgc->bLevel == 3)
		{
			item.atkSpeed += 3;
			item.DC *= 1.3f;
			item.maxDC *= 1.5f;
			item.maxAC += 10;
			item.accuracy += 4;
		}
	}
	else if(m_stData.bJob == 2)
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_SPIRIT);
		if(pMgc->bLevel == 1)
		{
			item.accuracy += 1;
		}
		else if(pMgc->bLevel == 2)
		{
			item.accuracy += 2;
			item.hide += 1;
		}
		else if(pMgc->bLevel == 3)
		{
			item.accuracy += 3;
			item.hide += 2;
		}
	}

	ItemAttrib* pItem = GetEquip(PLAYER_ITEM_WEAPON);
	if(pItem &&
		GETITEMATB(pItem, Type) == ITEM_WEAPON)
	{
		int nLevel = GetGrowLevelFromDWORD(GETITEMATB(pItem, HP));

		if(nLevel != 0)
		{
			/*if(GetHeroJob() == 0)
			{
				item.maxDC += nLevel * 2;
			}
			else if(GetHeroJob() == 1)
			{
				item.maxMC += nLevel * 2;
			}
			else if(GetHeroJob() == 2)
			{
				item.maxSC += nLevel * 2;
			}*/

			item.lucky += nLevel / 3;

			m_xStates.PushItem(EAID_IGNOREAC, STATE_FOREVER, nLevel * 2);
		}
	}

	if(item.AC > VALID_MAX_ABILITY ||
		item.maxAC > VALID_MAX_ABILITY ||
		item.MAC > VALID_MAX_ABILITY ||
		item.maxMAC > VALID_MAX_ABILITY ||
		item.DC > VALID_MAX_ABILITY ||
		item.maxDC > VALID_MAX_ABILITY ||
		item.SC > VALID_MAX_ABILITY ||
		item.maxSC > VALID_MAX_ABILITY ||
		item.MC > VALID_MAX_ABILITY ||
		item.maxMC > VALID_MAX_ABILITY)
	{
		//GameWorld::GetInstance().Stop(STOP_HEROATTRIBERR);
#ifdef _DEBUG
		LOG(INFO) << "MAX ability error";
#endif
		//return;
	}

	if(bEncrypt)
	{
		ObjectValid::EncryptAttrib(&item);
	}
	m_stData.stAttrib = item;

	UpdateSuitAttrib();
	UpdateSuitSameLevelAttrib();

	LoginSvr_UpdatePlayerRank();
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::UpdateStoveAttrib()
{
	
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::UpdateSuitAttrib()
{
	ItemAttrib item = m_stData.stAttrib;
	if(!IsEncrypt())
	{
		return;
	}

	ObjectValid::DecryptAttrib(&item);
	//	Already cleared in RefleshAttrib
	//m_xStates.ClearForever();

	ItemAttrib suitAttrib;
	ZeroMemory(&suitAttrib, sizeof(ItemAttrib));

	std::list<const ItemExtraAttribList*> xSuitAttribInfoList;
	std::list<const ItemExtraAttribList*>::const_iterator begIter;
	std::list<const ItemExtraAttribList*>::const_iterator endIter;
	bool bExist = false;
	const ItemExtraAttribList* pCurAttribList = NULL;

	//	Get all suit attrib information
	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		//if(m_stEquip[i].type != ITEM_NO)
		if(GETITEMATB(&m_stEquip[i], Type) != ITEM_NO)
		{
			//if(m_equip[i].atkPalsy != 0)
			if(GETITEMATB(&m_stEquip[i], AtkPalsy) != 0)
			{
				pCurAttribList = GetGlobalSuitExtraAttrib(GETITEMATB(&m_stEquip[i], AtkPalsy));

				if(pCurAttribList != NULL)
				{
					begIter = xSuitAttribInfoList.begin();
					endIter = xSuitAttribInfoList.end();
					bExist = false;

					for(begIter;
						begIter != endIter;
						++begIter)
					{
						if(*begIter == pCurAttribList)
						{
							bExist = true;
							break;
						}
					}

					if(!bExist)
					{
						xSuitAttribInfoList.push_back(GetGlobalSuitExtraAttrib(GETITEMATB(&m_stEquip[i], AtkPalsy)));
					}
				}
			}
		}
	}

	//	Calculate every suit list on every item on body
	bool bActivePos[10] = {false};
	int nActiveNumber = 0;

	if(!xSuitAttribInfoList.empty())
	{
		begIter = xSuitAttribInfoList.begin();
		endIter = xSuitAttribInfoList.end();

		for(begIter;
			begIter != endIter;
			++begIter)
		{
			pCurAttribList = *begIter;
			ZeroMemory(bActivePos, sizeof(bActivePos));
			nActiveNumber = 0;

			if(NULL != pCurAttribList)
			{
				for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
				{
					ItemAttrib* pItem = &m_stEquip[i];

					/*if(pItem->type != ITEM_NO &&
						pItem->atkPalsy == pCurAttribList->nSuitID)*/
					if(GETITEMATB(pItem, Type) != ITEM_NO &&
						GETITEMATB(pItem, AtkPalsy) == pCurAttribList->nSuitID)
					{
						for(int j = 0; j < 10; ++j)
						{
							if(/*pCurAttribList->nSuitEquipID[j] == pItem->id &&*/
								GETITEMATB(pItem, ID) == pCurAttribList->nSuitEquipID[j] &&
								!bActivePos[j] &&
								pCurAttribList->nSuitEquipID[j] != 0)
							{
								bActivePos[j] = true;
								++nActiveNumber;
								break;
							}
						}
					}
				}

				int nActiveAttribNumber = 0;
				int nActiveMax = 0;

				if(nActiveNumber != 0)
				{
					//	Activate some suit attrib,then update the player's attrib
					ZeroMemory(&suitAttrib, sizeof(ItemAttrib));

					for(int i = 0; i < MAX_EXTRAATTIRB; ++i)
					{
						if(nActiveNumber >= pCurAttribList->nActiveSum[i] &&
							pCurAttribList->nActiveSum[i] > nActiveMax)
						{
							nActiveMax = pCurAttribList->nActiveSum[i];
							nActiveAttribNumber = pCurAttribList->nActiveAttribSum[i];
						}
					}

					//	Now we can calculate the attrib that the suit bring to us
					for(int i = 0; i < nActiveAttribNumber; ++i)
					{
						int nCurValue = pCurAttribList->stExtraAttrib[i].nAttribValue;

						switch(pCurAttribList->stExtraAttrib[i].nAttribID)
						{
						case EAID_DC:
							{
								suitAttrib.DC += HIWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
								suitAttrib.maxDC += LOWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);

								item.DC += HIWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
								item.maxDC += LOWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
							}break;
						case EAID_AC:
							{
								suitAttrib.AC += HIWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
								suitAttrib.maxAC += LOWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);

								item.AC += HIWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
								item.maxAC += LOWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
							}break;
						case EAID_MAC:
							{
								suitAttrib.MAC += HIWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
								suitAttrib.maxMAC += LOWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);

								item.MAC += HIWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
								item.maxMAC += LOWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
							}break;
						case EAID_MC:
							{
								suitAttrib.MC += HIWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
								suitAttrib.maxMC += LOWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);

								item.MC += HIWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
								item.maxMC += LOWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
							}break;
						case EAID_SC:
							{
								suitAttrib.SC += HIWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
								suitAttrib.maxSC += LOWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);

								item.SC += HIWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
								item.maxSC += LOWORD(pCurAttribList->stExtraAttrib[i].nAttribValue);
							}break;
						case EAID_LUCKY:
							{
								suitAttrib.lucky += pCurAttribList->stExtraAttrib[i].nAttribValue;

								item.lucky += pCurAttribList->stExtraAttrib[i].nAttribValue;
							}break;
						case EAID_ATKSPEED:
							{
								suitAttrib.atkSpeed += pCurAttribList->stExtraAttrib[i].nAttribValue;

								item.atkSpeed += pCurAttribList->stExtraAttrib[i].nAttribValue;
							}break;
						case EAID_ACCURACY:
							{
								suitAttrib.accuracy += nCurValue;

								item.accuracy += nCurValue;
							}break;
						case EAID_MOVESPEED:
							{
								suitAttrib.moveSpeed += nCurValue;

								item.moveSpeed += nCurValue;
							}break;
						case EAID_MAGICITEM:
							{
								if(m_xStates.GetStateValue(EAID_MAGICITEM) != 0)
								{
									m_xStates.PushItem(EAID_MAGICITEM, STATE_FOREVER, nCurValue + m_xStates.GetStateValue(EAID_MAGICITEM));
								}
								else
								{
									m_xStates.PushItem(EAID_MAGICITEM, STATE_FOREVER, nCurValue);
								}
							}break;
						case EAID_MOUNTAIN:
							{
								if(m_xStates.GetStateValue(EAID_MOUNTAIN) != 0)
								{
									m_xStates.PushItem(EAID_MOUNTAIN, STATE_FOREVER, nCurValue + m_xStates.GetStateValue(EAID_MOUNTAIN));
								}
								else
								{
									m_xStates.PushItem(EAID_MOUNTAIN, STATE_FOREVER, nCurValue);
								}
							}break;
						case EAID_FARPALSY:
							{
								if(m_xStates.GetStateValue(EAID_FARPALSY) != 0)
								{
									m_xStates.PushItem(EAID_FARPALSY, STATE_FOREVER, nCurValue + m_xStates.GetStateValue(EAID_FARPALSY));
								}
								else
								{
									m_xStates.PushItem(EAID_FARPALSY, STATE_FOREVER, nCurValue);
								}
							}break;
						case EAID_SUCKDAMAGE:
							{
								if(m_xStates.GetStateValue(EAID_SUCKDAMAGE) != 0)
								{
									m_xStates.PushItem(EAID_SUCKDAMAGE, STATE_FOREVER, nCurValue + m_xStates.GetStateValue(EAID_SUCKDAMAGE));
								}
								else
								{
									m_xStates.PushItem(EAID_SUCKDAMAGE, STATE_FOREVER, nCurValue);
								}
							}break;
						case EAID_CRITICALPEC:
							{
								if(m_xStates.GetStateValue(EAID_CRITICALPEC) != 0)
								{
									m_xStates.PushItem(EAID_CRITICALPEC, STATE_FOREVER, nCurValue + m_xStates.GetStateValue(EAID_CRITICALPEC));
								}
								else
								{
									m_xStates.PushItem(EAID_CRITICALPEC, STATE_FOREVER, nCurValue);
								}
							}break;
						default:
							{
								int nAttribID = pCurAttribList->stExtraAttrib[i].nAttribID;

								if(m_xStates.GetStateValue(nAttribID) != 0)
								{
									m_xStates.PushItem(nAttribID, STATE_FOREVER, nCurValue + m_xStates.GetStateValue(nAttribID));
								}
								else
								{
									m_xStates.PushItem(nAttribID, STATE_FOREVER, nCurValue);
								}
							}break;
						}
					}

#ifdef _DEBUG
					LOG(INFO) << "SuitID[" << pCurAttribList->nSuitID << "] AC:" << suitAttrib.AC << "-" << suitAttrib.maxAC << "|MAC:" << suitAttrib.MAC << "-" << suitAttrib.maxMAC
						<< "|DC:" << suitAttrib.DC << "-" << suitAttrib.maxDC << "|MC:" << suitAttrib.MC << "-" << suitAttrib.maxMC << "|SC:" << suitAttrib.SC << "-" << suitAttrib.maxSC
						<< "|ACCURACY:" << (int)suitAttrib.accuracy << "|LUCKY:" << (int)suitAttrib.lucky << "|ATKSPEED:" << (int)suitAttrib.atkSpeed << "|MOVESPEED:" << (int)suitAttrib.moveSpeed;

					LOG(INFO) << "MAGICITEM:" << m_xStates.GetMagicItemAddition() << "|MOUNTAIN:" << m_xStates.GetDefStruckAddition() << "SUCKDAMAGE:" << m_xStates.GetStateValue(EAID_SUCKDAMAGE);
#endif
				}
			}
		}
	}

	ObjectValid::EncryptAttrib(&item);
	m_stData.stAttrib = item;
}

void HeroObject::UpdateSuitSameLevelAttrib()
{
	int nExtraSuitType = 0;

	int nSuitCount[9] = {0};

	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		ItemAttrib* pItem = &m_stEquip[i];
		//if(m_stEquip[i].type != ITEM_NO)
		if(GETITEMATB(pItem, Type) != ITEM_NO)
		{
			//if(m_equip[i].atkPalsy != 0)
			int nQualityIndex = GetItemUpgrade(GETITEMATB(pItem, Level));

			if (0 < nQualityIndex &&
				nQualityIndex < 9)
			{
				nSuitCount[nQualityIndex]++;
			}
		}
	}

	//	check active
	for (int i = 0; i < 9; ++i)
	{
		if (nSuitCount[i] >= 5)
		{
			nExtraSuitType = i;
			break;
		}
	}

	if (nExtraSuitType != m_nExtraSuitType)
	{
		m_nExtraSuitType = nExtraSuitType;

		//SendStatusInfo();
	}
}
//////////////////////////////////////////////////////////////////////////
/*
void HeroObject::ResetSupply()
{
	/ *for(int i = 0; i < HP_SUPPLY_NUMBER + MP_SUPPLY_NUMBER; ++i)
	{
		m_dwSupply[i] = 0;
	}* /
	ZeroMemory(m_dwSupply, sizeof(m_dwSupply));
}*/
//////////////////////////////////////////////////////////////////////////
int HeroObject::GetBagEmptySum()
{
	int nCounter = 0;
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
			m_xBag[i].tag != ITEMTAG_INQUERY)
		{
			++nCounter;
		}
	}
	return nCounter - GetAssistEmptySum();
}
int HeroObject::GetAssistEmptySum()
{
	return 0;

	if(m_nAssistItemSum >= HERO_ASSISTBAG_SIZE_CUR ||
		m_nAssistItemSum < 0)
	{
		return 0;
	}
	return HERO_ASSISTBAG_SIZE_CUR - m_nAssistItemSum;
}
int HeroObject::GetAllEmptySum()
{
	int nCounter = 0;
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
			m_xBag[i].tag != ITEMTAG_INQUERY)
		{
			++nCounter;
		}
	}
	return nCounter;
}
//////////////////////////////////////////////////////////////////////////
/*
bool HeroObject::AddDrugState(int _total, int _type)
{
	if(_type == 0)
	{
		for(int i = 0; i < HP_SUPPLY_NUMBER; ++i)
		{
			if(m_dwSupply[i] == 0)
			{
				m_dwSupply[i] = MAKELONG(_total, _total / 5);
				return true;
			}
		}
	}
	else if(_type == 1)
	{
		for(int i = HP_SUPPLY_NUMBER; i < HP_SUPPLY_NUMBER + MP_SUPPLY_NUMBER; ++i)
		{
			if(m_dwSupply[i] == 0)
			{
				m_dwSupply[i] = MAKELONG(_total, _total / 5);
				return true;
			}
		}
	}
	return false;
}*/
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseDrugItem(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, Type) == ITEM_COST &&
		GETITEMATB(_pItem, Curse) == 0)
	{
		//	First check if the drug is in cool status
		DWORD dwCoolTime = GETITEMATB(_pItem, Hide) * 1000;
		int nItemID = GETITEMATB(_pItem, ID);

		//	大补丸全部映射到一个ID上去
		if(nItemID >= 1231 &&
			nItemID <= 1233)
		{
			nItemID = 1231;
		}
		/*if(dwCoolTime != 0)
		{
			if(!DrugDelayManager::GetInstance()->CanUse(GETITEMATB(_pItem, ID)))
			{
				return false;
			}
			if(!DrugDelayManager::GetInstance()->PushDrugStatus(GETITEMATB(_pItem, ID), dwCoolTime))
			{
				return false;
			}
		}*/
		if(0 != dwCoolTime &&
			!CanUseCoolDownDrug(nItemID, dwCoolTime))
		{
			return false;
		}

		if(GETITEMATB(_pItem, AtkSpeed) == 0)
		{
			PkgPlayerUpdateCostNtf ntf;
			ntf.uTargetId = GetID();
			ntf.dwTag = _pItem->tag;
			ntf.nNumber = 0;
			ntf.bSelfUse = true;
			SendPacket(ntf);

			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);

			return true;
		}

		if(GETITEMATB(_pItem, Lucky) == 0)
		{
			if(GETITEMATB(_pItem, HP) > 0)
			{
				for(int i = 0; i < HP_SUPPLY_NUMBER; ++i)
				{
					if(m_dwSupply[i] == 0)
					{
						m_dwSupply[i] = MAKELONG(GETITEMATB(_pItem, HP), GETITEMATB(_pItem, HP) / 5);
						bUsed = true;
						break;
					}
				}
			}
			if(GETITEMATB(_pItem, MP) > 0)
			{
				for(int i = HP_SUPPLY_NUMBER; i < HP_SUPPLY_NUMBER + MP_SUPPLY_NUMBER; ++i)
				{
					if(m_dwSupply[i] == 0)
					{
						m_dwSupply[i] = MAKELONG(GETITEMATB(_pItem, MP), GETITEMATB(_pItem, MP) / 5);
						bUsed = true;
						break;
					}
				}
			}
		}
		else if(GETITEMATB(_pItem, Lucky) == 1)
		{
			int nPreHP =GetObject_HP();
			int nPreMP = GetObject_MP();

			IncHP(GETITEMATB(_pItem, HP));
			m_pValid->IncHP(GETITEMATB(_pItem, HP));
			IncMP(GETITEMATB(_pItem, MP));
			m_pValid->IncMP(GETITEMATB(_pItem, MP));
			PkgPlayerUpdateAttribNtf ntf;
			ntf.uTargetId = GetID();
			ntf.bType = UPDATE_HP;
			ntf.dwParam = GetObject_HP();
			if(nPreHP != ntf.dwParam)
			{
				SendPacket(ntf);
			}
			
			ntf.bType = UPDATE_MP;
			ntf.dwParam =GetObject_MP();
			if(nPreMP != ntf.dwParam)
			{
				SendPacket(ntf);
			}
			bUsed = true;
		}
		else if(GETITEMATB(_pItem, Lucky) == 2)
		{
			//	按比例加
			int nPreHP =GetObject_HP();
			int nPreMP = GetObject_MP();
			int nAddHP = GetObject_MaxHP() * GETITEMATB(_pItem, HP) / 100;
			int nAddMP = GetObject_MaxMP() * GETITEMATB(_pItem, MP) / 100;

			IncHP(nAddHP);
			m_pValid->IncHP(nAddHP);
			IncMP(nAddMP);
			m_pValid->IncMP(nAddMP);
			PkgPlayerUpdateAttribNtf ntf;
			ntf.uTargetId = GetID();
			ntf.bType = UPDATE_HP;
			ntf.dwParam = GetObject_HP();
			if(nPreHP != ntf.dwParam)
			{
				SendPacket(ntf);
			}

			ntf.bType = UPDATE_MP;
			ntf.dwParam =GetObject_MP();
			if(nPreMP != ntf.dwParam)
			{
				SendPacket(ntf);
			}
			bUsed = true;
		}

		if(bUsed)
		{
			//	记录下使用的时间
			if(0 != dwCoolTime)
			{
				SetLastDrugUseTime(nItemID, GetTickCount());
			}
		}
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		ntf.bSelfUse = true;
		ntf.uTargetId = GetID();
		SendPacket(ntf);

		if(nLeftSum == 0)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
		/*PkgPlayerUseItemAck ack;
		ack.dwUsage = USE_DRUG;
		ack.dwTag = _pItem->tag;
		ack.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		//SendBuffer(GetUserIndex(), &g_xThreadBuffer);
		SendPlayerBuffer(g_xThreadBuffer);*/
	}
	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
/*
bool HeroObject::UseLandScroll(ItemAttrib* _pItem)
{
	int nDestMapID = GETITEMATB(_pItem, Lucky);
}*/
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseSummonItem(ItemAttrib* _pItem)
{
	int nMonsterID = GETITEMATB(_pItem, MaxDC);
	int nHeroX = GetUserData()->wCoordX;
	int nHeroY = GetUserData()->wCoordY;
	int nTargetX = -1;
	int nTargetY = -1;
	bool bUsed = false;

	if(GetLocateScene()->GetMonsterSum(nMonsterID) > 2)
	{
		return false;
	}
	if(GETITEMATB(_pItem, ID) == 420)
	{
		if(GetLocateScene()->GetMapID() != 8)
		{
			return false;
		}
	}
	if(GETITEMATB(_pItem, ID) == 419)
	{
		//if(GetLocateScene()->GetMapID() != 31)
		{
			return false;
		}
	}
	if(GETITEMATB(_pItem, ID) == 1229)
	{
		if(GetLocateScene()->GetMapID() != 31)
		{
			SendSystemMessage("只能在寒冰王座召唤地狱深处的恶魔");
			return false;
		}
		else
		{
			int nPosX = GetUserData()->wCoordX;
			int nPosY = GetUserData()->wCoordY;
			int nOftX = nPosX - 104;
			int nOftY = nPosY - 91;

			if(abs(nOftX) > 8 ||
				abs(nOftY) > 8)
			{
				SendSystemMessage("请在(104,91)附近使用符咒召唤恶魔");
				return false;
			}
			else
			{
				GetLocateScene()->BroadcastChatMessage("地狱深处的恶魔已唤醒", 1);
			}
		}
	}
	else if(GETITEMATB(_pItem, ID) == 1238)
	{
		if(GetLocateScene()->GetMapID() != 52)
		{
			SendSystemMessage("只能在火龙神殿使用");
			return false;
		}
		else
		{
			if(GetLocateScene()->GetMonsterSum(nMonsterID) > 0)
			{
				SendSystemMessage("已使用");
				return false;
			}
			nTargetX = 82;
			nTargetY = 42;
		}
	}

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(nTargetX != -1 &&
		nTargetY != -1)
	{
		GetLocateScene()->CreateMonster(nMonsterID, nTargetX, nTargetY);
		bUsed = true;
	}
	else
	{
		for(int i = 0; i < 8; ++i)
		{
			nTargetX = nHeroX + g_nMoveOft[i * 2];
			nTargetY = nHeroY + g_nMoveOft[i * 2 + 1];

			if(GetLocateScene()->CanThrough(nTargetX, nTargetY))
			{
				GetLocateScene()->CreateMonster(nMonsterID, nTargetX, nTargetY);
				bUsed = true;
				break;
			}
		}
	}

	if(bUsed)
	{
		/*PkgPlayerUseItemAck ack;
		ack.dwUsage = USE_DRUG;
		ack.dwTag = _pItem->tag;
		ack.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		//SendBuffer(GetUserIndex(), &g_xThreadBuffer);
		SendPlayerBuffer(g_xThreadBuffer);*/
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseLotteryItem(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 6)
	{
		int nGetMoney = 0;

		int nBaseMoney = GETITEMATB(_pItem, Lucky);
		int nMinMoney = nBaseMoney * 200;
		int nMaxMoney = nBaseMoney * 2000;

		if(nBaseMoney > 5 ||
			nBaseMoney == 0)
		{
			return false;
		}

		nGetMoney = nMinMoney + rand() % nMaxMoney;

		if(nGetMoney > 0)
		{
			if(GameWorld::GetInstance().GetFinnalExprMulti() == 1)
			{
				nGetMoney = 0;
			}

			AddMoney(nGetMoney);

			/*PkgPlayerUpdateAttribNtf ppuan;
			ppuan.uTargetId = GetID();
			ppuan.bType = UPDATE_MONEY;
			ppuan.dwParam = GetMoney();
			SendPacket(ppuan);*/

			bUsed = true;
		}
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseFireworkItem(ItemAttrib* _pItem)
{
	bool bUsed = false;

	int nItemId = GETITEMATB(_pItem, ID);
	int nContinueTime = 60 * 60 * 1000;
#ifdef _DEBUG
	nContinueTime = 60 * 1 * 1000;
#endif

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 16)
	{
		PkgPlayerPlayAniAck ack;
		ack.uTargetId = GetID();

		if(nItemId == 1665)
		{
			if(GameWorld::GetInstance().GetExpFireworkTime() != 0)
			{
				SendSystemMessage("礼花已燃放");
				return false;
			}
			GameWorld::GetInstance().SetExpFireworkUserName(GetName());
			GameWorld::GetInstance().SetExpFireworkTime(GetTickCount() + nContinueTime);
			char szTip[256] = {0};
			sprintf(szTip, "玩家[%s]燃放了一个五彩礼花，整个服务器经验值提升0.5倍，持续一小时", GetName());
			GameSceneManager::GetInstance()->SendSystemNotifyAllScene(szTip);
			ack.wAniID = 1000;
		}
		else if(nItemId == 1676)
		{
			if(GameWorld::GetInstance().GetBurstFireworkTime() != 0)
			{
				SendSystemMessage("礼花已燃放");
				return false;
			}
			GameWorld::GetInstance().SetBurstFireworkUserName(GetName());
			GameWorld::GetInstance().SetBurstFireworkTime(GetTickCount() + nContinueTime);
			char szTip[256] = {0};
			sprintf(szTip, "玩家[%s]燃放了一个炫紫礼花，整个服务器爆率提升0.5倍，持续一小时", GetName());
			GameSceneManager::GetInstance()->SendSystemNotifyAllScene(szTip);
			ack.wAniID = 1001;
		}
		else if(nItemId == 1677)
		{
			if(GameWorld::GetInstance().GetMagicDropFireworkTime() != 0)
			{
				SendSystemMessage("礼花已燃放");
				return false;
			}
			GameWorld::GetInstance().SetMagicDropFireworkUserName(GetName());
			GameWorld::GetInstance().SetMagicDropFireworkTime(GetTickCount() + nContinueTime);
			char szTip[256] = {0};
			sprintf(szTip, "玩家[%s]燃放了一个魔法礼花，整个服务器魔法装备加成+8，持续一小时", GetName());
			GameSceneManager::GetInstance()->SendSystemNotifyAllScene(szTip);
			ack.wAniID = 1002;
		}
		else
		{
			return false;
		}

		for(int i = 0; i < 8; ++i)
		{
			WORD wX = GetUserData()->wCoordX + g_nMoveOft[i * 2] * 4;
			WORD wY = GetUserData()->wCoordY + g_nMoveOft[i * 2 + 1] * 4;
			ack.xPos.push_back(MAKELONG(wX, wY));
		}

		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseWeaponStyleResetCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 25)
	{
		int nStyle = 0;
		m_stExtAttrib.uWeaponLook = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_WeaponLook;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseWeaponStyleCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 24)
	{
		static int s_nWeaponStyle[] = {
			30,
			43,
			31,
			36,
			32,
			67,
			33,
			34,
			35,
			40,
			39,
			38,
			37,
			41,
			42,
			45,
			47,
			44,
			46,
			48,
			49,
			50,
			51,
			52,
			53,
			54,
			55,
			56,
			57,
			58,
			59,
			65,
			66,
			69,
			70,
			71,
			72,
			73,
			74,
			828,
			1405,
			1406,
			1407,
			1410,
			1420,
			1421
		};
		int nTableLength = nTableLength = sizeof(s_nWeaponStyle) / sizeof(s_nWeaponStyle[0]);

		int nStyle = m_stExtAttrib.uWeaponLook;
		int nCounter = 0;

		while(1)
		{
			int nRandom = rand() % nTableLength;
			if(s_nWeaponStyle[nRandom] != nStyle)
			{
				nStyle = s_nWeaponStyle[nRandom];
				break;
			}
			++nCounter;
		}

		g_xConsole.CPrint("Change weapon style cost:%d", nCounter);

		m_stExtAttrib.uWeaponLook = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_WeaponLook;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseClothStyleResetCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 23)
	{
		int nStyle = 0;
		m_stExtAttrib.uClothLook = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_ClothLook;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseNameFrameCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 26)
	{
		static int s_nNameFrameStyle[] = {
			43,
			46,
			49,
			52,
			55,
			58,
			61,
			64,
			67,
			70,
			73,
			76,
			79,
			82,
			85,
			88,
			91,
			94,
			97,
			100,
			103,
			106,
			109,
			112,
			115,
			118,
			121,
			124,
			127,
			130,
			133,
			136,
			139,
			142,
			145,
			148,
			151,
			154,
			157,
			160,
			163,
			166,
			169,
			172,
			175,
			178,
			181,
			184,
			187,
			190,
			193,
			196,
			199,
			202,
			205,
			208,
			211,
			214,
			217,
			220,
			223,
			226,
			229,
			232,
			235,
			238,
			241,
			244,
			247,
			250,
			253,
			256,
			259,
			262,
			265,
			268,
			271,
			274,
			277,
			280,
			283,
			286,
			289,
			292,
			295,
			298,
			301,
			304,
			307
		};
		int nTableLength = nTableLength = sizeof(s_nNameFrameStyle) / sizeof(s_nNameFrameStyle[0]);

		int nStyle = m_stExtAttrib.uNameFrame;
		int nStyleCp = nStyle;
		int nCounter = 0;

		while(1)
		{
			int nRandom = rand() % nTableLength;
			if(s_nNameFrameStyle[nRandom] != nStyle)
			{
				nStyle = s_nNameFrameStyle[nRandom];
				break;
			}
			++nCounter;
		}

		g_xConsole.CPrint("Change name frame style cost:%d", nCounter);

		m_stExtAttrib.uNameFrame = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_NameFrame;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseNameFrameResetCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 27)
	{
		int nStyle = 0;
		m_stExtAttrib.uNameFrame = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_NameFrame;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseChatFrameCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 28)
	{
		static int s_nChatFrameStyle[] = {
			320,
			330,
			340,
			350,
			360,
			370,
			380,
			390,
			400,
			410,
			420,
			430,
			440,
			450,
			460,
			470,
			480,
			490,
			500,
			510,
			520,
			530,
			560,
			570,
			580,
			590,
			600,
			610,
			620,
			630,
			640,
			650
		};
		int nTableLength = nTableLength = sizeof(s_nChatFrameStyle) / sizeof(s_nChatFrameStyle[0]);

		int nStyle = m_stExtAttrib.uChatFrame;
		int nStyleCp = nStyle;
		int nCounter = 0;

		while(1)
		{
			int nRandom = rand() % nTableLength;
			if(s_nChatFrameStyle[nRandom] != nStyle)
			{
				nStyle = s_nChatFrameStyle[nRandom];
				break;
			}
			++nCounter;
		}

		g_xConsole.CPrint("Change chat frame style cost:%d", nCounter);

		m_stExtAttrib.uChatFrame = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_ChatFrame;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseChatFrameResetCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 29)
	{
		int nStyle = 0;
		m_stExtAttrib.uChatFrame = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_ChatFrame;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseChestKey(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 30)
	{
		/*if (GETITEMATB(_pItem, ID) == 1181)
		{
			if (CountItem())
			bUsed = true;
		}
		else if (GETITEMATB(_pItem, ID) == 1182)
		{
			bUsed = true;
		}
		else if (GETITEMATB(_pItem, ID) == 1183)
		{
			bUsed = true;
		}
		else if (GETITEMATB(_pItem, ID) == 1184)
		{
			bUsed = true;
		}*/
		// Call lua
		if (GETITEMATB(_pItem, ID) >= 1181 &&
			GETITEMATB(_pItem, ID) <= 1184)
		{
			lua_State* L = GameWorld::GetInstance().GetLuaState();
			lua_getglobal(L, "OnUseChestKey");
			if (lua_isnil(L, -1))
			{
				// Not found lua function
				lua_pop(L, 1);
			}
			else
			{
				tolua_pushusertype(L, this, "HeroObject");
				lua_pushnumber(L, GETITEMATB(_pItem, ID));
				if (0 != lua_pcall(L, 2, 1, 0))
				{
					// Call failed
					LOG(ERROR) << "Failed to call OnUseChestKey : " << lua_tostring(L, -1);
					lua_pop(L, 1);
				}
				else
				{
					bUsed = lua_toboolean(L, -1);
					lua_pop(L, 1);
				}
			}
		}
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseClothStyleCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 22)
	{
		static int s_nClothStyleMale[] = {60,61,62,63,64,85,86,87,590,595,596,869,871,1394,1396,1398};
		static int s_nClothStyleFemale[] = {80,81,82,83,84,88,89,90,591,600,606,870,872,1395,1397,1399};

		int* pClothTable = NULL;
		int nTableLength = 0;

		if(GetObject_Sex() == 1)
		{
			pClothTable = s_nClothStyleMale;
			nTableLength = sizeof(s_nClothStyleMale) / sizeof(s_nClothStyleMale[0]);
		}
		else if(GetObject_Sex() == 2)
		{
			pClothTable = s_nClothStyleFemale;
			nTableLength = sizeof(s_nClothStyleFemale) / sizeof(s_nClothStyleFemale[0]);
		}

		if(NULL == pClothTable)
		{
			return false;
		}

		int nStyle = m_stExtAttrib.uClothLook;

		int nCounter = 0;

		while(1)
		{
			int nRandom = rand() % nTableLength;
			if(pClothTable[nRandom] != nStyle)
			{
				nStyle = pClothTable[nRandom];
				break;
			}
			++nCounter;
		}

		g_xConsole.CPrint("Change cloth style cost:%d", nCounter);

		m_stExtAttrib.uClothLook = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_ClothLook;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseWingStyleResetCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 14)
	{
		int nStyle = 0;
		m_stExtAttrib.uWing = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_Wing;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseWingStyleCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 11)
	{
		static int s_nWingStyle[] = {0,1,2,3,4,5,6,7,8,9,10,13,14,15,17};

		int nStyle = m_stExtAttrib.uHair;

		int nCounter = 0;

		while(1)
		{
			int nRandom = rand() % (sizeof(s_nWingStyle) / sizeof(s_nWingStyle[0]));
			if(s_nWingStyle[nRandom] != nStyle)
			{
				nStyle = s_nWingStyle[nRandom];
				break;
			}
			++nCounter;
		}

		g_xConsole.CPrint("Change wing style cost:%d", nCounter);

		m_stExtAttrib.uWing = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_Wing;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UsePackItem(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 12)
	{
		int nDestItemId = GETITEMATB(_pItem, MaxDC);
		int nDestItemCount = GETITEMATB(_pItem, Lucky);

		if(nDestItemCount == 0)
		{
			return true;
		}

		int nBagNeed = 1;

		ItemAttrib item = {0};
		if(!GetRecordInItemTable(nDestItemId, &item))
		{
			return true;
		}

		if(item.type == ITEM_COST)
		{
			nBagNeed = nDestItemCount / GRID_MAX;
			if(0 == nBagNeed)
			{
				nBagNeed = 1;
			}
		}
		else
		{
			nBagNeed = nDestItemCount;
		}

		if(0 == nBagNeed)
		{
			return true;
		}

		//	进行广播
		if(GetBagEmptySum() < nBagNeed)
		{
			SendQuickMessage(QMSG_NOBAGROOM);
			return true;
		}

		for(int i = 0; i < nDestItemCount; ++i)
		{
			AddItem(nDestItemId);
		}
		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseHairStyleResetCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 13)
	{
		int nStyle = 0;
		m_stExtAttrib.uHair = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_Hair;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseHairStyleCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 8)
	{
		static int s_nMaleStyle[] = {0, 5, 7, 9, 11, 13, 15, 17};
		static int s_nFemaleStyle[] = {0, 1, 4, 6, 8, 10, 12, 14, 16, 18};

		int nStyle = m_stExtAttrib.uHair;
		int* pStyleArray = s_nMaleStyle;
		int nStyleSize = sizeof(s_nMaleStyle) / sizeof(s_nMaleStyle[0]);

		int nSex = GetObject_Sex();
		if(nSex != 1)
		{
			pStyleArray = s_nFemaleStyle;
			nStyleSize = sizeof(s_nFemaleStyle) / sizeof(s_nFemaleStyle[0]);
		}

		int nCounter = 0;

		while(1)
		{
			int nRandom = rand() % (nStyleSize);
			if(pStyleArray[nRandom] != nStyle)
			{
				nStyle = pStyleArray[nRandom];
				break;
			}
			++nCounter;
		}

		g_xConsole.CPrint("Change hair style cost:%d", nCounter);

		m_stExtAttrib.uHair = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_Hair;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseChatColorCard(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 15)
	{
		static unsigned int s_nColorStyle[] = 
		{
			0xffBF3EFF,
			0xffffffff,
			0xffff0000,
			0xff00ff00,
			0xff0000ff,
			0xffffff00,
			0xffFFFAFA,
			0xffBBFFFF,
			0xffF5F5F5,
			0xffFFDEAD,
			0xff97FFFF,
			0xffFFE4E1,
			0xff4EEE94,
			0xff483D8B,
			0xff4169E1,
			0xff76EE00,
			0xffB0C4DE,
			0xffFFF68F,
			0xff98FB98,
			0xffCDAD00,
			0xffCD5C5C,
			0xffFF8247,
			0xffB22222,
			0xffEE9A49,
			0xffFF1493,
			0xffFF69B4,
			0xffFF00FF,
			0xffEE9572,
			0xffFF1493,
			0xff4876FF,
			0xff1E90FF,
			0xffFFE1FF,
			0xffE0FFFF,
			0xff1E90FF
		};

		int nStyle = m_stExtAttrib.uChatColor;

		int nCounter = 0;

		while(1)
		{
			int nRandom = rand() % (sizeof(s_nColorStyle) / sizeof(s_nColorStyle[0]));
			if(s_nColorStyle[nRandom] != nStyle)
			{
				nStyle = s_nColorStyle[nRandom];
				break;
			}
			++nCounter;
		}

		g_xConsole.CPrint("Change color style cost:%d", nCounter);

		m_stExtAttrib.uChatColor = nStyle;

		//	进行广播
		PkgPlayerExtendAttribNot not;
		not.uTargetId = GetID();
		not.xAttrib.resize(1);
		not.xAttrib[0].uType = kExtendAttrib_ChatColor;
		not.xAttrib[0].nValue = nStyle;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		bUsed = true;
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseMoneyItem(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);

		return true;
	}

	if(GETITEMATB(_pItem, Curse) == 5)
	{
		int nGetMoney = GETITEMATB(_pItem, Lucky) * 1000000;
		nGetMoney *= 0.95;

		if(nGetMoney > 0)
		{
			AddMoney(nGetMoney);

			/*PkgPlayerUpdateAttribNtf ppuan;
			ppuan.uTargetId = GetID();
			ppuan.bType = UPDATE_MONEY;
			ppuan.dwParam = GetMoney();
			SendPacket(ppuan);*/

			bUsed = true;
		}
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseTreasureMap(ItemAttrib* _pItem)
{
	//	Get the instance map id
	int nInstanceMapID = GETITEMATB(_pItem, Hide);
	bool bRet = true;

	if(GetMapID() != GETITEMATB(_pItem, Accuracy))
	{
		bRet = false;
	}
	//	Check the position
	int nOftX = abs((int)GETITEMATB(_pItem, MaxAC) - (int)GetUserData()->wCoordX);
	int nOftY = abs((int)GETITEMATB(_pItem, AC) - (int)GetUserData()->wCoordY);

	if(nOftX >= 5 ||
		nOftY >= 5)
	{
		bRet = false;
	}

	if(false == bRet)
	{
		return false;
	}

	if(GETITEMATB(_pItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);
		return true;
	}

	int nRandom = rand() % 2;
	if(nInstanceMapID != 0)
	{
		GameScene* pInstanceScene = GameSceneManager::GetInstance()->GetScene(nInstanceMapID);
		if(NULL != pInstanceScene)
		{
			if(nRandom == 0)
			{
				//if(pInstanceScene->IsInstance())
				if(pInstanceScene->IsTreasureMap())
				{
					if(pInstanceScene->GetPlayerSum() == 0 &&
						pInstanceScene->GetSlaveSum() == 0)
					{
						//	OK
						int nLastTime = 1 * 60 * 1000;
						GetLocateScene()->CreateDoorEvent(nInstanceMapID, GETITEMATB(_pItem, MaxAC), GETITEMATB(_pItem, AC), GETITEMATB(_pItem, MaxDC), GETITEMATB(_pItem, DC), nLastTime);
						pInstanceScene->DeleteAllMonster();
						pInstanceScene->DeleteAllItem();
						pInstanceScene->ResetGiveReward();

						lua_State* pLua = pInstanceScene->GetLuaState();
						lua_getglobal(pLua, "OnStartInstance");
						tolua_pushusertype(pLua, pInstanceScene, "GameScene");
						int nRet = lua_pcall(pLua, 1, 0, 0);
						if(0 != nRet)
						{
							LOG(ERROR) << lua_tostring(pLua, -1);
							lua_pop(pLua, 1);
						}

						int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
						--nLeftSum;

						PkgPlayerUpdateCostNtf ntf;
						ntf.uTargetId = GetID();
						ntf.dwTag = _pItem->tag;
						ntf.nNumber = nLeftSum;
						SendPacket(ntf);

						PkgPlayerShowDoorAniAck dack;
						dack.wMgcID = MEFF_DOOR;
						dack.uTargetId = GetID();
						dack.uUserId = nLastTime;
						dack.wPosX = GETITEMATB(_pItem, MaxAC);
						dack.wPosY = GETITEMATB(_pItem, AC);
						SendPacket(dack);

						if(0 == nLeftSum)
						{
							ZeroMemory(_pItem, sizeof(ItemAttrib));
							ObjectValid::EncryptAttrib(_pItem);
						}
						else
						{
							SETITEMATB(_pItem, AtkSpeed, nLeftSum);
						}

						return true;
					}
					else
					{
						SendSystemMessage("[提示]请等待玩家结束");
						return false;
					}
				}
			}
			else
			{
				//	...
				int nPosX = GetUserData()->wCoordX;
				int nPosY = GetUserData()->wCoordY;

				for(int i = 0; i < 8; i += 4)
				{
					if(GetLocateScene()->CanThrough(nPosX + 3 * g_nMoveOft[i * 2],
						nPosY + 3 * g_nMoveOft[i * 2 + 1]))
					{
						GetLocateScene()->CreateMonster(45, nPosX + 3 * g_nMoveOft[i * 2],
							nPosY + 3 * g_nMoveOft[i * 2 + 1]);
					}
				}

				int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
				--nLeftSum;

				PkgPlayerUpdateCostNtf ntf;
				ntf.uTargetId = GetID();
				ntf.dwTag = _pItem->tag;
				ntf.nNumber = nLeftSum;
				SendPacket(ntf);

				if(0 == nLeftSum)
				{
					ZeroMemory(_pItem, sizeof(ItemAttrib));
					ObjectValid::EncryptAttrib(_pItem);
				}
				else
				{
					SETITEMATB(_pItem, AtkSpeed, nLeftSum);
				}

				return true;
			}
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseCostItem(ItemAttrib* _pItem)
{
	bool bUsed = false;

	if(GETITEMATB(_pItem, Type) == ITEM_COST)
	{
		int nCostType = GETITEMATB(_pItem, Curse);

		if(GETITEMATB(_pItem, Curse) == 0)
		{
			//	Drug
			return UseDrugItem(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 1)
		{
			//	Scroll
			return UseScrollItem(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 3)
		{
			if(GETITEMATB(_pItem, ID) == 858)
			{
				//	pandora's box
				PkgPlayerShowDlgAck ack;
				ack.bType = DLG_CUBE;
				ack.uTargetId = GetID();
				SendPacket(ack);
			}
			else if(GETITEMATB(_pItem, ID) == 1664)
			{
				PkgPlayerShowDlgAck ack;
				ack.bType = DLG_WORLDCHAT;
				ack.uTargetId = GetID();
				SendPacket(ack);
			}
		}
		else if(GETITEMATB(_pItem, Curse) == 2)
		{
			//	treasure map
			return UseTreasureMap(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 4)
		{
			//
			return UseSummonItem(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 5)
		{
			//	land scroll
			//return UseLandScroll(_pItem);
			//	Money
			return UseMoneyItem(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 6)
		{
			//	彩票
			return UseLotteryItem(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 8)
		{
			//	发型卡
			return UseHairStyleCard(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 11)
		{
			//	翅膀卡
			return UseWingStyleCard(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 12)
		{
			//	捆物品
			return UsePackItem(_pItem);
		}
		else if(nCostType == 13)
		{
			//	发型重置
			return UseHairStyleResetCard(_pItem);
		}
		else if(nCostType == 14)
		{
			//	翅膀重置
			return UseWingStyleResetCard(_pItem);
		}
		else if(nCostType == 15)
		{
			//	聊天颜色
			return UseChatColorCard(_pItem);
		}
		else if(nCostType == 16)
		{
			//	礼花
			return UseFireworkItem(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 22)
		{
			//	服装卡
			return UseClothStyleCard(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 23)
		{
			//	服装重置卡
			return UseClothStyleResetCard(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 24)
		{
			//	武器卡
			return UseWeaponStyleCard(_pItem);
		}
		else if(GETITEMATB(_pItem, Curse) == 25)
		{
			//	服装重置卡
			return UseWeaponStyleResetCard(_pItem);
		}
		else if(nCostType == 26)
		{
			//	名字卡
			return UseNameFrameCard(_pItem);
		}
		else if(nCostType == 27)
		{
			//	名字重置卡
			return UseNameFrameResetCard(_pItem);
		}
		else if(nCostType == 28)
		{
			//	聊天卡
			return UseChatFrameCard(_pItem);
		}
		else if(nCostType == 29)
		{
			//	聊天重置卡
			return UseChatFrameResetCard(_pItem);
		}
		else if(nCostType == 30)
		{
			//	聊天重置卡
			return UseChestKey(_pItem);
		}
	}
	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseBaleItem(ItemAttrib* _pItem)
{
	return false;

	int nLeft = GetAllEmptySum();
	if(nLeft >= 5)
	{
		/*DBOperationParam* pParam = new DBOperationParam;
		pParam->dwOperation = DO_QUERY_ITEMATTRIB;
		//	Head of GroundItem, so can transform to ItemAttrib
		pParam->dwParam[0] = (DWORD)_pItem;
		pParam->dwParam[1] = MAKELONG(GetMapID(), _pItem->maxDC);
		pParam->dwParam[2] = MAKELONG(IE_ADDBALEITEM, GetID());
		DBThread::GetInstance()->AsynExecute(pParam);*/

		PkgPlayerUseItemAck ack;
		ack.dwUsage = USE_DRUG;
		ack.dwTag = _pItem->tag;
		ack.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		//SendBuffer(GetUserIndex(), &g_xThreadBuffer);
		SendPlayerBuffer(g_xThreadBuffer);

		int nBaleItemID = GETITEMATB(_pItem, MaxDC);
		memset(_pItem, 0, sizeof(ItemAttrib));
		for(int i = 0; i < 6; ++i)
		{
			AddCostItem(nBaleItemID);
		}
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::LearnMagic(DWORD _dwMgcID, BYTE _bBookLevel)
{
	const UserMagic* pMgc = NULL;
	pMgc = GetUserMagic(_dwMgcID);
	if(NULL == pMgc)
	{
		return false;
	}

	if(pMgc->bLevel >= 7)
	{
		return false;
	}

	if(_bBookLevel == 0)
	{
		if(pMgc->bLevel >= 3)
		{
			return false;
		}
	}
	else if(_bBookLevel == 1)
	{
		if(pMgc->bLevel == 6)
		{
			return false;
		}
		//	大于等级三级 才能继续学
		if(pMgc->bLevel < 3)
		{
			return false;
		}
	}

#ifdef _DEBUG
	/*if(pMgc->bLevel == 0)
	{
		MagicInfo* pInfo = &g_xMagicInfoTable[_dwMgcID];
		if(pInfo->wLevel[0] != 0 &&
			m_stData.bJob == pInfo->bJob)
		{
			return AddUserMagic(_dwMgcID);
		}
		else
		{
			return false;
		}
	}
	else
	{
		if(pMgc->pInfo->wLevel[pMgc->bLevel] != 0 &&
			m_stData.bJob == pMgc->pInfo->bJob)
		{
			return UpgradeUserMagic(_dwMgcID);
		}
		else
		{
			return false;
		}
	}*/
	if(pMgc->bLevel == 0)
	{
		MagicInfo* pInfo = &g_xMagicInfoTable[_dwMgcID];
		if(GetObject_Level() >= pInfo->wLevel[0] &&
			pInfo->wLevel[0] != 0 &&
			m_stData.bJob == pInfo->bJob)
		{
			return AddUserMagic(_dwMgcID);
		}
		else
		{
			return false;
		}
	}
	else
	{
		MagicInfo* pInfo = &g_xMagicInfoTable[_dwMgcID];
		if(GetObject_Level() >= pInfo->wLevel[pMgc->bLevel] &&
			pMgc->pInfo->wLevel[pMgc->bLevel] != 0 &&
			m_stData.bJob == pMgc->pInfo->bJob)
		{
			return UpgradeUserMagic(_dwMgcID);
		}
		else
		{
			/*static char szTip[] = "Next level:";
			char szMsg[100];
			sprintf(szMsg, "%s:%d",
				szTip, pInfo->wLevel[pMgc->bLevel]);
			SendSystemMessage(szMsg);*/
			SendQuickMessage(QMSG_SKILLLEVELLOW, MAKELONG(_dwMgcID, pInfo->wLevel[pMgc->bLevel]));
			return false;
		}
	}
#else
	if(pMgc->bLevel == 0)
	{
		MagicInfo* pInfo = &g_xMagicInfoTable[_dwMgcID];
		if(GetObject_Level() >= pInfo->wLevel[0] &&
			pInfo->wLevel[0] != 0 &&
			m_stData.bJob == pInfo->bJob)
		{
			return AddUserMagic(_dwMgcID);
		}
		else
		{
			return false;
		}
	}
	else
	{
		MagicInfo* pInfo = &g_xMagicInfoTable[_dwMgcID];
		if(GetObject_Level() >= pInfo->wLevel[pMgc->bLevel] &&
			pMgc->pInfo->wLevel[pMgc->bLevel] != 0 &&
			m_stData.bJob == pMgc->pInfo->bJob)
		{
			return UpgradeUserMagic(_dwMgcID);
		}
		else
		{
			/*static char szTip[] = "Next level:";
			char szMsg[100];
			sprintf(szMsg, "%s:%d",
				szTip, pInfo->wLevel[pMgc->bLevel]);
			SendSystemMessage(szMsg);*/
			SendQuickMessage(QMSG_SKILLLEVELLOW, MAKELONG(_dwMgcID, pInfo->wLevel[pMgc->bLevel]));
			return false;
		}
	}
#endif
	
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::ReadBook(ItemAttrib* _pItem)
{
	UserMagic* pMgc = NULL;
	bool bUsed = false;
	DWORD dwMagic = MEFF_NONE;

	switch(GETITEMATB(_pItem, ID))
	{
	case 291:
		{
			//	基本剑术
			bUsed = LearnMagic(MEFF_BASESWORD, 0);
			dwMagic = MEFF_BASESWORD;
			if(bUsed)
			{
				RefleshAttrib();
			}
		}break;
	case 292:
		{
			//	小火球
			dwMagic = MEFF_SFIREBALL;
			bUsed = LearnMagic(MEFF_SFIREBALL, 0);
		}break;
	case 293:
		{
			//	治愈术
			dwMagic = MEFF_HEAL;
			bUsed = LearnMagic(MEFF_HEAL, 0);
		}break;
	case 294:
		{
			//	攻杀
			dwMagic = MEFF_GONGSHA;
			bUsed = LearnMagic(MEFF_GONGSHA, 0);
		}break;
	case 295:
		{
			//	刺杀
			dwMagic = MEFF_CISHA;
			bUsed = LearnMagic(MEFF_CISHA, 0);
		}break;
	case 299:
		{
			//	半月弯刀
			dwMagic = MEFF_BANYUE;
			bUsed = LearnMagic(MEFF_BANYUE, 0);
		}break;
	case 300:
		{
			//	烈火
			dwMagic = MEFF_LIEHUO;
			bUsed = LearnMagic(MEFF_LIEHUO, 0);
		}break;
	case 301:
		{
			//	大火球
			dwMagic = MEFF_LFIREBALL;
			bUsed = LearnMagic(MEFF_LFIREBALL, 0);
		}break;
	case 302:
		{
			//	爆裂火焰
			dwMagic = MEFF_BURSTFIRE;
			bUsed = LearnMagic(MEFF_BURSTFIRE, 0);
		}break;
	case 303:
		{
			//	烈火
			dwMagic = MEFF_ICEROAR;
			bUsed = LearnMagic(MEFF_ICEROAR, 0);
		}break;
	case 305:
		{
			//	雷电术
			dwMagic = MEFF_THUNDER;
			bUsed = LearnMagic(MEFF_THUNDER, 0);
		}break;
	case 306:
		{
			//	火墙
			dwMagic = MEFF_FIREWALL;
			bUsed = LearnMagic(MEFF_FIREWALL, 0);
		}break;
	case 307:
		{
			//	魔法盾
			dwMagic = MEFF_SHIELD;
			bUsed = LearnMagic(MEFF_SHIELD, 0);
		}break;
	case 308:
		{
			//	灵魂火符
			dwMagic = MEFF_FIRECHARM;
			bUsed = LearnMagic(MEFF_FIRECHARM, 0);
		}break;
	case 309:
		{
			//	神圣战甲术
			dwMagic = MEFF_CHARMAC;
			bUsed = LearnMagic(MEFF_CHARMAC, 0);
		}break;
	case 310:
		{
			//	隐身术
			dwMagic = MEFF_HIDE;
			bUsed = LearnMagic(MEFF_HIDE, 0);
		}break;
	case 311:
		{
			//	精神力战法
			dwMagic = MEFF_SPIRIT;
			bUsed = LearnMagic(MEFF_SPIRIT, 0);
			if(bUsed)
			{
				RefleshAttrib();
			}
		}break;
	case 315:
		{
			//	召唤术
			dwMagic = MEFF_SUMMON;
			bUsed = LearnMagic(MEFF_SUMMON, 0);
		}break;
	case 316:
		{
			//	施毒术
			dwMagic = MEFF_POISON;
			bUsed = LearnMagic(MEFF_POISON, 0);
		}break;
	case 317:
		{
			//	召唤骷髅
			dwMagic = MEFF_KULOU;
			bUsed = LearnMagic(MEFF_KULOU, 0);
		}break;
	case 318:
		{
			//	诱惑之光
			dwMagic = MEFF_TEMPT;
			bUsed = LearnMagic(MEFF_TEMPT, 0);
		}break;
	case 319:
		{
			//	稳如泰山
			dwMagic = MEFF_MOUNTAIN;
			bUsed = LearnMagic(MEFF_MOUNTAIN, 0);
		}break;
	case 321:
		{
			//	狂战士
			dwMagic = MEFF_BERSERKER;
			bUsed = LearnMagic(MEFF_BERSERKER, 0);
			if(bUsed)
			{
				RefleshAttrib();
			}
		}break;
	case 322:
		{
			//	狮子吼
			dwMagic = MEFF_LIONROAR;
			bUsed = LearnMagic(MEFF_LIONROAR, 0);
		}break;
	case 324:
		{
			//	寒冰掌
			dwMagic = MEFF_ICEPALM;
			bUsed = LearnMagic(MEFF_ICEPALM, 0);
		}break;
	case 323:
		{
			//	进阶召唤
			dwMagic = MEFF_SUPERSUMMON;
			bUsed = LearnMagic(MEFF_SUPERSUMMON, 0);
		}break;
	case 304:
		{
			//	地狱雷光
			dwMagic = MEFF_HELLTHUNDER;
			bUsed = LearnMagic(MEFF_HELLTHUNDER, 0);
		}break;
	case 325:
		{
			//	炙炎剑法
			dwMagic = MEFF_SLIEHUO;
			bUsed = LearnMagic(MEFF_SLIEHUO, 0);
		};break;
	case 326:
		{
			//	神兽天御
			dwMagic = MEFF_SUMMONAC;
			bUsed = LearnMagic(MEFF_SUMMONAC, 0);
		}break;
	case 327:
		{
			//	灭天火
			dwMagic = MEFF_SKYFIRE;
			bUsed = LearnMagic(MEFF_SKYFIRE, 0);
		}break;
	case 329:
		{
			//	火流星
			dwMagic = MEFF_FIRESHOWER;
			bUsed = LearnMagic(MEFF_FIRESHOWER, 0);
		}break;
	case 330:
		{
			//	开天斩
			dwMagic = MEFF_KTSWORD;
			bUsed = LearnMagic(MEFF_KTSWORD, 0);
		}break;
	case 331:
		{
			//	先天气功
			dwMagic = MEFF_ENERGYSHIELD;
			bUsed = LearnMagic(MEFF_ENERGYSHIELD, 0);
		}break;
	case 332:
		{
			//	天霜冰环
			dwMagic = MEFF_ICETHRUST;
			bUsed = LearnMagic(MEFF_ICETHRUST, 0);
		}break;
	case 333:
		{
			//	血龙剑法
			dwMagic = MEFF_BLOODDRAGON;
			bUsed = LearnMagic(MEFF_BLOODDRAGON, 0);
		}break;
	case 334:
		{
			//	召唤白虎
			dwMagic = MEFF_SUMMONTIGER;
			bUsed = LearnMagic(MEFF_SUMMONTIGER, 0);
		}break;
	case 335:
		{
			//	强化刺杀剑法
			dwMagic = MEFF_CISHA;
			bUsed = LearnMagic(dwMagic, 1);
		}break;
	case 337:
		{
			//	强化雷电术
			dwMagic = MEFF_THUNDER;
			bUsed = LearnMagic(dwMagic, 1);
		}break;
	case 338:
		{
			//	强化施毒术
			dwMagic = MEFF_POISON;
			bUsed = LearnMagic(dwMagic, 1);
		}break;
	case 339:
		{
			//	群体治愈术
			dwMagic = MEFF_SUPERHEAL;
			bUsed = LearnMagic(dwMagic, 0);
		}break;
	case 340:
		{
			//	强化召唤白虎
			dwMagic = MEFF_SUMMONTIGER;
			bUsed = LearnMagic(dwMagic, 1);
		}break;
	case 341:
		{
			//	索命毒雾
			dwMagic = MEFF_BIGPOISON;
			bUsed = LearnMagic(dwMagic, 0);
		}break;
	case 342:
		{
			//	强化灵魂火符
			dwMagic = MEFF_FIRECHARM;
			bUsed = LearnMagic(dwMagic, 1);
		}break;
	case 343:
		{
			//	金刚不坏
			dwMagic = MEFF_JINGANG;
			bUsed = LearnMagic(dwMagic, 0);
		}break;
	case 345:
		{
			//	护体神盾
			dwMagic = MEFF_SHIELD;
			bUsed = LearnMagic(dwMagic, 1);
		}break;
	case 346:
		{
			//	召唤天王
			dwMagic = MEFF_SUMMONBOWMAN;
			bUsed = LearnMagic(dwMagic, 0);
		}break;
	case 347:
		{
			//	火龙气焰
			dwMagic = MEFF_DRAGONBLUSTER;
			bUsed = LearnMagic(dwMagic, 0);
		}break;
	case 348:
		{
			//	先天罡气
			dwMagic = MEFF_ENERGYSHIELD;
			bUsed = LearnMagic(dwMagic, 1);
		}break;
	}

	if(bUsed)
	{
		PkgPlayerUseItemAck ack;
		ack.dwUsage = 0;
		ack.dwTag = _pItem->tag;
		ack.uTargetId = GetID();

		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		SendPlayerBuffer(g_xThreadBuffer);

		PkgPlayerUpdateAttribNtf uantf;
		uantf.uTargetId = GetID();
		uantf.bType = UPDATE_MAGIC;
		uantf.dwParam = dwMagic;
		uantf.dwExtra = GetUserMagic(dwMagic)->bLevel;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << uantf;
		SendPlayerBuffer(g_xThreadBuffer);

		ZeroMemory(_pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(_pItem);
	}
	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UseScrollItem(ItemAttrib* _pItem)
{
	bool bUsed = false;
	int nGainExp = 0;

	if(GETITEMATB(_pItem, Type) == ITEM_COST &&
		GETITEMATB(_pItem, Curse) == 1)
	{
		if(GETITEMATB(_pItem, AtkSpeed) == 0)
		{
			PkgPlayerUpdateCostNtf ntf;
			ntf.uTargetId = GetID();
			ntf.dwTag = _pItem->tag;
			ntf.nNumber = 0;
			SendPacket(ntf);
			return true;
		}

		if(GETITEMATB(_pItem, Lucky) == 0)
		{
			if(!GetLocateScene()->CanUseScroll())
			{
				return false;
			}
			//	
			DWORD dwPos = 0;
			if(GetLocateScene()->GetRandomPosition(&dwPos))
			{
				/*WORD wPosX = LOWORD(dwPos);
				WORD wPosY = HIWORD(dwPos);
				m_stData.wCoordX = wPosX;
				m_stData.wCoordY = wPosY;*/

				if(FlyTo(LOWORD(dwPos), HIWORD(dwPos)))
				{
					/*PkgPlayerUseItemAck ack;
					ack.dwUsage = USE_RANDOM_POS;
					//ack.dwParam0 = dwPos;
					ack.dwTag = _pItem->tag;
					ack.uTargetId = GetID();

					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ack;
					//GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
					SendPlayerBuffer(g_xThreadBuffer);*/

					bUsed = true;
				}
			}
		}
		else if(GETITEMATB(_pItem, Lucky) == 1)
		{
			//
		}
		else if(GETITEMATB(_pItem, Lucky) == 2)
		{
			if(!GetLocateScene()->CanUseScroll())
			{
				//return false;
			}
			//	Go home
			GameScene* pScene = GetHomeScene();
			if(NULL != pScene)
			{
				WORD wPosX = pScene->GetCityCenterX();
				WORD wPosY = pScene->GetCityCenterY();

				if(pScene->GetMapID() != GetLocateScene()->GetMapID())
				{
					//
					bUsed = FlyToMap(wPosX, wPosY, pScene->GetMapID());
				}
				else
				{
					bUsed = true;
					//m_stData.wCoordX = wPosX;
					//m_stData.wCoordY = wPosY;
					//m_stData.eGameState = OS_STAND;

					//PkgPlayerForceActionAck ack;
					//ack.uTargetId = GetID();
					//ack.bType = FORCE_POSITION;
					//ack.dwData = MAKE_POSITION_DWORD(this);
					//SendPacket(ack);
					FlyTo(wPosX, wPosY);
				}

				if(bUsed)
				{
					/*PkgPlayerUseItemAck ack;
					ack.dwTag = _pItem->tag;
					ack.uTargetId = GetID();
					SendPacket(ack);*/
				}
			}
		}
		else if(GETITEMATB(_pItem, Lucky) == 3)
		{
			ShowShopDlg(NULL, SHOP_STORAGE);
			bUsed = true;

			if(bUsed)
			{
				/*PkgPlayerUseItemAck ack;
				ack.dwTag = _pItem->tag;
				ack.uTargetId = GetID();
				SendPacket(ack);*/
			}
		}
		else if(GETITEMATB(_pItem, Lucky) == 4)
		{
			nGainExp = 50000;
			/*if(GetObject_Level() >= 35)
			{
				nGainExp /= 2;
			}*/
			if(GetObject_Level() < 35)
			{
				nGainExp /= 50;
			}
			if(GameWorld::GetInstance().GetFinnalExprMulti() == 1)
			{
				nGainExp = 0;
			}

			GainExp(nGainExp);
			bUsed = true;

			if(bUsed)
			{
				/*PkgPlayerUseItemAck ack;
				ack.dwTag = _pItem->tag;
				ack.uTargetId = GetID();
				SendPacket(ack);*/
			}
		}
		else if(GETITEMATB(_pItem, Lucky) == 5)
		{
			nGainExp = 100000;
			/*if(GetObject_Level() >= 35)
			{
				nGainExp /= 2; 
			}*/
			if(GetObject_Level() < 35)
			{
				nGainExp /= 50;
			}
			if(GameWorld::GetInstance().GetFinnalExprMulti() == 1)
			{
				nGainExp = 0;
			}
			GainExp(nGainExp);
			bUsed = true;

			if(bUsed)
			{
				/*PkgPlayerUseItemAck ack;
				ack.dwTag = _pItem->tag;
				ack.uTargetId = GetID();
				SendPacket(ack);*/
			}
		}
		else if(GETITEMATB(_pItem, Lucky) == 6)
		{
			if(!GetLocateScene()->CanUseScroll())
			{
				return false;
			}

			GameScene* pScene = GameSceneManager::GetInstance()->GetScene(GETITEMATB(_pItem, Hide));
			if(NULL != pScene)
			{
				WORD wPosX = 0;
				WORD wPosY = 0;
				DWORD dwPos = 0;

				if(pScene->GetRandomPosition(&dwPos))
				{
					wPosX = LOWORD(dwPos);
					wPosY = HIWORD(dwPos);

					if(pScene->GetMapID() != GetLocateScene()->GetMapID())
					{
						//
						bUsed = FlyToMap(wPosX, wPosY, pScene->GetMapID());
					}
					else
					{
						bUsed = true;
						//m_stData.wCoordX = wPosX;
						//m_stData.wCoordY = wPosY;
						//m_stData.eGameState = OS_STAND;

						//PkgPlayerForceActionAck ack;
						//ack.uTargetId = GetID();
						//ack.bType = FORCE_POSITION;
						//ack.dwData = MAKE_POSITION_DWORD(this);
						//SendPacket(ack);
						FlyTo(wPosX, wPosY);
					}

					if(bUsed)
					{
						/*PkgPlayerUseItemAck ack;
						ack.dwTag = _pItem->tag;
						ack.uTargetId = GetID();
						SendPacket(ack);*/
					}
				}
			}
		}
		else if(GETITEMATB(_pItem, Lucky) == 7)
		{
			//	双倍爆率卷轴
			ActiveDoubleDrop(30*60*1000);
			SendSystemMessage("开启双倍爆率时间30分钟");
			SendStatusInfo();
			bUsed = true;
		}
	}

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(_pItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = _pItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(_pItem);
		}
		else
		{
			SETITEMATB(_pItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}
//////////////////////////////////////////////////////////////////////////
ItemAttrib* HeroObject::GetEmptyItem()
{
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
			m_xBag[i].tag != ITEMTAG_INQUERY)
		{
			return &m_xBag[i];
		}
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
GameScene* HeroObject::GetHomeScene()
{
	return GameSceneManager::GetInstance()->GetScene(m_dwLastCityMap);
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::GetMagicDamage(const UserMagic* _pMgc, GameObject* _pAttacked)
{
	if(_pMgc->bLevel != 0)
	{
		//int nCost = _pMgc->pInfo->bBaseCost + (_pMgc->pInfo->bBaseCost / 2 * (_pMgc->bLevel - 1));
		//if(m_stData.stAttrib.MP >= nCost)
		{
			if(_pAttacked != NULL)
			{
				
				/*
				float fDamage = 0;
												float fMultiple = 0;
												float fMultipleBase = 1 + (float)_pMgc->pInfo->bMultiple / 100;*/
				
				//int nDC = 0;
				int nAC = 0;
				
				if(m_stData.bJob == 0)
				{
					//nDC = GetRandomAbility(AT_DC);
					nAC = _pAttacked->GetRandomAbility(AT_AC);
				}
				else if(m_stData.bJob == 1)
				{
					//nDC = GetRandomAbility(AT_MC);
					nAC = _pAttacked->GetRandomAbility(AT_MAC);
				}
				else if(m_stData.bJob == 2)
				{
					//nDC = GetRandomAbility(AT_SC);
					nAC = _pAttacked->GetRandomAbility(AT_MAC);
				}

				//fMultiple = pow(fMultipleBase, _pMgc->bLevel);
				//fDamage = (float)nDC * fMultiple + _pMgc->bLevel * _pMgc->pInfo->wIncrease;
				//int nDamage = (int)fDamage;
				//nDamage -= nAC;
				int nDamage = GetMagicDamageNoDefence(_pMgc);

				if(_pMgc->pInfo->wID == MEFF_SKYFIRE)
				{
					nAC /= 2;
				}
				
				int nIgnoreAC = m_xStates.GetIgnoreACAddition();
				if(0 != nIgnoreAC)
				{
					float fPercent = 1.0f - (float)nIgnoreAC / 100;
					nAC *= fPercent;
				}
				nDamage -= nAC;

				if(nDamage < 0)
				{
					nDamage = 0;
				}
				return nDamage;
			}
		}
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::GetMagicDamageNoDefence(const UserMagic* _pMgc)
{
	if(_pMgc->bLevel != 0)
	{
		//int nCost = _pMgc->pInfo->bBaseCost + (_pMgc->pInfo->bBaseCost / 2 * (_pMgc->bLevel - 1));
		//if(m_stData.stAttrib.MP >= nCost)
		{
			//if(_pAttacked != NULL)
			{
				float fDamage = 0;
				float fMultiple = 0;
				int nDC = 0;
				int nAC = 0;
				if(m_stData.bJob == 0)
				{
					nDC = GetRandomAbility(AT_DC);
					//nAC = _pAttacked->GetRandomAbility(AT_AC);
				}
				else if(m_stData.bJob == 1)
				{
					nDC = GetRandomAbility(AT_MC);
					//nAC = _pAttacked->GetRandomAbility(AT_MAC);
				}
				else if(m_stData.bJob == 2)
				{
					nDC = GetRandomAbility(AT_SC);
					//nAC = _pAttacked->GetRandomAbility(AT_MAC);
				}

				float fMultipleBase = 1 + (float)_pMgc->pInfo->bMultiple / 100;
				if(_pMgc->bLevel <= 3)
				{
					fMultiple = pow(fMultipleBase, _pMgc->bLevel);
					fDamage = (float)nDC * fMultiple + _pMgc->bLevel * _pMgc->pInfo->wIncrease;
				}
				else if(_pMgc->bLevel <= 6)
				{
					fMultiple = pow(fMultipleBase, _pMgc->bLevel);
					//fMultiple *= pow((fMultipleBase * 1.05f), _pMgc->bLevel - 3);
					fDamage = (float)nDC * fMultiple + (_pMgc->bLevel - 3) * (_pMgc->pInfo->wIncrease * 1);
					/*GameWorld::GetInstance().Stop(10);
					FlyToPrison();
					return 0;*/
				}
				
				//fDamage = (float)nDC * fMultiple + _pMgc->bLevel * _pMgc->pInfo->wIncrease;
				int nDamage = (int)fDamage;
				//nDamage -= nAC;

				if(nDamage < 0)
				{
					nDamage = 0;
				}
				return nDamage;
			}
		}
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::GetMagicCost(const UserMagic* _pMgc)
{
#ifdef _DEBUG
	return 0;
	int nCost = _pMgc->pInfo->bBaseCost + (_pMgc->pInfo->bBaseCost / 2 * (_pMgc->bLevel - 1));
	return nCost;
#else
	int nCost = _pMgc->pInfo->bBaseCost + (_pMgc->pInfo->bBaseCost / 2 * (_pMgc->bLevel - 1));
	return nCost;
#endif
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::CanAllowLongHit()
{
	const UserMagic* pMgc = GetUserMagic(MEFF_CISHA);
	if(pMgc)
	{
		if(pMgc->bLevel > 0)
		{
			int nCost = GetMagicCost(pMgc);
			if(GetObject_MP() > nCost)
			{
				return true;
			}
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::CanAllowWideHit()
{
	const UserMagic* pMgc = GetUserMagic(MEFF_BANYUE);
	if(pMgc)
	{
		if(pMgc->bLevel > 0)
		{
			//int nCost = GetMagicCost(pMgc);
			//if(m_stData.stAttrib.MP > nCost)
			{
				return true;
			}
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::CanAllowPowerHit()
{
	const UserMagic* pMgc = GetUserMagic(MEFF_GONGSHA);
	if(pMgc)
	{
		if(pMgc->bLevel > 0)
		{
			//int nCost = GetMagicCost(pMgc);
			//if(m_stData.stAttrib.MP > nCost)
			{
				return true;
			}
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::CanAllowFireHit()
{
	const UserMagic* pMgc = GetUserMagic(MEFF_LIEHUO);
	if(pMgc)
	{
		if(pMgc->bLevel > 0)
		{
			//int nCost = GetMagicCost(pMgc);
			//if(m_stData.stAttrib.MP > nCost)
			{
				int nCost = GetMagicCost(pMgc);
				if(GetObject_MP() < nCost)
				{
					return false;
				}
				if(GetTickCount() - m_dwHumEffTime[MMASK_LIEHUO_INDEX] < 10000)
				{
#ifdef _DEBUG
					return true;
#else
					SendSystemMessage("召唤烈火精灵失败!");
					return false;
#endif
				}
				else
				{
					return true;
				}
			}
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::CanAllowSuperFireHit()
{
	const UserMagic* pMgc = GetUserMagic(MEFF_SLIEHUO);
	if(pMgc)
	{
		if(pMgc->bLevel > 0)
		{
			int nCost = GetMagicCost(pMgc);
			if(GetObject_MP() < nCost)
			{
				return false;
			}
			if(GetTickCount() - m_dwHumEffTime[MMASK_SLIEHUO_INDEX] < 12000)
			{
#ifdef _DEBUG
				return true;
#else
				SendSystemMessage("无法凝聚炙炎之力");
				return false;
#endif
			}
			else
			{
				return true;
			}
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::GetMagicMinDC(const UserMagic* _pMgc)
{
	if(_pMgc->bLevel != 0)
	{
		int nCost = _pMgc->pInfo->bBaseCost + (_pMgc->pInfo->bBaseCost / 2 * (_pMgc->bLevel - 1));
		if(GetObject_MP() >= nCost)
		{
			//if(_pAttacked != NULL)
			{
				float fDamage = 0;
				float fMultiple = 0;
				int nDC = 0;
				int nAC = 0;
				if(m_stData.bJob == 0)
				{
					//nDC = GetRandomAbility(AT_DC);
					nDC = GetObject_DC();
					//nAC = _pAttacked->GetRandomAbility(AT_AC);
				}
				else if(m_stData.bJob == 1)
				{
					//nDC = GetRandomAbility(AT_MC);
					nDC = GetObject_MC();
					//nAC = _pAttacked->GetRandomAbility(AT_MAC);
				}
				else if(m_stData.bJob == 2)
				{
					//nDC = GetRandomAbility(AT_SC);
					nDC = GetObject_SC();
					//nAC = _pAttacked->GetRandomAbility(AT_MAC);
				}

				float fMultipleBase = 1 + (float)_pMgc->pInfo->bMultiple / 100;
				if(_pMgc->bLevel <= 3)
				{
					fMultiple = pow(fMultipleBase, _pMgc->bLevel);
					fDamage = (float)nDC * fMultiple + _pMgc->bLevel * _pMgc->pInfo->wIncrease;
				}
				else
				{
					fMultiple = pow(fMultipleBase, 3);
					fMultiple *= pow((fMultipleBase * 1.05f), _pMgc->bLevel - 3);
					fDamage = (float)nDC * fMultiple + (_pMgc->bLevel * 3) * (_pMgc->pInfo->wIncrease * 2);
				}

				//fDamage = (float)nDC * fMultiple + _pMgc->bLevel * _pMgc->pInfo->wIncrease;
				int nDamage = (int)fDamage;
				//nDamage -= nAC;

				if(nDamage < 0)
				{
					nDamage = 0;
				}
				return nDamage;
			}
		}
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::GetMagicMaxDC(const UserMagic* _pMgc)
{
	if(_pMgc->bLevel != 0)
	{
		int nCost = _pMgc->pInfo->bBaseCost + (_pMgc->pInfo->bBaseCost / 2 * (_pMgc->bLevel - 1));
		if(GetObject_MP() >= nCost)
		{
			//if(_pAttacked != NULL)
			{
				float fDamage = 0;
				float fMultiple = 0;
				int nDC = 0;
				int nAC = 0;
				if(m_stData.bJob == 0)
				{
					//nDC = GetRandomAbility(AT_DC);
					nDC = GetObject_MaxDC();
					//nAC = _pAttacked->GetRandomAbility(AT_AC);
				}
				else if(m_stData.bJob == 1)
				{
					//nDC = GetRandomAbility(AT_MC);
					nDC = GetObject_MaxMC();
					//nAC = _pAttacked->GetRandomAbility(AT_MAC);
				}
				else if(m_stData.bJob == 2)
				{
					//nDC = GetRandomAbility(AT_SC);
					nDC = GetObject_MaxSC();
					//nAC = _pAttacked->GetRandomAbility(AT_MAC);
				}

				float fMultipleBase = 1 + (float)_pMgc->pInfo->bMultiple / 100;
				if(_pMgc->bLevel <= 3)
				{
					fMultiple = pow(fMultipleBase, _pMgc->bLevel);
					fDamage = (float)nDC * fMultiple + _pMgc->bLevel * _pMgc->pInfo->wIncrease;
				}
				else
				{
					fMultiple = pow(fMultipleBase, 3);
					fMultiple *= pow((fMultipleBase * 1.05f), _pMgc->bLevel - 3);
					fDamage = (float)nDC * fMultiple + (_pMgc->bLevel * 3) * (_pMgc->pInfo->wIncrease * 2);
				}

				//fDamage = (float)nDC * fMultiple + _pMgc->bLevel * _pMgc->pInfo->wIncrease;
				int nDamage = (int)fDamage;
				//nDamage -= nAC;

				if(nDamage < 0)
				{
					nDamage = 0;
				}
				return nDamage;
			}
		}
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::DoSpell(const PkgUserActionReq& req)
{
	if(req.uAction != ACTION_SPELL)
	{
		return false;
	}

	DWORD dwCurTick = GetTickCount();
	DWORD dwInterval = dwCurTick - m_dwLastSpellTime;
	if(dwInterval < GetHeroSpellInterval() - 200)
	{
		++m_dwTimeOut;
		return false;
	}

	bool bCanCross = true;
	DWORD dwTargetPos = 0;

	bool bBroadcast = false;
	const UserMagic* pMagic = NULL;
	//GameObject* pObj = GetLocateScene()->GetNPCByHandleID(req.uTargetId);
	GameObject* pObj = NULL;
	int nTargetX = -1;
	int nTargetY = -1;
	int nSelfX = m_stData.wCoordX;
	int nSelfY = m_stData.wCoordY;
	if(req.uTargetId != 0)
	{
		pObj = GetLocateScene()->GetNPCByHandleID(req.uTargetId);
		if(NULL == pObj)
		{
			pObj = GetLocateScene()->GetPlayerWithoutLock(req.uTargetId);
		}
		if(pObj)
		{
			nTargetX = pObj->GetUserData()->wCoordX;
			nTargetY = pObj->GetUserData()->wCoordY;
		}
	}
	else
	{
		nTargetX = LOWORD(req.uParam2) / 48;
		nTargetY = HIWORD(req.uParam2) / 32;
	}

	PkgObjectActionNot not;
	not.uAction = ACTION_SPELL;
	not.uParam0 = req.uParam0;
	not.uParam1 = req.uParam1;

	static int s_nAround[] =
	{
		0,-1,
		1,-1,
		1,0,
		1,1,
		0,1,
		-1,1,
		-1,0,
		-1,-1
	};

	//////////////////////////////////////////////////////////////////////////
	//	Just for anti-cheating
	int nPreMP = GetUserData()->stAttrib.MP;
	//////////////////////////////////////////////////////////////////////////

	DWORD dwMgcID = LOWORD(req.uParam1);
	SetLastUseMagicID(dwMgcID);

	PkgPlayerEnableSkillNot ppesn;
	ppesn.uTargetId = GetID();
	ppesn.nSkillId = LOWORD(req.uParam1);

	switch(LOWORD(req.uParam1))
	{
	case MEFF_GONGSHA:
		{
			//	nothing
		}break;
	case MEFF_CISHA:
		{
			//	
			//m_dwHumEffFlag |= MEFF_CISHA;
			if(m_dwHumEffFlag & MMASK_CISHA)
			{
				RESET_FLAG(m_dwHumEffFlag, MMASK_CISHA);
				SendChatMessage("刺杀剑法关闭", 1);
				ppesn.bEnabled = false;
			}
			else
			{
				SET_FLAG(m_dwHumEffFlag, MMASK_CISHA);
				SendChatMessage("刺杀剑法开启", 1);
				ppesn.bEnabled = true;
			}
			SendPacket(ppesn);
		}break;
	case MEFF_BANYUE:
		{
			//
			if(CanAllowWideHit())
			{
				//m_dwHumEffFlag |= MEFF_BANYUE;
				if(m_dwHumEffFlag & MMASK_BANYEU)
				{
					RESET_FLAG(m_dwHumEffFlag, MMASK_BANYEU);
					SendChatMessage("半月剑法关闭", 1);
					ppesn.bEnabled = false;
				}
				else
				{
					SET_FLAG(m_dwHumEffFlag, MMASK_BANYEU);
					SendChatMessage("半月剑法开启", 1);
					ppesn.bEnabled = true;
				}
				SendPacket(ppesn);
			}break;
		}
	case MEFF_LIEHUO:
		{
			//
			if(CanAllowFireHit())
			{
				pMagic = GetUserMagic(MEFF_LIEHUO);
				if(pMagic)
				{
					int nCost = GetMagicCost(pMagic);
					DecMP(nCost);
					m_pValid->DecMP(nCost);
					nPreMP -= nCost;
					SyncMP(this);
					m_dwHumEffFlag |= MMASK_LIEHUO;
					m_dwHumEffTime[MMASK_LIEHUO_INDEX] = GetTickCount();
					SendChatMessage("召唤烈火精灵成功", 1);
					ppesn.bEnabled = true;
					SendPacket(ppesn);
				}
			}
		}break;
	case MEFF_SLIEHUO:
		{
			if(CanAllowSuperFireHit())
			{
				pMagic = GetUserMagic(MEFF_SLIEHUO);
				if(pMagic)
				{
					int nCost = GetMagicCost(pMagic);
					DecMP(nCost);
					m_pValid->DecMP(nCost);
					nPreMP -= nCost;
					SyncMP(this);
					m_dwHumEffFlag |= MMASK_SLIEHUO;
					m_dwHumEffTime[MMASK_SLIEHUO_INDEX] = GetTickCount();
					SendSystemMessage("凝聚炙炎之力成功");
					ppesn.bEnabled = true;
					SendPacket(ppesn);
				}
			}
		}break;
	case MEFF_BLOODDRAGON://	血龙剑法
		{
			pMagic = GetUserMagic(dwMgcID);
			if(pMagic)
			{
				if(pMagic->bLevel != 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);
						bBroadcast = true;

						int nTime = 5000 * pMagic->bLevel + GetRandomAbility(AT_DC) * 200;
						not.uParam3 = nTime;

						m_xStates.PushItem(EAID_CRITICAL, nTime, pMagic->bLevel);
					}
				}
			}
		}break;
	case MEFF_JINGANG:
		{
			pMagic = GetUserMagic(dwMgcID);
			if(pMagic)
			{
				if(pMagic->bLevel != 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);
						bBroadcast = true;

						int nTime = 5000 * pMagic->bLevel + GetRandomAbility(AT_DC) * 400;
						not.uParam3 = nTime;

						m_dwJinGangExpireTime = GetTickCount() + nTime;

						//SendStatusInfo();
						BroadcastStatusInfo();
					}
				}
			}
		}break;
	case MEFF_SFIREBALL:		//"小火球",
	case MEFF_LFIREBALL:			//"大火球",
	case MEFF_THUNDER:			//"雷电术",
	case MEFF_FIRECHARM:		//"灵魂火符"
	case MEFF_ICEPALM:			//"寒冰掌"
	case MEFF_SKYFIRE:
		{
			if (!IsMagicAttackValid(dwMgcID, nTargetX, nTargetY))
			{
				if (m_nInvalidMagicAttackTimes > INVALID_MAGIC_KICK_TIMES)
				{
					ForceDisconnectHero();
				}
				return false;
			}

			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(pMagic)
			{
				if(pMagic->bLevel != 0)
				{
					//	加入技能等级信息
					not.uParam3 = MAKELONG(pMagic->bLevel, 0);
					//int nCost = pMagic->pInfo->bBaseCost + (pMagic->pInfo->bBaseCost / 2 * (pMagic->bLevel - 1));
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);
						bBroadcast = true;

						if(pMagic->pInfo->wID != MEFF_THUNDER &&
							pMagic->pInfo->wID != MEFF_SKYFIRE)
						{
							if(pObj != NULL)
							{
								bCanCross = GetLocateScene()->CanCross(GetUserData()->wCoordX,
									GetUserData()->wCoordY,
									pObj->GetUserData()->wCoordX,
									pObj->GetUserData()->wCoordY);

								if(!bCanCross)
								{
									dwTargetPos = MAKELONG(pObj->GetUserData()->wCoordX * UNIT_WIDTH, pObj->GetUserData()->wCoordY * UNIT_HEIGHT);
									pObj = NULL;
								}
							}
						}
						if(pObj != NULL)
						{
							int nDamage = GetMagicDamageNoDefence(pMagic);
							if(nDamage > 0)
							{
								if(pObj->GetType() == SOT_MONSTER)
								{
									MonsterObject* pMonster = (MonsterObject*)pObj;

									if((pMagic->pInfo->wID == MEFF_SFIREBALL ||
										pMagic->pInfo->wID == MEFF_LFIREBALL ||
										pMagic->pInfo->wID == MEFF_SKYFIRE) &&
										pObj->IsFrozen())
									{
										nDamage *= 1.3f;
									}
								}

								if(pMagic->pInfo->wID == MEFF_SKYFIRE)
								{
									int nSkyFireAddition = m_xStates.GetAddSkyFireAddition();
									if(nSkyFireAddition != 0)
									{
										float fMulti = 1 + (float)nSkyFireAddition / 10.0f;
										nDamage *= fMulti;
									}
								}

								if(0 != pObj->ReceiveDamage(this, true, nDamage, 1600))
								{
									SetSlaveTarget(pObj);
								}
							}
						}
					}
				}
			}
		}break;
	case MEFF_BURSTFIRE:			//"爆裂火焰",
	case MEFF_ICEROAR:			//"冰咆哮",
	case MEFF_ICETHRUST:
	//case MEFF_HELLTHUNDER:		//"地狱雷光",
		{
			if(LOWORD(req.uParam1) == MEFF_HELLTHUNDER)
			{
				nTargetX = m_stData.wCoordX;
				nTargetY = m_stData.wCoordY;
			}


			if (!IsMagicAttackValid(dwMgcID, nTargetX, nTargetY))
			{
				if (m_nInvalidMagicAttackTimes > INVALID_MAGIC_KICK_TIMES)
				{
					ForceDisconnectHero();
				}
				return false;
			}

			pMagic = GetUserMagic(LOWORD(req.uParam1));
			int nDamage = 0;
			if(pMagic)
			{
				if(pMagic->bLevel != 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);
						bBroadcast = true;

						float fMulti = 1.0f;
						if(LOWORD(req.uParam1) == MEFF_ICEROAR)
						{
							int nAddIceRoar = m_xStates.GetAddIceRoarAddition();
							if(nAddIceRoar > 0)
							{
								fMulti = 1 + (float)nAddIceRoar / 10;
							}
						}

						if(nTargetX != -1 &&
							nTargetY != -1)
						{
							GameObjectList xObjs;
							GetLocateScene()->GetMappedObjects(nTargetX, nTargetY, 1, 1, xObjs, MAPPEDOBJECT_ALL_ALIVE);
							GameObjectList::iterator it = xObjs.begin();
							for(it;
								it != xObjs.end();
								++it)
							{
								GameObject* pAttacked = *it;
								nDamage = GetMagicDamageNoDefence(pMagic);
								nDamage *= fMulti;
								if(nDamage > 0)
								{
									pAttacked->ReceiveDamage(this, true, nDamage, 1800);
								}
							}
						}
					}
				}
			}
		}break;
		case MEFF_FIRESHOWER:			//"火流星",
		//case MEFF_ICESTORM:
			{

				if (!IsMagicAttackValid(dwMgcID, nTargetX, nTargetY))
				{
					if (m_nInvalidMagicAttackTimes > INVALID_MAGIC_KICK_TIMES)
					{
						ForceDisconnectHero();
					}
					return false;
				}

				pMagic = GetUserMagic(LOWORD(req.uParam1));
				int nDamage = 0;
				if(pMagic)
				{
					if(pMagic->bLevel != 0)
					{
						int nCost = GetMagicCost(pMagic);
						if(GetObject_MP() >= nCost)
						{
							nPreMP -= nCost;
							DecMP(nCost);
							m_pValid->DecMP(nCost);
							SyncMP(this);
							bBroadcast = true;

							if(nTargetX != -1 &&
								nTargetY != -1)
							{
								GameObjectList xObjs;
								GetLocateScene()->GetMappedObjects(nTargetX, nTargetY, 1, 1, xObjs, MAPPEDOBJECT_ALL_ALIVE);
								GameObjectList::iterator it = xObjs.begin();
								for(it;
									it != xObjs.end();
									++it)
								{
									GameObject* pAttacked = *it;

									for(int i = 0; i < 3; ++i)
									{
										//nDamage = GetMagicDamage(pMagic, pTarget);
										nDamage = GetMagicDamageNoDefence(pMagic);

										if(dwMgcID == MEFF_FIRESHOWER)
										{
											if(pAttacked->IsFrozen())
											{
												nDamage *= 1.5f;
											}
										}

										if(nDamage > 0)
										{
											pAttacked->ReceiveDamage(this, true, nDamage, 2500 + i * 700);
										}
									}
								}
							}
						}
					}
				}
			}break;
	case MEFF_WORDSPELL:			//"圣言术",
		{

		}break;
	//case MEFF_FIRESHOWER:
	case MEFF_FIREWALL:			//"火墙",
		{
			if(GetLocateScene()->GetMapData(nTargetX, nTargetY) != NULL)
			{
				if(LOWORD(req.uParam1) == MEFF_FIREWALL)
				{
					//nTargetX = m_stData.wCoordX;
					//nTargetY = m_stData.wCoordY;
				}


				if (!IsMagicAttackValid(dwMgcID, nTargetX, nTargetY))
				{
					if (m_nInvalidMagicAttackTimes > INVALID_MAGIC_KICK_TIMES)
					{
						ForceDisconnectHero();
					}
					return false;
				}

				pMagic = GetUserMagic(LOWORD(req.uParam1));
				int nDamage = 0;
				if(pMagic)
				{
					if(pMagic->bLevel > 0)
					{
						int nCost = GetMagicCost(pMagic);
						if(GetObject_MP() >= nCost)
						{
							nPreMP -= nCost;
							DecMP(nCost);
							m_pValid->DecMP(nCost);
							SyncMP(this);

							StaticMagic sm = {0};
							sm.dwEnableTime = 0;
							sm.dwEffectActive = 2;
							sm.dwFire = GetID();
							sm.sPosX = nTargetX;
							sm.sPoxY = nTargetY;
							sm.wMaxDC = GetMagicMaxDC(pMagic);
							sm.wMinDC = GetMagicMinDC(pMagic);
							not.uParam3 = 2500 + GetRandomAbility(AT_MC) * 600;
							sm.dwExpire = GetTickCount() + not.uParam3;
							sm.wMgcID = LOWORD(req.uParam1);
							sm.pFire = this;
							GetLocateScene()->PushStaticMagic(&sm);
							bBroadcast = true;
						}
					}
				}
			}
		}break;
	case MEFF_DRAGONBLUSTER:
		{
			//	火龙气焰

			if (!IsMagicAttackValid(dwMgcID, nTargetX, nTargetY))
			{
				if (m_nInvalidMagicAttackTimes > INVALID_MAGIC_KICK_TIMES)
				{
					ForceDisconnectHero();
				}
				return false;
			}

			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(pMagic)
			{
				if(pMagic->bLevel != 0)
				{
					//int nCost = pMagic->pInfo->bBaseCost + (pMagic->pInfo->bBaseCost / 2 * (pMagic->bLevel - 1));
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);
						bBroadcast = true;

						/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
						pMsg->uOp = DELAY_DRAGONBLUSTER;
						//pMsg->uParam[0] = MAKELONG(GetCoordX(), GetCoordY());
						pMsg->uParam[0] = MAKELONG(nTargetX, nTargetY);

						bool bCanCritical = CanCriticalAttack();
						if(bCanCritical)
						{
							pMsg->uParam[1] = MAKELONG(GetMagicMaxDC(pMagic) * GetCriticalAttackLimit(), GetMagicMaxDC(pMagic) / 2);
						}
						else
						{
							pMsg->uParam[1] = MAKELONG(GetMagicMaxDC(pMagic), GetMagicMaxDC(pMagic) / 2);
						}
						pMsg->uParam[2] = 3000;
						pMsg->uParam[3] = GetID();
						pMsg->uParam[4] = pMagic->bLevel;
						if(bCanCritical)
						{
							SET_FLAG(pMsg->uParam[5], ATTACKMSG_MASK_CRITICAL);
						}
						pMsg->dwDelayTime = GetTickCount() + 300;
						GetLocateScene()->PushDelayBuf(pMsg);*/
						DelayActionStaticMagic* pAction = new DelayActionStaticMagic;
						pAction->nPosX = nTargetX;
						pAction->nPosY = nTargetY;
						pAction->wMaxDC = GetMagicMaxDC(pMagic);
						pAction->wMinDC = GetMagicMaxDC(pMagic) / 2;
						pAction->nContinueTime = 3000;
						pAction->nMagicId = MEFF_DRAGONBLUSTER;
						pAction->nMagicLevel = pMagic->bLevel;
						pAction->nMapId = GetMapID();
						pAction->dwActiveTime = GetTickCount() + 300;
						AddDelayAction(pAction);

						not.uParam3 = 3000 + GetRandomAbility(AT_SC) * 100;
					}
				}
			}
		}break;
	case MEFF_BIGPOISON:
		{

			if (!IsMagicAttackValid(dwMgcID, nTargetX, nTargetY))
			{
				if (m_nInvalidMagicAttackTimes > INVALID_MAGIC_KICK_TIMES)
				{
					ForceDisconnectHero();
				}
				return false;
			}

			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(pMagic)
			{
				if(pMagic->bLevel != 0)
				{
					//int nCost = pMagic->pInfo->bBaseCost + (pMagic->pInfo->bBaseCost / 2 * (pMagic->bLevel - 1));
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);
						bBroadcast = true;

						if(pObj != NULL)
						{
							bCanCross = GetLocateScene()->CanCross(GetUserData()->wCoordX,
								GetUserData()->wCoordY,
								pObj->GetUserData()->wCoordX,
								pObj->GetUserData()->wCoordY);

							if(!bCanCross)
							{
								dwTargetPos = MAKELONG(pObj->GetUserData()->wCoordX * UNIT_WIDTH, pObj->GetUserData()->wCoordY * UNIT_HEIGHT);
								pObj = NULL;
							}
						}

						if(pObj != NULL)
						{
							/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
							pMsg->uOp = DELAY_BIGPOISONSKL;
							pMsg->uParam[0] = MAKELONG(nTargetX, nTargetY);
							pMsg->uParam[1] = MAKELONG(GetObject_MaxSC(), GetObject_SC());
							pMsg->uParam[2] = 3000 + GetRandomAbility(AT_SC) * 100;
							pMsg->uParam[3] = GetID();
							pMsg->uParam[4] = pMagic->bLevel;
							pMsg->dwDelayTime = GetTickCount() + 800;
							GetLocateScene()->PushDelayBuf(pMsg);*/
							DelayActionStaticMagic* pAction = new DelayActionStaticMagic;
							pAction->nPosX = nTargetX;
							pAction->nPosY = nTargetY;
							pAction->wMaxDC = GetObject_MaxSC();
							pAction->wMinDC = GetObject_SC();
							pAction->nContinueTime = 3000 + GetRandomAbility(AT_SC) * 100;
							pAction->nMagicId = MEFF_BIGPOISON;
							pAction->nMagicLevel = pMagic->bLevel;
							pAction->nMapId = GetMapID();
							pAction->dwActiveTime = GetTickCount() + 800;
							AddDelayAction(pAction);

							not.uParam3 = 3000 + GetRandomAbility(AT_SC) * 100;
						}
					}
				}
			}
		}break;
	case MEFF_SHIELD:			//"魔法盾",
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						//if(!TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_SHIELD))
						{
							//	魔法盾解除状态 才能有效
							int nTime = GetRandomAbility(AT_MC) * 2000 + 5000;
							not.uParam3 = nTime;
							SET_FLAG(m_dwHumEffFlag, MMASK_SHIELD);
							m_dwHumEffTime[MMASK_SHIELD_INDEX] = GetTickCount() + nTime;
							m_dwDefence = 0;
							//SendStatusInfo();
						}
						
						bBroadcast = true;
					}
				}
			}
		}break;
	case MEFF_ENERGYSHIELD:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						int nTime = GetRandomAbility(AT_SC) * 500 + 5000 * pMagic->bLevel;
						if(pMagic->bLevel > 3)
						{
							nTime = GetRandomAbility(AT_SC) * 500 + 5000 * 3;
						}
						not.uParam3 = nTime;
						SET_FLAG(m_dwHumEffFlag, MMASK_ENERGYSHIELD);
						m_dwHumEffTime[MMASK_ENERGYSHIELD_INDEX] = GetTickCount() + nTime;
						m_dwDefence = 0;
						bBroadcast = true;
						SendStatusInfo();
					}
				}
			}
		}break;
	case MEFF_HELLTHUNDER:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						nTargetX = m_stData.wCoordX;
						nTargetY = m_stData.wCoordY;

						int nAtkX = 0;
						int nAtkY = 0;
						CellData* pData = NULL;
						MapCellInfo* pCell = NULL;
						int nDamage = 0;

						GameObjectList xObjs;
						GetLocateScene()->GetMappedObjects(nTargetX, nTargetY, 2, 2, xObjs, MAPPEDOBJECT_ALL_ALIVE);
						GameObjectList::iterator it = xObjs.begin();
						for(it;
							it != xObjs.end();
							++it)
						{
							GameObject* pAttacked = *it;
							nDamage = GetMagicDamageNoDefence(pMagic);

							if(pAttacked->GetType() == SOT_MONSTER)
							{
								MonsterObject* pMons = (MonsterObject*)pAttacked;
								if(pMons->GetObject_Curse() == 1)
								{
									nDamage *= 2;
								}
							}

							if(abs(pAttacked->GetCoordX() - GetCoordX()) == 2 ||
								abs(pAttacked->GetCoordY() - GetCoordY()) == 2)
							{
								nDamage /= 2;
							}

							if(nDamage > 0)
							{
								pAttacked->ReceiveDamage(this, true, nDamage, 800);
							}
						}

						bBroadcast = true;
					}
				}
			}
		}break;
		case MEFF_SUPERHEAL:
		{

			if (!IsMagicAttackValid(dwMgcID, nTargetX, nTargetY))
			{
				if (m_nInvalidMagicAttackTimes > INVALID_MAGIC_KICK_TIMES)
				{
					ForceDisconnectHero();
				}
				return false;
			}

			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						//nTargetX = m_stData.wCoordX;
						//nTargetY = m_stData.wCoordY;

						int nAtkX = 0;
						int nAtkY = 0;
						CellData* pData = NULL;
						MapCellInfo* pCell = NULL;
						int nDamage = 0;

						for(int i = -1; i <= 1; ++i)
						{
							for(int j = -1; j <= 1; ++j)
							{
								nAtkX = nTargetX + i;
								nAtkY = nTargetY + j;
								pCell = GetLocateScene()->GetMapData(nAtkX, nAtkY);
								if(pCell)
								{
									if(pCell->pCellObjects)
									{
										CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
										CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();
										CellData* pData = NULL;
										for(begiter; begiter != enditer; ++begiter)
										{
											pData = *begiter;
											if(pData->bType == CELL_MOVEOBJECT)
											{
												GameObject* pTarget = (GameObject*)pData->pData;

												if(pTarget->GetType() == SOT_MONSTER ||
													pTarget->GetType() == SOT_HERO)
												{
													//nDamage = GetMagicDamageNoDefence(pMagic);
													nDamage = GetMagicDamageNoDefence(pMagic);

													bool bCanAttack = false;
													int nTargetType = 0;

													if(pTarget->GetType() == SOT_MONSTER)
													{
														//	宠物可以加血
														if(static_cast<MonsterObject*>(pTarget)->GetMaster() != NULL)
														{
															bCanAttack = true;
															nTargetType = 1;
														}
													}
													if(pTarget->GetType() == SOT_HERO)
													{
														bCanAttack = true;
														nTargetType = 0;
													}

													if(nDamage > 0 &&
														bCanAttack)
													{
														if(nDamage > 0)
														{
															/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
															pMsg->uOp = DELAY_HEAL;
															pMsg->uParam[0] = pTarget->GetID();
															pMsg->uParam[1] = nDamage;
															pMsg->uParam[2] = nTargetType;
															pMsg->uParam[3] = 0;
															pMsg->dwDelayTime = GetTickCount() + 500;
															GetLocateScene()->PushDelayBuf(pMsg);*/
															DelayActionHeal* pHealAction = new DelayActionHeal;
															pHealAction->nHealCnt = nDamage;
															pHealAction->dwActiveTime = GetTickCount() + 500;
															pTarget->AddDelayAction(pHealAction);
														}
													}
												}
											}
										}
									}
								}
							}
						}

						bBroadcast = true;
					}
				}
			}
		}break;
	case MEFF_HEAL:
		{
			//	治愈术
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			int nHeal = 0;
			if(pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					if(pObj == NULL)
					{
						//	heal hero himself
						int nCost = GetMagicCost(pMagic);
						if(GetObject_MP() >= nCost)
						{
							nPreMP -= nCost;
							DecMP(nCost);
							m_pValid->DecMP(nCost);
							SyncMP(this);
							nHeal = GetMagicDamageNoDefence(pMagic);

							/*DelaySendInfo info;
							info.uOp = DELAY_HEAL;
							info.uParam[0] = GetID();
							info.uParam[1] = nHeal;
							info.uParam[2] = 0;
							info.uDelayTime = GetTickCount() + 500;
							GetLocateScene()->PushDelayBuf(info);*/
							/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
							pMsg->uOp = DELAY_HEAL;
							pMsg->uParam[0] = GetID();
							pMsg->uParam[1] = nHeal;
							pMsg->uParam[2] = 0;
							pMsg->dwDelayTime = GetTickCount() + 500;
							GetLocateScene()->PushDelayBuf(pMsg);*/
							DelayActionHeal* pHealAction = new DelayActionHeal;
							pHealAction->nHealCnt = nHeal;
							pHealAction->dwActiveTime = GetTickCount() + 500;
							AddDelayAction(pHealAction);

							bBroadcast = true;
						}
					}
					else
					{
						if(pObj->GetType() == SOT_HERO)
						{
							int nCost = GetMagicCost(pMagic);
							if(GetObject_MP() >= nCost)
							{
								nPreMP -= nCost;
								DecMP(nCost);
								m_pValid->DecMP(nCost);
								SyncMP(this);
								nHeal = GetMagicDamageNoDefence(pMagic);
								
								/*DelaySendInfo info;
								info.uOp = DELAY_HEAL;
								info.uParam[0] = pObj->GetID();
								info.uParam[1] = nHeal;
								info.uParam[2] = 0;
								info.uDelayTime = GetTickCount() + 500;
								GetLocateScene()->PushDelayBuf(info);*/
								/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
								pMsg->uOp = DELAY_HEAL;
								pMsg->uParam[0] = pObj->GetID();
								pMsg->uParam[1] = nHeal;
								pMsg->uParam[2] = 0;
								pMsg->dwDelayTime = GetTickCount() + 500;
								GetLocateScene()->PushDelayBuf(pMsg);*/
								DelayActionHeal* pHealAction = new DelayActionHeal;
								pHealAction->nHealCnt = nHeal;
								pHealAction->dwActiveTime = GetTickCount() + 500;
								pObj->AddDelayAction(pHealAction);

								bBroadcast = true;
							}
						}
						else if(pObj->GetType() == SOT_MONSTER)
						{
							int nCost = GetMagicCost(pMagic);
							if(GetObject_MP() >= nCost)
							{
								nPreMP -= nCost;
								DecMP(nCost);
								m_pValid->DecMP(nCost);
								SyncMP(this);
								nHeal = GetMagicDamageNoDefence(pMagic);

								if(static_cast<MonsterObject*>(pObj)->GetMaster() != NULL)
								{
									/*DelaySendInfo info;
									info.uOp = DELAY_HEAL;
									info.uParam[0] = pObj->GetID();
									info.uParam[1] = nHeal;
									info.uParam[2] = 1;
									info.uDelayTime = GetTickCount() + 500;
									GetLocateScene()->PushDelayBuf(info);*/
									/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
									pMsg->uOp = DELAY_HEAL;
									pMsg->uParam[0] = pObj->GetID();
									pMsg->uParam[1] = nHeal;
									pMsg->uParam[2] = 1;
									pMsg->dwDelayTime = GetTickCount() + 500;
									GetLocateScene()->PushDelayBuf(pMsg);*/
									DelayActionHeal* pHealAction = new DelayActionHeal;
									pHealAction->nHealCnt = nHeal;
									pHealAction->dwActiveTime = GetTickCount() + 500;
									pObj->AddDelayAction(pHealAction);
								}
								bBroadcast = true;
							}
						}
					}
				}
			}
		}break;
	case MEFF_SUMMON:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
						pMsg->uOp = DELAY_MAKESLAVE;
						pMsg->uParam[0] = GetID();
						if(pMagic->bLevel < 4)
						{
							pMsg->uParam[1] = 14;
						}
						pMsg->dwDelayTime = GetTickCount() + 500;
						GetLocateScene()->PushDelayBuf(pMsg);*/
						DelayActionMakeSlave* pMakeSlaveAction = new DelayActionMakeSlave;
						pMakeSlaveAction->dwActiveTime = GetTickCount() + 500;
						pMakeSlaveAction->nSlaveId = 14;
						AddDelayAction(pMakeSlaveAction);

						bBroadcast = true;
					}
				}
			}
		}break;
	case MEFF_SUPERSUMMON:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
						pMsg->uOp = DELAY_MAKESLAVE;
						pMsg->uParam[0] = GetID();
						if(pMagic->bLevel < 4)
						{
							pMsg->uParam[1] = 62;
						}
						pMsg->dwDelayTime = GetTickCount() + 500;
						GetLocateScene()->PushDelayBuf(pMsg);*/

						DelayActionMakeSlave* pMakeSlaveAction = new DelayActionMakeSlave;
						pMakeSlaveAction->dwActiveTime = GetTickCount() + 500;
						pMakeSlaveAction->nSlaveId = 62;
						AddDelayAction(pMakeSlaveAction);

						bBroadcast = true;
					}
				}
			}
		}break;
	case MEFF_SUMMONTIGER:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
						pMsg->uOp = DELAY_MAKESLAVE;
						pMsg->uParam[0] = GetID();
						if(pMagic->bLevel < 7)
						{
							pMsg->uParam[1] = 99;
						}
						pMsg->dwDelayTime = GetTickCount() + 500;
						GetLocateScene()->PushDelayBuf(pMsg);*/
						DelayActionMakeSlave* pMakeSlaveAction = new DelayActionMakeSlave;
						pMakeSlaveAction->dwActiveTime = GetTickCount() + 500;
						pMakeSlaveAction->nSlaveId = 99;
						AddDelayAction(pMakeSlaveAction);

						bBroadcast = true;
					}
				}
			}
		}break;
	case MEFF_SUMMONBOWMAN:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
						pMsg->uOp = DELAY_MAKESLAVE;
						pMsg->uParam[0] = GetID();
						pMsg->uParam[1] = 144;
						pMsg->dwDelayTime = GetTickCount() + 500;
						GetLocateScene()->PushDelayBuf(pMsg);*/
						DelayActionMakeSlave* pMakeSlaveAction = new DelayActionMakeSlave;
						pMakeSlaveAction->dwActiveTime = GetTickCount() + 500;
						pMakeSlaveAction->nSlaveId = 144;
						AddDelayAction(pMakeSlaveAction);

						bBroadcast = true;
					}
				}
			}
		}break;
	case MEFF_SUMMONAC:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						for(int i = 0; i < MAX_SLAVE_SUM; ++i)
						{
							if(GetSlave(i) != NULL)
							{
								MonsterObject* pMonster = static_cast<MonsterObject*>(GetSlave(i));
								if(pMonster->GetMaster() != NULL)
								{
#ifdef _DEBUG
									if(pMonster->GetObject_ID() == 62)
#else
									if(pMonster->GetObject_ID() == 62)
#endif
									{
										int nSlaveAC = pMonster->GetObject_AC();
										int nSlaveMAC = pMonster->GetObject_MAC();

										nSlaveAC += pMagic->bLevel * 6;
										nSlaveMAC += pMagic->bLevel * 3;

										pMonster->SetObject_AC(nSlaveAC);
										pMonster->SetObject_MAC(nSlaveMAC);
										pMonster->SetObject_ID(82);
										pMonster->SetObject_Hide(pMonster->GetObject_Hide() + pMagic->bLevel);
										pMonster->SetObject_MaxDC(pMonster->GetObject_MaxDC() + 7 * pMagic->bLevel);

										PkgPlayerChangeMonsLookAck ack;
										ack.uTargetId = pMonster->GetID();
										ack.dwChg = 82;
										g_xThreadBuffer.Reset();
										g_xThreadBuffer << ack;
										GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
										break;
									}
								}
							}
						}
						bBroadcast = true;
					}
				}
			}
		}break;
	case MEFF_KULOU:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
						pMsg->uOp = DELAY_MAKESLAVE;
						pMsg->uParam[0] = GetID();
						if(pMagic->bLevel < 4)
						{
							pMsg->uParam[1] = 29;
						}
						pMsg->dwDelayTime = GetTickCount() + 500;
						GetLocateScene()->PushDelayBuf(pMsg);*/
						DelayActionMakeSlave* pMakeSlaveAction = new DelayActionMakeSlave;
						pMakeSlaveAction->dwActiveTime = GetTickCount() + 500;
						pMakeSlaveAction->nSlaveId = 29;
						AddDelayAction(pMakeSlaveAction);

						bBroadcast = true;
					}
				}
			}
		}break;
	case MEFF_TEMPT:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						if(pObj)
						{
							if(pObj->GetType() == SOT_MONSTER)
							{
								MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);

								/*bool bTst = pMonster->IsElite();
								bTst = pMonster->IsLeader();*/

								if(pObj->GetObject_Curse() == 2 &&
									pMonster->GetMaster() == NULL &&
									!pMonster->IsLeader() &&
									!pMonster->IsElite())
								{
									//	可召唤
									int nSlaveSum = 0;
									int nInsertIndex = -1;
									//int nFoxCounter = 0;
									for(int i = 0; i < MAX_SLAVE_SUM; ++i)
									{
										if(m_pSlaves[i])
										{
											++nSlaveSum;

											/*if(m_pSlaves[i]->GetObject_ID() == 94)
											{
												++nFoxCounter;
											}*/
										}
										else
										{
											if(nInsertIndex == -1)
											{
												nInsertIndex = i;
											}
										}
									}

									/*if(nFoxCounter > 0 &&
										pMonster->GetObject_ID() == 94)
									{
										nInsertIndex = -1;
									}*/

									if(nSlaveSum < pMagic->bLevel)
									{
										int nAbl = GetRandomAbility(AT_MC);
										int nLevelInterval = (int)GetObject_Level() - (int)pObj->GetObject_Level();
										int nBasePercent = 70 - nAbl;
										bool bCanTempt = false;

#ifdef _DEBUG
										if(nLevelInterval >= 3)
										{
											bCanTempt = true;
										}
										else
										{
											bCanTempt = false;
										}
										//bCanTempt = true;
#else
										if(nLevelInterval >= 3)
										{
											bCanTempt = true;
										}
										else
										{
											bCanTempt = false;
										}
#endif

										if(bCanTempt)
										{
											bCanTempt = false;
											if(nBasePercent < 10)
											{
												bCanTempt = true;
											}
											else
											{
												if(0 == rand() % (nBasePercent / 10))
												{
													bCanTempt = true;
												}
											}

											if(bCanTempt &&
												nInsertIndex != -1)
											{
												pMonster->SetMaster(this);
												DWORD dwDefectTime = GetTickCount() + 45 * 60 * 1000 + nAbl * 30 * 1000;
												dwDefectTime = 0xFFFFFFFF;
												pMonster->SetDefectTime(dwDefectTime);
												m_pSlaves[nInsertIndex] = pMonster;
												pMonster->SetTarget(NULL);
												pMonster->SetCanDropItems(false);

												//	Other slaves have target?and the target is it..
												for(int i = 0; i < MAX_SLAVE_SUM; ++i)
												{
													if(m_pSlaves[i] != NULL &&
														m_pSlaves[i] != pMonster)
													{
														if(m_pSlaves[i]->GetType() == SOT_MONSTER)
														{
															MonsterObject* pSlave = static_cast<MonsterObject*>(m_pSlaves[i]);
															if((void*)pSlave->GetTarget() == (void*)pMonster)
															{
																pSlave->SetTarget(NULL);
															}
														}
													}
												}

												PkgPlayerUpdateAttribNtf ntf;
												ntf.bType = UPDATE_MASTER;
												ntf.uTargetId = pMonster->GetID();
												ntf.dwParam = GetID();
												g_xThreadBuffer.Reset();
												g_xThreadBuffer << ntf;
												GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
											}
										}
									}
								}
							}
						}
						bBroadcast = true;
					}
				}
			}
		}break;
	case MEFF_POISON:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						if(NULL != pObj)
						{
							if(pObj->GetType() == SOT_MONSTER)
							{
								if(static_cast<MonsterObject*>(pObj)->GetMaster() == NULL)
								{
									int nTime = GetRandomAbility(AT_SC) * 2000 + 5000;
									not.uParam3 = nTime;
									//SET_FLAG(m_dwHumEffFlag, MMASK_SHIELD);
									//m_dwHumEffTime[MMASK_SHIELD_INDEX] = GetTickCount() + nTime;
									//m_dwDefence = 0;
									int nPoisonParam = pMagic->bLevel;
									if(pMagic->bLevel > 0 &&
										pMagic->bLevel <= 7)
									{
										nPoisonParam *= 2;
									}
									pObj->SetEffStatus(MMASK_LVDU, nTime, nPoisonParam);
									if(static_cast<MonsterObject*>(pObj)->GetMapID() == GetMapID())
									{
										static_cast<MonsterObject*>(pObj)->SetTarget(this);
									}
									bBroadcast = true;

									SetSlaveTarget(pObj);
								}
							}
						}
						bBroadcast = true;
					}
				}
			}
		}break;
	case MEFF_HIDE:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);
						bBroadcast = true;

						//	Hide hero himself
						int nSC = GetRandomAbility(AT_SC);
						int nHideTime = nSC * 500 + pMagic->bLevel * 4000;

						//	Because of the time delay
						SetEffStatus(MMASK_HIDE, nHideTime + 500, 0);
						not.uParam3 = nHideTime;

						//	Now erase the hero target
						GetLocateScene()->EraseTarget(this);
					}
				}
			}
		}break;
	case MEFF_LIONROAR:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);
						bBroadcast = true;

						static const int s_nPalsyRange[3] = {2,3,4};
						int nCurX = GetUserData()->wCoordX;
						int nCurY = GetUserData()->wCoordY;

						int nLeftX = nCurX - s_nPalsyRange[pMagic->bLevel - 1];
						int nRightX = nCurX + s_nPalsyRange[pMagic->bLevel - 1];
						int nUpY = nCurY - s_nPalsyRange[pMagic->bLevel - 1];
						int nDownY = nCurY + s_nPalsyRange[pMagic->bLevel - 1];

						if(nLeftX < 0)
						{
							nLeftX = 0;
						}
						if(nUpY < 0)
						{
							nUpY = 0;
						}

						MapCellInfo* pCell = NULL;
						CELLDATALIST* pObjs = NULL;
						for(int i = nLeftX; i <= nRightX; ++i)
						{
							for(int j = nUpY; j <= nDownY; ++j)
							{
								pCell = GetLocateScene()->GetMapData(i, j);
								if(NULL != pCell)
								{
									if(NULL != pCell->pCellObjects)
									{
										pObjs = pCell->pCellObjects;

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
													if(pMonster->GetMaster() == NULL &&
														pMonster->GetUserData()->eGameState != OS_DEAD &&
														pMonster->GetObject_HP() != 0)
													{
#ifdef _DEBUG
#else
														if(GetObject_Level() >= pMonster->GetObject_Level() + 2)
#endif
														{
															//	can palsy this monster
															PkgPlayerSetEffectAck ack;
															ack.bShow = true;
															ack.dwMgcID = MMASK_STONE;
															ack.uTargetId = pMonster->GetID();
															ack.dwTime = pMagic->bLevel * 2000;
															g_xThreadBuffer.Reset();
															g_xThreadBuffer << ack;
															GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

															pMonster->SetEffStatus(MMASK_STONE, ack.dwTime, 0);
															pMonster->SetStoneRestore(true);
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
				}
			}
		}break;
	case MEFF_CHARMAC:
		{
			pMagic = GetUserMagic(LOWORD(req.uParam1));
			if(NULL != pMagic)
			{
				if(pMagic->bLevel > 0)
				{
					int nCost = GetMagicCost(pMagic);
					if(GetObject_MP() >= nCost)
					{
						nPreMP -= nCost;
						DecMP(nCost);
						m_pValid->DecMP(nCost);
						SyncMP(this);

						bBroadcast = true;
						CellData* pData = NULL;
						MapCellInfo* pCell = NULL;

						int nSC = GetRandomAbility(AT_SC);
						int nLastMs = 0;

						if(nSC < 70)
						{
							nLastMs = nSC * 8 * 1000;
						}
						else
						{
							nLastMs = 70 * 8 * 1000;
							nLastMs += (nSC - 70) * 3 * 1000;
						}

						if(nTargetX != -1 &&
							nTargetY != -1)
						{
							pCell = GetLocateScene()->GetMapData(nTargetX, nTargetY);
							if(pCell)
							{
								if(pCell->pCellObjects)
								{
									CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
									CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();
									CellData* pData = NULL;
									for(begiter; begiter != enditer; ++begiter)
									{
										pData = *begiter;
										if(pData->bType == CELL_MOVEOBJECT)
										{
											GameObject* pObj = (GameObject*)pData->pData;
											if(pObj->GetType() == SOT_MONSTER)
											{
												if(static_cast<MonsterObject*>(pObj)->GetMaster() != NULL)
												{
													HeroObject* pMaster = static_cast<HeroObject*>(pObj);
													int nDef = pMagic->bLevel * 2;
													if(pMaster == this)
													{
														//	给自己宠物增加防御力
														int nLevel = pObj->GetObject_Level();
														nLevel -= 1;
														nLevel /= 10;
														nLevel += 1;

														if(nLevel > 0)
														{
															nDef = nLevel * pMagic->bLevel;
														}
													}
													pObj->SetEffStatus(MMASK_CHARMAC, nLastMs, nDef);
												}
											}
											else if(pObj->GetType() == SOT_HERO)
											{
												HeroObject* pHero = static_cast<HeroObject*>(pObj);
												int nLevel = pObj->GetObject_Level();
												int nDef = 0;
												int nTime = 0;
												nLevel -= 1;
												nLevel /= 10;
												nLevel += 1;
												if(nLevel > 0)
												{
													if(pHero == this)
													{
														//	给自己加状态
														nDef = nLevel * pMagic->bLevel * 2;
														nTime = nLastMs;
													}
													else
													{
														nDef = nLevel * pMagic->bLevel;
														nTime = nLastMs / 4;
													}
													
													pObj->SetEffStatus(MMASK_CHARMAC, nTime, nDef);
													pHero->SendStatusInfo();
												}
											}
										}
									}
								}
							}
						}

						int nNextX = 0;
						int nNextY = 0;
						for(int i = 0; i < 8; ++i)
						{
							nNextX = nTargetX + s_nAround[2 * i];
							nNextY = nTargetY + s_nAround[2 * i + 1];
							pCell = GetLocateScene()->GetMapData(nNextX, nNextY);
							if(pCell)
							{
								if(pCell->pCellObjects)
								{
									CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
									CELLDATALIST::const_iterator enditer = pCell->pCellObjects->end();
									CellData* pData = NULL;
									for(begiter; begiter != enditer; ++begiter)
									{
										pData = *begiter;
										if(pData->bType == CELL_MOVEOBJECT)
										{
											GameObject* pObj = (GameObject*)pData->pData;
											if(pObj->GetType() == SOT_MONSTER)
											{
												if(static_cast<MonsterObject*>(pObj)->GetMaster() != NULL)
												{
													HeroObject* pMaster = static_cast<HeroObject*>(pObj);
													int nDef = pMagic->bLevel * 2;
													if(pMaster == this)
													{
														//	给自己宠物增加防御力
														int nLevel = pObj->GetObject_Level();
														nLevel -= 1;
														nLevel /= 10;
														nLevel += 1;

														if(nLevel > 0)
														{
															nDef = nLevel * pMagic->bLevel;
														}
													}
													pObj->SetEffStatus(MMASK_CHARMAC, nLastMs, nDef);
												}
											}
											else if(pObj->GetType() == SOT_HERO)
											{
												HeroObject* pHero = static_cast<HeroObject*>(pObj);
												int nLevel = pObj->GetObject_Level();
												int nDef = 0;
												int nTime = 0;
												nLevel -= 1;
												nLevel /= 10;
												nLevel += 1;
												if(nLevel > 0)
												{
													if(pHero == this)
													{
														//	给自己加状态
														nDef = nLevel * pMagic->bLevel * 2;
														nTime = nLastMs;
													}
													else
													{
														nDef = nLevel * pMagic->bLevel;
														nTime = nLastMs / 4;
													}

													pObj->SetEffStatus(MMASK_CHARMAC, nTime, nDef);
													pHero->SendStatusInfo();
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
		}break;
	}

	if(bBroadcast)
	{
#ifdef _DEBUG
#else
		//	Check if the player is cheating...
		/*if(nPreMP != GetUserData()->stAttrib.MP)
		{
			//GameWorld::GetInstance().Stop();
			GameWorld::GetInstance().Stop(STOP_HEROATTRIBERR);
#ifdef _DEBUG
			LOG(ERROR) << "MP err";
#endif
		}*/
#endif
		if(req.uTargetId != 0)
		{
			if(NULL == pObj)
			{
				if(!bCanCross)
				{
					not.uTargetId = GetID();
					if(dwTargetPos == 0)
					{
						not.uParam2 = req.uParam2;
					}
					else
					{
						not.uParam2 = dwTargetPos;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				//
				not.uTargetId = GetID();
				not.uUserId = req.uTargetId;
			}
		}
		else
		{
			not.uTargetId = GetID();
			not.uParam2 = req.uParam2;
		}

		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		m_dwLastSpellTime = GetTickCount();

		//IncWeaponGrow();
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::DoSpeHit(MonsterObject* _pMonster, HeroObject* _pHero, DWORD* _pHitStyle)
{
	//WORD wPosX = _pMonster->GetUserData()->wCoordX;
	//WORD wPosY = _pMonster->GetUserData()->wCoordY;
	*_pHitStyle = 0;

	int nDamage = 0;
	int nDC = 0;
	int nAC = 0;
	const UserMagic* pMgc = NULL;
	MapCellInfo* pCell = NULL;
	bool bProc = false;
	const UserMagic* pGongShaMgc = GetUserMagic(MEFF_GONGSHA);
	const UserMagic* pKtSwordMgc = GetUserMagic(MEFF_KTSWORD);
	bool bCanPk = CanPk();

	int nAttackDelayTime = 350 - GetServerNetDelay();
	if(nAttackDelayTime < 0)
	{
		nAttackDelayTime = 0;
	}

	static const int s_nKtSwordProp[3] = {12, 10, 8};

	static int s_nAround[] =
	{
		0,-1,
		1,-1,
		1,0,
		1,1,
		0,1,
		-1,1,
		-1,0,
		-1,-1
	};

	PkgPlayerEnableSkillNot ppesn;
	ppesn.uTargetId = GetID();

	SceneDelayMsg* pMsg = NULL;

	int nPreMP = GetUserData()->stAttrib.MP;

	if(m_dwHumEffFlag & MMASK_LIEHUO)
	{
		pMgc = GetUserMagic(MEFF_LIEHUO);
		if(pMgc != NULL)
		{
			if(pMgc->bLevel > 0)
			{
				//int nCost = GetMagicCost(pMgc);
				//if(nCost >= m_stData.stAttrib.MP)
				{
					*_pHitStyle = MEFF_LIEHUO;
					RESET_FLAG(m_dwHumEffFlag, MMASK_LIEHUO);

					ppesn.nSkillId = MEFF_LIEHUO;
					ppesn.bEnabled = false;
					SendPacket(ppesn);
					//nDamage = GetMagicDamage(pMgc, _pMonster);
					float fDamage = 0;

					if(pMgc->bLevel == 1)
					{
						fDamage = GetRandomAbility(AT_DC);
						fDamage *= 1.8f;
						nDamage = (int)fDamage;
					}
					else if(pMgc->bLevel == 2)
					{
						fDamage = GetRandomAbility(AT_DC);
						fDamage *= 2.6f;
						nDamage = (int)fDamage;
					}
					else if(pMgc->bLevel == 3)
					{
						fDamage = GetRandomAbility(AT_DC);
						fDamage *= 3.4f;
						nDamage = (int)fDamage;
					}

					int nAddLieHuo = m_xStates.GetAddLieHuoAddition();
					if(nAddLieHuo > 0)
					{
						float fMulti = 1.0f + (float)(nAddLieHuo) / 10;
						nDamage *= fMulti;
					}

					if(_pMonster)
					{
						_pMonster->ReceiveDamage(this, false, nDamage, nAttackDelayTime);
					}
					if(_pHero)
					{
						_pHero->ReceiveDamage(this, false, nDamage, nAttackDelayTime);
					}
					
					bProc = true;
				}
			}
		}
	}
	else if(m_dwHumEffFlag & MMASK_SLIEHUO)
	{
		pMgc = GetUserMagic(MEFF_SLIEHUO);
		if(pMgc != NULL)
		{
			if(pMgc->bLevel > 0)
			{
				//int nCost = GetMagicCost(pMgc);
				//if(nCost >= m_stData.stAttrib.MP)
				{
					*_pHitStyle = MEFF_SLIEHUO;
					RESET_FLAG(m_dwHumEffFlag, MMASK_SLIEHUO);

					ppesn.nSkillId = MEFF_SLIEHUO;
					ppesn.bEnabled = false;
					SendPacket(ppesn);
					//nDamage = GetMagicDamage(pMgc, _pMonster);
					float fDamage = 0;

					if(pMgc->bLevel == 1)
					{
						/*fDamage = GetRandomAbility(AT_DC);
						fDamage *= 1.8f;
						nDamage = (int)fDamage;*/
						fDamage = GetRandomAbility(AT_DC) * 2.0f;
					}
					else if(pMgc->bLevel == 2)
					{
						/*fDamage = GetRandomAbility(AT_DC);
						fDamage *= 2.2f;
						nDamage = (int)fDamage;*/
						fDamage = GetRandomAbility(AT_DC) * 2.6f;
					}
					else if(pMgc->bLevel == 3)
					{
						/*fDamage = GetRandomAbility(AT_DC);
						fDamage *= 2.6f;
						nDamage = (int)fDamage;*/
						fDamage = GetRandomAbility(AT_DC) * 3.2f;
					}
					nDamage = fDamage;

					int nSLieHuoAddition = m_xStates.GetAddSLieHuoAddition();
					if(nSLieHuoAddition != 0)
					{
						float fMulti = 1 + (float)nSLieHuoAddition / 10;
						nDamage *= fMulti;
					}

					int nOriginDamage = nDamage;
					if(_pMonster)
					{
						if(nOriginDamage != 0)
						{
							_pMonster->ReceiveDamage(this, false, nOriginDamage, nAttackDelayTime);
						}
					}
					if(_pHero &&
						nOriginDamage != 0)
					{
						_pHero->ReceiveDamage(this, false, nOriginDamage, nAttackDelayTime);
					}
					//	Next 3 position
					int nNextPosY = 0;
					int nNextPosX = 0;
					GameObject* pObj = NULL;
					MonsterObject* pMonster = NULL;
					float fRadio = 1.0f;
					GameObjectList xObjs;

					for(int i = 2; i < 6; ++i)
					{
						nNextPosX = GetUserData()->wCoordX + g_nMoveOft[GetUserData()->nDrt * 2] * i;
						nNextPosY = GetUserData()->wCoordY + g_nMoveOft[GetUserData()->nDrt * 2 + 1] * i;

						int nObjCount = GetLocateScene()->GetMappedObjects(nNextPosX, nNextPosY, 0, 0, xObjs, MAPPEDOBJECT_ALL_ALIVE);;

						if(0 != nObjCount)
						{
							GameObjectList::iterator begIter = xObjs.begin();
							GameObjectList::iterator endIter = xObjs.end();

							for(begIter;
								begIter != endIter;
								++begIter)
							{
								//GameObject* pMonster = (MonsterObject*)(*begIter);
								GameObject* pMonster = *begIter;

								if(pMonster->GetUserData()->eGameState == OS_DEAD ||
									pMonster->GetObject_HP() == 0)
								{
									continue;
								}

								fRadio = 1.0f - (i - 1) * 0.1f;
								//nDamage = nOriginDamage - pMonster->GetRandomAbility(AT_AC);
								//nDamage *= fRadio;
								nDamage = nOriginDamage * fRadio;

								if(nDamage > 0)
								{
									ReceiveDamageInfo info = {0};
									info.nStoneTime = STONE_TIME / 2;
									info.bFarPalsy = true;
									pMonster->ReceiveDamage(this, false, nDamage, nAttackDelayTime, &info);
								}
							}
						}
					}
					
					bProc = true;
				}
			}
		}
	}
	else if(m_dwHumEffFlag & MMASK_BANYEU)
	{
		pMgc = GetUserMagic(MEFF_BANYUE);
		if(pMgc != NULL)
		{
			if(pMgc->bLevel > 0)
			{
				int nCost = GetMagicCost(pMgc);
				if(nCost <= GetObject_MP())
				{
					*_pHitStyle = MEFF_BANYUE;

					DecMP(nCost);
					nPreMP -= nCost;
					m_pValid->DecMP(nCost);
					SyncMP(this);

					int nStartDrt = m_stData.nDrt - 1;
					int nAtkDrt = nStartDrt;
					int nAtkX = 0;
					int nAtkY = 0;

					float fAroundAtk = 0;
					if(pMgc->bLevel == 1)
					{
						fAroundAtk = 0.3f;
					}
					else if(pMgc->bLevel == 2)
					{
						fAroundAtk = 0.4f;
					}
					else if(pMgc->bLevel == 3)
					{
						fAroundAtk = 0.5f;
					}
					nDC = GetRandomAbility(AT_DC);

					for(int i = 0; i < 5; ++i)
					{
						nAtkDrt = nStartDrt + i;
						if(nAtkDrt < 0)
						{
							nAtkDrt = 8 + nAtkDrt;
						}
						if(nAtkDrt >= 8)
						{
							nAtkDrt = nAtkDrt - 8;
						}

						nAtkX = m_stData.wCoordX + s_nAround[nAtkDrt * 2];
						nAtkY = m_stData.wCoordY + s_nAround[nAtkDrt * 2 + 1];

						if(_pMonster)
						{
							if(nAtkX == _pMonster->GetUserData()->wCoordX &&
								nAtkY == _pMonster->GetUserData()->wCoordY)
							{
								if(nDC != 0)
								{
									_pMonster->ReceiveDamage(this, false, nDC, nAttackDelayTime);
								}
								continue;
							}
						}
						if(_pHero)
						{
							if(nAtkX == _pHero->GetUserData()->wCoordX &&
								nAtkY == _pHero->GetUserData()->wCoordY)
							{
								if(nDC != 0)
								{
									_pHero->ReceiveDamage(this, false, nDC, nAttackDelayTime);
								}
								continue;
							}
						}

						GameObjectList xObjs;
						int nObjCount = GetLocateScene()->GetMappedObjects(nAtkX, nAtkY, 0, 0, xObjs, MAPPEDOBJECT_ALL_ALIVE);;

						//if(0 != GetLocateScene()->GetMappedObjects(nAtkX, nAtkY, 0, 0, xObjs, MAPPEDOBJECT_MONSTER))
						if(0 != nObjCount)
						{
							nDamage = nDC * fAroundAtk;

							GameObjectList::iterator begIter = xObjs.begin();
							GameObjectList::iterator endIter = xObjs.end();

							for(begIter;
								begIter != endIter;
								++begIter)
							{
								//MonsterObject* pMonster = (MonsterObject*)(*begIter);
								GameObject* pMonster = *begIter;

								if(pMonster->GetUserData()->eGameState == OS_DEAD ||
									0 == pMonster->GetObject_HP())
								{
									continue;
								}

								ReceiveDamageInfo info = {0};
								info.bIgnoreAC = true;
								info.bFarPalsy = true;
								pMonster->ReceiveDamage(this, false, nDamage, nAttackDelayTime, &info);
							}
						}
					}
					bProc = true;
				}
			}
		}
	}

	//	计算特殊攻击概率 保证只计算一次概率
	bool bGongShaEff = false;
	bool bKtSwordEff = false;
	int nGongShaCrashNumber = 20;
	int nKtSwordCrashNumber = 0;
	int nAtkRand = m_xNormalAttackRand.GetPrevRand(0, 99);

	if(pGongShaMgc != NULL &&
		pGongShaMgc->bLevel > 0 &&
		//NULL != _pMonster)
		(NULL != _pMonster || NULL != _pHero))
	{
		//	20/100 1/5
		if(nAtkRand < nGongShaCrashNumber)
		{
			bGongShaEff = true;
		}
	}
	if(!bGongShaEff &&
		NULL != pKtSwordMgc &&
		pKtSwordMgc->bLevel > 0)
	{
		if(pKtSwordMgc->bLevel == 1)
		{
			nKtSwordCrashNumber = 8;
		}
		else if(pKtSwordMgc->bLevel == 2)
		{
			nKtSwordCrashNumber = 10;
		}
		else if(pKtSwordMgc->bLevel == 3)
		{
			nKtSwordCrashNumber = 12;
		}

		if(nAtkRand < nGongShaCrashNumber + nKtSwordCrashNumber)
		{
			bKtSwordEff = true;
		}
	}

	if(!bProc)
	{
		//	gongsha
		if(bGongShaEff)
		{
			*_pHitStyle = MEFF_GONGSHA;
			bProc = true;

			//	攻杀
			float fDamage = 0;
			if(pGongShaMgc->bLevel == 1)
			{
				//	
				fDamage = GetRandomAbility(AT_DC);
				fDamage *= 1.5f;
				nDamage = (int)fDamage;
			}
			else if(pGongShaMgc->bLevel == 2)
			{
				fDamage = GetRandomAbility(AT_DC);
				fDamage *= 2.0f;
				nDamage = (int)fDamage;
			}
			else if(pGongShaMgc->bLevel == 3)
			{
				fDamage = GetRandomAbility(AT_DC);
				fDamage *= 2.5f;
				nDamage = (int)fDamage;
			}

			if(nDamage > 0)
			{
				if(_pMonster)
				{
					_pMonster->ReceiveDamage(this, false, nDamage, nAttackDelayTime);
				}
				if(_pHero)
				{
					_pHero->ReceiveDamage(this, false, nDamage, nAttackDelayTime);
				}
			}
		}
	}

	if(!bProc)
	{
		if(bKtSwordEff)
		{
			*_pHitStyle = MEFF_KTSWORD;
			bProc = true;

			float fDamage = 0;

			if(pKtSwordMgc->bLevel == 1)
			{
				fDamage = GetRandomAbility(AT_DC) * 1.8f;
			}
			else if(pKtSwordMgc->bLevel == 2)
			{
				fDamage = GetRandomAbility(AT_DC) * 2.4f;
			}
			else if(pKtSwordMgc->bLevel == 3)
			{
				fDamage = GetRandomAbility(AT_DC) * 3.0f;
			}
			nDamage = fDamage;

			int nOriginDamage = nDamage;
			if(_pMonster)
			{
				nDamage = nOriginDamage;

				if(nDamage > 0)
				{
					ReceiveDamageInfo info = {0};
					info.bIgnoreAC = true;
					_pMonster->ReceiveDamage(this, false, nDamage, nAttackDelayTime, &info);
				}
			}
			if(_pHero)
			{
				nDamage = nOriginDamage;

				if(nDamage > 0)
				{
					ReceiveDamageInfo info = {0};
					info.bIgnoreAC = true;
					_pHero->ReceiveDamage(this, false, nDamage, nAttackDelayTime, &info);
				}
			}
			//	Next 2 position
			int nNextPosY = 0;
			int nNextPosX = 0;
			GameObject* pObj = NULL;
			MonsterObject* pMonster = NULL;
			float fRadio = 1.0f;
			GameObjectList xObjs;

			for(int i = 2; i < 4; ++i)
			{
				nNextPosX = GetUserData()->wCoordX + g_nMoveOft[GetUserData()->nDrt * 2] * i;
				nNextPosY = GetUserData()->wCoordY + g_nMoveOft[GetUserData()->nDrt * 2 + 1] * i;

				xObjs.clear();

				//GetLocateScene()->GetMappedObjects(nNextPosX, nNextPosY, 0, 0, xObjs, MAPPEDOBJECT_MONSTER);
				GetLocateScene()->GetMappedObjects(nNextPosX, nNextPosY, 0, 0, xObjs, MAPPEDOBJECT_ALL_ALIVE);
				GameObjectList::const_iterator begIter = xObjs.begin();
				GameObjectList::const_iterator endIter = xObjs.end();
				ReceiveDamageInfo info = {0};
				info.nStoneTime = STONE_TIME / 2;
				info.bIgnoreAC = true;
				info.bFarPalsy = true;

				for(begIter;
					begIter != endIter;
					++begIter)
				{
					//MonsterObject* pMonster = (MonsterObject*)(*begIter);
					GameObject* pMonster = *begIter;

					if(pMonster->GetUserData()->eGameState != OS_DEAD &&
						pMonster->GetObject_HP() != 0)
					{
						nDamage = nOriginDamage;
						fRadio = 1.0f - (i - 1) * 0.1f;
						nDamage *= fRadio;
						if(nDamage > 0)
						{
							pMonster->ReceiveDamage(this, false, nDamage, nAttackDelayTime, &info);
						}
					}
				}
			}
		}
	}
	
	if(!bProc)
	{
		if(m_dwHumEffFlag & MMASK_CISHA)
		{
			pMgc = GetUserMagic(MEFF_CISHA);
			int nNextPosX = 0;
			int nNextPosY = 0;
			nNextPosX = s_nAround[m_stData.nDrt * 2] * 2 + m_stData.wCoordX;
			nNextPosY = s_nAround[m_stData.nDrt * 2 + 1] * 2 + m_stData.wCoordY;

			if(pMgc != NULL)
			{
				if(pMgc->bLevel > 0)
				{
					bProc = true;
					*_pHitStyle = MEFF_CISHA;

					nDC = GetRandomAbility(AT_DC);

					if(_pMonster)
					{
						nAC = _pMonster->GetRandomAbility(AT_AC);
						nDamage = nDC;

						_pMonster->ReceiveDamage(this, false, nDamage, nAttackDelayTime);
					}
					if(_pHero)
					{
						nAC = _pHero->GetRandomAbility(AT_AC);
						nDamage = nDC;

						_pHero->ReceiveDamage(this, false, nDamage, nAttackDelayTime);
					}

					float fNextAtk = 0;
					if(pMgc->bLevel == 1)
					{
						fNextAtk = 0.5f;
					}
					else if(pMgc->bLevel == 2)
					{
						fNextAtk = 0.7f;
					}
					else if(pMgc->bLevel == 3)
					{
						fNextAtk = 0.9f;
					}
					else if(pMgc->bLevel == 4)
					{
						fNextAtk = 1.0f;
					}
					else if(pMgc->bLevel == 5)
					{
						fNextAtk = 1.2f;
					}
					else if(pMgc->bLevel == 6)
					{
						fNextAtk = 1.3f;
					}

					GameObjectList xObjs;
					ReceiveDamageInfo info = {0};
					info.bFarPalsy = true;
					info.nStoneTime = STONE_TIME / 2;
					info.bIgnoreAC = true;
					//GetLocateScene()->GetMappedObjects(nNextPosX, nNextPosY, 0, 0, xObjs, MAPPEDOBJECT_MONSTER);
					GetLocateScene()->GetMappedObjects(nNextPosX, nNextPosY, 0, 0, xObjs, MAPPEDOBJECT_ALL_ALIVE);

					for(GameObjectList::const_iterator begIter = xObjs.begin();
						begIter != xObjs.end();
						++begIter)
					{
						//MonsterObject* pMonster = (MonsterObject*)(*begIter);
						GameObject* pMonster = *begIter;

						if(pMonster->GetUserData()->eGameState != OS_DEAD &&
							pMonster->GetObject_HP() != 0)
						{
							pMonster->ReceiveDamage(this, false, nDamage, nAttackDelayTime, &info);
						}
					}
				}
			}
		}
	}

#ifdef _DEBUG
#else
	/*if(nPreMP != GetUserData()->stAttrib.MP)
	{
		//GameWorld::GetInstance().Stop();
		GameWorld::GetInstance().Stop(STOP_HEROATTRIBERR);
#ifdef _DEBUG
		LOG(ERROR) << "MP err";
#endif
	}*/
#endif

	if(!bProc)
	{
		nDC = GetRandomAbility(AT_DC);

		if(_pMonster)
		{
			_pMonster->ReceiveDamage(this, false, nDC, nAttackDelayTime);
			bProc = true;
		}
		if(_pHero)
		{
			_pHero->ReceiveDamage(this, false, nDC, nAttackDelayTime);
			bProc = true;
		}
	}

	if(bProc &&
		_pMonster &&
		_pMonster->GetMaster() == NULL)
	{
		//IncWeaponGrow();
	}
	return bProc;
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::ResetInteractiveDialog()
{
	s_xInteractiveDialogItemPkg.xItems.clear();
}

bool HeroObject::AddInteractiveDialogItem(int _nX, int _nY, int _nType, int _nId, const char* _pszText)
{
	if(_nType != kInteractiveDialogItem_Button &&
		_nType != kInteractiveDialogItem_Text)
	{
		LOG(ERROR) << "Invalid interactive dialog item to add, type:" << _nType;
		return false;
	}

	if(_nType == kInteractiveDialogItem_Button &&
		0 == _nId)
	{
		LOG(ERROR) << "Invalid button id:0";
		return false;
	}

	if(_nType == kInteractiveDialogItem_Text &&
		0 != _nId)
	{
		LOG(ERROR) << "Invalid text id: not 0";
		return false;
	}

	if(NULL == _pszText)
	{
		LOG(ERROR) << "Invalid string nil.";
		return false;
	}

	InteractiveDialogItem item;
	item.nX = _nX;
	item.nY = _nY; 
	item.uType = _nType;
	item.uId = _nId;
	item.xText = _pszText;

	s_xInteractiveDialogItemPkg.xItems.push_back(item);
	return true;
}

bool HeroObject::AddInteractiveDialogItem_Button(int _nX, int _nY, int _nId, const char *_pszText)
{
	return AddInteractiveDialogItem(_nX, _nY, kInteractiveDialogItem_Button, _nId, _pszText);
}

bool HeroObject::AddInteractiveDialogItem_CloseButton(int _nX, int _nY, const char* _pszText)
{
	return AddInteractiveDialogItem(_nX, _nY, kInteractiveDialogItem_Button, 1, _pszText);
}

bool HeroObject::AddInteractiveDialogItem_String(int _nX, int _nY, const char* _pszText)
{
	return AddInteractiveDialogItem(_nX, _nY, kInteractiveDialogItem_Text, 0, _pszText);
}

void HeroObject::ShowInteractiveDialog(NPCObject* _pNpc)
{
	s_xInteractiveDialogItemPkg.uTargetId = _pNpc->GetID();
	s_xInteractiveDialogItemPkg.nNpcId = _pNpc->GetObject_ID();

	SendPacket(s_xInteractiveDialogItemPkg);
}

void HeroObject::HideInteractiveDialog()
{
	HideQuestDlg();
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::ShowQuestDlg(NPCObject* _pnpc, int _questid, int _step)
{
	if(NULL == _pnpc)
	{
		return false;
	}
	PkgPlayerClickNPCAck ack;
	ack.uTargetId = _pnpc->GetID();
	ack.uUserId = GetID();
	ack.dwNPCID = _pnpc->GetObject_ID();
	ack.dwParam = MAKELONG(_questid, _step);
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << ack;
	SendPlayerBuffer(g_xThreadBuffer);

	return true;
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::HideQuestDlg()
{
	PkgPlayerHideQuestDlgAck ack;
	ack.uTargetId = GetID();
	SendPacket(ack);
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::ShowShopDlg(NPCObject* _pnpc, int _type)
{
#define MAX_SHOP_BUFFER_SIZE	(MAX_BIGSTORE_NUMBER * sizeof(ItemAttrib) + 1)
	static char* s_pBuffer = new char[MAX_SHOP_BUFFER_SIZE];

	if(_type == SHOP_SELLSHOP)
	{
		PkgPlayerShowShopAck ack;
		ack.bType = _type;

		for(int i = 0; i < _pnpc->GetItemSum(); ++i)
		{
			ack.xItems.push_back(_pnpc->GetItems()[i]);
		}
		ack.uTargetId = _pnpc->GetID();
		ack.uUserId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;

		SendPlayerBuffer(g_xThreadBuffer);
	}
	else if(_type == SHOP_STORAGE)
	{
		PkgPlayerStoreDataAck ack;
		ack.uTargetId = GetID();
		g_xThreadBuffer.Reset();

		BYTE bSize = 0;
		for(int i = 0; i < MAX_STORE_NUMBER; ++i)
		{
			if(GETITEMATB(&m_stStore[i], Type) != ITEM_NO)
			{
				++bSize;
				ObjectValid::DecryptAttrib(&m_stStore[i]);
			}
		}

		g_xThreadBuffer << bSize;
		if(bSize != 0)
		{
			for(int i = 0; i < MAX_STORE_NUMBER; ++i)
			{
				if(GETITEMATB(&m_stStore[i], Type) != ITEM_NO)
				{
					g_xThreadBuffer << m_stStore[i];
					ObjectValid::EncryptAttrib(&m_stStore[i]);
				}
			}
		}

		//char szBuf[MAX_STORE_NUMBER * sizeof(ItemAttrib) + 1];
		uLongf usrcsize = (uLongf)g_xThreadBuffer.GetLength();
		//uLongf udestsize = (uLongf)sizeof(szBuf);
		uLongf udestsize = (uLongf)MAX_SHOP_BUFFER_SIZE;

		int nRet = compress((Bytef*)s_pBuffer, &udestsize, (const Bytef*)g_xThreadBuffer.GetBuffer(), usrcsize);
		if(nRet == Z_OK)
		{
			ack.xData.resize(udestsize);
			/*for(int i = 0; i < udestsize; ++i)
			{
				ack.xData[i] = szBuf[i];
			}*/
			memcpy(&ack.xData[0], s_pBuffer, udestsize);
			SendPacket(ack);
		}
		else
		{
			LOG(WARNING) << "Can't compress the store data";
		}
	}
	else if(_type == SHOP_BIGSTORAGE)
	{
		PkgPlayerDialogDataAck ack;
		ack.nType = 0;
		ack.uTargetId = GetID();
		g_xThreadBuffer.Reset();

		BYTE bSize = 0;
		for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
		{
			if(GETITEMATB(&m_stBigStore[i], Type) != ITEM_NO)
			{
				++bSize;
				ObjectValid::DecryptAttrib(&m_stBigStore[i]);
			}
		}

		g_xThreadBuffer << bSize;
		if(bSize != 0)
		{
			for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
			{
				if(GETITEMATB(&m_stBigStore[i], Type) != ITEM_NO)
				{
					g_xThreadBuffer << m_stBigStore[i];
					ObjectValid::EncryptAttrib(&m_stBigStore[i]);
				}
			}
		}

		//char szBuf[MAX_STORE_NUMBER * sizeof(ItemAttrib) + 1];
		uLongf usrcsize = (uLongf)g_xThreadBuffer.GetLength();
		//uLongf udestsize = (uLongf)sizeof(szBuf);
		uLongf udestsize = (uLongf)MAX_SHOP_BUFFER_SIZE;

		int nRet = compress((Bytef*)s_pBuffer, &udestsize, (const Bytef*)g_xThreadBuffer.GetBuffer(), usrcsize);
		if(nRet == Z_OK)
		{
			ack.xData.resize(udestsize);
			/*for(int i = 0; i < udestsize; ++i)
			{
				ack.xData[i] = szBuf[i];
			}*/
			memcpy(&ack.xData[0], s_pBuffer, udestsize);
			SendPacket(ack);
		}
		else
		{
			LOG(WARNING) << "Can't compress the store data";
		}
	}
	else if(_type == SHOP_DONATE)
	{
		PkgPlayerShowShopAck ack;
		ack.bType = _type;
		ack.uTargetId = _pnpc->GetID();
		ack.uUserId = GetID();
		SendPacket(ack);
	}
	else if(_type == SHOP_IDENTIFY ||
		_type == SHOP_IDENTIFY_LOW ||
		_type == SHOP_UNBIND)
	{
		PkgPlayerShowShopAck ack;
		ack.bType = _type;
		ack.uTargetId = _pnpc->GetID();
		ack.uUserId = GetID();
		SendPacket(ack);
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::GetRandomAbility(ABILITY_TYPE _type)
{
	m_bLastAttackCritical = false;

	int nValue = GameObject::GetRandomAbility(_type);

	if(GetObject_Lucky() != 0)
	{
		int nLucky = GetObject_Lucky();
		if(nLucky > 10)
		{
			nLucky = 10;
		}
		if(nLucky > (rand() % 15 + 1))
		{
			if(_type == AT_DC)
			{
				nValue = GetObject_MaxDC();
			}
			else if(_type == AT_MC)
			{
				nValue = GetObject_MaxMC();
			}
			else if(_type == AT_SC)
			{
				nValue = GetObject_MaxSC();
			}
		}
	}

	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_ENERGYSHIELD))
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_ENERGYSHIELD);
		if(NULL != pMgc)
		{
			if(pMgc->bLevel > 0)
			{
				if(_type == AT_SC)
				{
					float fRadio = 1.0f;
					if(pMgc->bLevel <= 3)
					{
						fRadio += (float)pMgc->bLevel / 10;
					}
					else
					{
						fRadio += 0.3f;
						fRadio += (float)(pMgc->bLevel - 3) / 15;
					}
					nValue *= fRadio;
				}
			}
		}
	}

	if(m_stData.bJob == 0)
	{
		//	jingang
		const UserMagic* pMgc = GetUserMagic(MEFF_JINGANG);
		if(NULL != pMgc &&
			m_dwJinGangExpireTime != 0)
		{
			if(pMgc->bLevel > 0)
			{
				if(_type == AT_MAC ||
					_type == AT_AC)
				{
					nValue += 8 * pMgc->bLevel;
				}
			}
		}
	}

	int nStateValue = m_xStates.GetStateValue(EAID_CRITICAL);
	if(0 != nStateValue)
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_BLOODDRAGON);
		if(pMgc)
		{
			if(pMgc->bLevel > 0)
			{
				if(_type == AT_DC)
				{
					float fRadio = 1.0f + pMgc->bLevel * 0.2f;

					int nRandom = pMgc->bLevel * 15;
					if(nRandom > rand() % 100)
					{
						nValue *= fRadio;
					}
				}
			}
		}
	}

	if(_type == AT_DC ||
		_type == AT_SC ||
		_type == AT_MC)
	{
		if(CanCriticalAttack())
		{
			float fRadio = GetCriticalAttackLimit();
			nValue *= fRadio;
			m_bLastAttackCritical = true;
		}
	}

	static float s_fSuitSameLevelFactor[] = {
		1.0f,
		1.03f,
		1.06f,
		1.09f,
		1.12f,
		1.15f,
		1.20f,
		1.30f,
		1.40f
	};

	if (m_nExtraSuitType > 0 &&
		m_nExtraSuitType < 9)
	{
		if ((m_stData.bJob == 0 &&
			_type == AT_DC) ||
			(m_stData.bJob == 1 &&
			_type == AT_MC) ||
			(m_stData.bJob == 2 &&
			_type == AT_SC))
		{
			nValue *= s_fSuitSameLevelFactor[m_nExtraSuitType];
		}
	}

	/*nStateValue = m_xStates.GetStateValue(EAID_CRITICALPEC);
	if(0 != nStateValue)
	{
		if(_type == AT_DC ||
			_type == AT_SC ||
			_type == AT_MC)
		{
			float fCriticalPecLimit = m_xStates.GetStateValue(EAID_CRITICALLIMIT);
			if(fCriticalPecLimit > 0.75f)
			{
				fCriticalPecLimit = 0.75f;
			}

			float fRadio = 1.25f + fCriticalPecLimit;
			int nRandom = rand() % 25;

			if(nStateValue > nRandom)
			{
				nValue *= fRadio;
			}
		}
	}*/
	return nValue;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::CanCriticalAttack()
{
	int nStateValue = m_xStates.GetStateValue(EAID_CRITICALPEC);
	if(0 != nStateValue)
	{
		int nRandom = rand() % 40;

		if(nStateValue > nRandom)
		{
			return true;
		}
	}
	return false;
}

float HeroObject::GetCriticalAttackLimit()
{
	float fCriticalPecLimit = m_xStates.GetStateValue(EAID_CRITICALLIMIT);
	fCriticalPecLimit /= 100;
	if(fCriticalPecLimit > 1.5f)
	{
		fCriticalPecLimit = 1.5f;
	}

	float fRadio = 1.3f + fCriticalPecLimit;
	return fRadio;
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::SendChatMessage(std::string& _xMsg, unsigned int _dwExtra)
{
	PkgChatNot not;
	not.xMsg = _xMsg;
	not.uExtra = _dwExtra;
	not.uTargetId = GetID();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	SendPlayerBuffer(g_xThreadBuffer);
}
void HeroObject::SendChatMessage(const char* _pszMsg, unsigned int _dwExtra)
{
	PkgChatNot not;
	not.xMsg = _pszMsg;
	not.uExtra = _dwExtra;
	not.uTargetId = GetID();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	SendPlayerBuffer(g_xThreadBuffer);
}
void HeroObject::SendSystemMessage(const char* _pszMsg)
{
	std::string xMsg = "[系统]";
	xMsg += _pszMsg;
	SendChatMessage(xMsg, 1);
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::CountItem(int _nAttribID)
{
	ItemAttrib item;
	int nCounter = 0;

	if(GetRecordInItemTable(_nAttribID, &item))
	{
		if(item.type == ITEM_COST)
		{
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				ItemAttrib* pItem = NULL;
				pItem = GetItemByIndex(i);
				if(GETITEMATB(pItem, Type) != ITEM_NO &&
					GETITEMATB(pItem, ID) == _nAttribID)
				{
					//++nCounter;
					nCounter += GETITEMATB(pItem, AtkSpeed);
				}
			}
		}
		else
		{
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				ItemAttrib* pItem = NULL;
				pItem = GetItemByIndex(i);
				if(GETITEMATB(pItem, Type) != ITEM_NO &&
					GETITEMATB(pItem, ID) == _nAttribID)
				{
					++nCounter;
				}
			}
		}
	}
	
	/*for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		ItemAttrib* pItem = NULL;
		pItem = GetItemByIndex(i);
		if(GETITEMATB(pItem, ID) == _nAttribID)
		{
			++nCounter;
		}
	}*/
	return nCounter;
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::ClearItem(int _nAttribID, int _number)
{
	if(0 == _number)
	{
		return;
	}
	int nCounter = 0;

	PkgPlayerClearBagNtf ntf;
	ntf.uTargetId = GetID();

	PkgPlayerUpdateCostNtf ppucn;
	ppucn.uTargetId = GetID();

	ItemAttrib item;
	if(GetRecordInItemTable(_nAttribID, &item))
	{
		if(item.type == ITEM_COST)
		{
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], ID) == _nAttribID &&
					GETITEMATB(&m_xBag[i], Type) != ITEM_NO)
				{
					int nLeft = GETITEMATB(&m_xBag[i], AtkSpeed);

					if(nLeft >= _number - nCounter)
					{
						SETITEMATB(&m_xBag[i], AtkSpeed, nLeft - (_number - nCounter));
						ppucn.nNumber = GETITEMATB(&m_xBag[i], AtkSpeed);
						ppucn.dwTag = m_xBag[i].tag;
						SendPacket(ppucn);

						if(ppucn.nNumber == 0)
						{
							ZeroMemory(&m_xBag[i], sizeof(ItemAttrib));
							ObjectValid::EncryptAttrib(&m_xBag[i]);
						}

						return;
					}
					else
					{
						ppucn.nNumber = 0;
						ppucn.dwTag = m_xBag[i].tag;
						nCounter += nLeft;

						ZeroMemory(&m_xBag[i], sizeof(ItemAttrib));
						ObjectValid::EncryptAttrib(&m_xBag[i]);
						SendPacket(ppucn);
					}
				}
			}
		}
		else
		{
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], ID) == _nAttribID &&
					GETITEMATB(&m_xBag[i], Type) != ITEM_NO)
				{
					ntf.dwTag = m_xBag[i].tag;
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendPlayerBuffer(g_xThreadBuffer);
					ZeroMemory(&m_xBag[i], sizeof(ItemAttrib));
					ObjectValid::EncryptAttrib(&m_xBag[i]);
					++nCounter;
					if(nCounter == _number)
					{
						return;
					}
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AddItem_GM(int _nAttribID)
{
//#ifdef _DEBUG
	//	1. cost items
	ItemAttrib destItem;
	ZeroMemory(&destItem, sizeof(ItemAttrib));
	ItemAttrib* pBagItem = NULL;
	bool bRet = false;

	if(GetRecordInItemTable(_nAttribID, &destItem))
	{
		if(destItem.type == ITEM_COST)
		{
			//	1. cost items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				//	search the same item that already exists
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_COST &&
					GETITEMATB(&m_xBag[i], ID) == _nAttribID)
				{
					if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX &&
						!TEST_FLAG_BOOL(GETITEMATB(&m_xBag[i], AtkPois), POIS_MASK_BIND))
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(pBagItem == NULL)
			{
				//	search empty item
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
						m_xBag[i].tag != ITEMTAG_INQUERY)
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					pBagItem->atkSpeed = 1;
					//SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
				}
				else if(GETITEMATB(pBagItem, Type) == ITEM_COST)
				{
					int nSum = GETITEMATB(pBagItem, AtkSpeed);
					if(nSum < GRID_MAX)
					{
						++nSum;
						SETITEMATB(pBagItem, AtkSpeed, nSum);

						PkgPlayerUpdateCostNtf ntf;
						ntf.uTargetId = GetID();
						ntf.nNumber = nSum;
						ntf.dwTag = pBagItem->tag;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendPlayerBuffer(g_xThreadBuffer);

						bRet = true;
					}
				}
			}
		}
		else
		{
			if(destItem.type == ITEM_OTHER &&
				destItem.curse > 0 &&
				destItem.curse <= 8)
			{
				if(destItem.curse % 2 == 1)
				{
					destItem.lucky = 15 + rand() % 5;
				}
				else
				{
					destItem.lucky = 50 + rand() % 40;
				}
			}

			//	2.other items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
					m_xBag[i].tag != ITEMTAG_INQUERY)
				{
					pBagItem = &m_xBag[i];
					break;
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					//SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
				}
			}
		}
	}

	return bRet;
//#endif
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AddItemSuper_GM(int _nAttribID, int _nValue)
{
	//#ifdef _DEBUG
	//	1. cost items
	ItemAttrib destItem;
	ZeroMemory(&destItem, sizeof(ItemAttrib));
	ItemAttrib* pBagItem = NULL;
	bool bRet = false;

	if(GetRecordInItemTable(_nAttribID, &destItem))
	{
		if(destItem.type == ITEM_COST)
		{
			//	1. cost items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				//	search the same item that already exists
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_COST &&
					GETITEMATB(&m_xBag[i], ID) == _nAttribID)
				{
					if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX &&
						!TEST_FLAG_BOOL(GETITEMATB(&m_xBag[i], AtkPois), POIS_MASK_BIND))
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(pBagItem == NULL)
			{
				//	search empty item
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
						m_xBag[i].tag != ITEMTAG_INQUERY)
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					pBagItem->atkSpeed = 1;
					//SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
				}
				else if(GETITEMATB(pBagItem, Type) == ITEM_COST)
				{
					int nSum = GETITEMATB(pBagItem, AtkSpeed);
					if(nSum < GRID_MAX)
					{
						++nSum;
						SETITEMATB(pBagItem, AtkSpeed, nSum);

						PkgPlayerUpdateCostNtf ntf;
						ntf.uTargetId = GetID();
						ntf.nNumber = nSum;
						ntf.dwTag = pBagItem->tag;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendPlayerBuffer(g_xThreadBuffer);

						bRet = true;
					}
				}
			}
		}
		else
		{
			if(destItem.type == ITEM_OTHER &&
				destItem.curse > 0 &&
				destItem.curse <= 8)
			{
				if(destItem.curse % 2 == 1)
				{
					destItem.lucky = 15 + rand() % 5;
				}
				else
				{
					destItem.lucky = 50 + rand() % 40;
				}
			}

			//	2.other items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
					m_xBag[i].tag != ITEMTAG_INQUERY)
				{
					pBagItem = &m_xBag[i];
					break;
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();

					if (IsEquipItem(pBagItem->type))
					{
						GameWorld::GetInstance().UpgradeItemsWithAddition(pBagItem, _nValue);
					}
					//SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
				}
			}
		}
	}

	return bRet;
	//#endif
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AddItem(int _nAttribID)
{
	//	1. cost items
	ItemAttrib destItem;
	ZeroMemory(&destItem, sizeof(ItemAttrib));
	ItemAttrib* pBagItem = NULL;
	bool bRet = false;

	if(GetRecordInItemTable(_nAttribID, &destItem))
	{
		if(destItem.type == ITEM_COST)
		{
			//	1. cost items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				//	search the same item that already exists
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_COST &&
					GETITEMATB(&m_xBag[i], ID) == _nAttribID)
				{
					if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX &&
						TEST_FLAG_BOOL(GETITEMATB(&m_xBag[i], AtkPois), POIS_MASK_BIND))
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(pBagItem == NULL)
			{
				//	search empty item
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
						m_xBag[i].tag != ITEMTAG_INQUERY)
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					pBagItem->atkSpeed = 1;
					SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
				}
				else if(GETITEMATB(pBagItem, Type) == ITEM_COST)
				{
					int nSum = GETITEMATB(pBagItem, AtkSpeed);
					if(nSum < GRID_MAX)
					{
						++nSum;
						SETITEMATB(pBagItem, AtkSpeed, nSum);

						PkgPlayerUpdateCostNtf ntf;
						ntf.uTargetId = GetID();
						ntf.nNumber = nSum;
						ntf.dwTag = pBagItem->tag;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendPlayerBuffer(g_xThreadBuffer);

						bRet = true;
					}
				}
			}
		}
		else
		{
			//	2.other items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
					m_xBag[i].tag != ITEMTAG_INQUERY)
				{
					pBagItem = &m_xBag[i];
					break;
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
				}
			}
		}
	}

	return bRet;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AddSuperItem(int _nAttribID, int _nExt)
{
	//	1. cost items
	ItemAttrib destItem;
	ZeroMemory(&destItem, sizeof(ItemAttrib));
	ItemAttrib* pBagItem = NULL;
	bool bRet = false;

	if(GetRecordInItemTable(_nAttribID, &destItem))
	{
		if(destItem.type == ITEM_COST)
		{
			//	1. cost items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				//	search the same item that already exists
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_COST &&
					GETITEMATB(&m_xBag[i], ID) == _nAttribID)
				{
					if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX &&
						TEST_FLAG_BOOL(GETITEMATB(&m_xBag[i], AtkPois), POIS_MASK_BIND))
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(pBagItem == NULL)
			{
				//	search empty item
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
						m_xBag[i].tag != ITEMTAG_INQUERY)
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					pBagItem->atkSpeed = 1;
					SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
				}
				else if(GETITEMATB(pBagItem, Type) == ITEM_COST)
				{
					int nSum = GETITEMATB(pBagItem, AtkSpeed);
					if(nSum < GRID_MAX)
					{
						++nSum;
						SETITEMATB(pBagItem, AtkSpeed, nSum);

						PkgPlayerUpdateCostNtf ntf;
						ntf.uTargetId = GetID();
						ntf.nNumber = nSum;
						ntf.dwTag = pBagItem->tag;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendPlayerBuffer(g_xThreadBuffer);

						bRet = true;
					}
				}
			}
		}
		else
		{
			//	2.other items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
					m_xBag[i].tag != ITEMTAG_INQUERY)
				{
					pBagItem = &m_xBag[i];
					break;
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);

					//	upgrade item
					if (_nExt > 0 &&
						_nExt <= 8 &&
						IsEquipItem(pBagItem->type))
					{
						lua_State* pState = GameWorld::GetInstance().GetLuaState();
						lua_getglobal(pState, "mustUpgradeItem");
						if(lua_isfunction(pState, -1))
						{
							tolua_pushusertype(pState, pBagItem, "ItemAttrib");
							lua_pushnumber(pState, _nExt);

							if(0 != lua_pcall(pState, 2, 0, 0))
							{
								LOG(ERROR) << "Lua error : " << lua_tostring(pState, -1);
								lua_pop(pState, 1);
							}
						}
					}

					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
				}
			}
		}
	}

	return bRet;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::AddItemReturnTag(int _nAttribID)
{
	//	1. cost items
	ItemAttrib destItem;
	ZeroMemory(&destItem, sizeof(ItemAttrib));
	ItemAttrib* pBagItem = NULL;
	int nRet = 0;

	if(GetRecordInItemTable(_nAttribID, &destItem))
	{
		if(destItem.type == ITEM_COST)
		{
			//	1. cost items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				//	search the same item that already exists
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_COST &&
					GETITEMATB(&m_xBag[i], ID) == _nAttribID)
				{
					if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX &&
						TEST_FLAG_BOOL(GETITEMATB(&m_xBag[i], AtkPois), POIS_MASK_BIND))
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(pBagItem == NULL)
			{
				//	search empty item
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
						m_xBag[i].tag != ITEMTAG_INQUERY)
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					pBagItem->atkSpeed = 1;
					SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					nRet = pBagItem->tag;
				}
				else if(GETITEMATB(pBagItem, Type) == ITEM_COST)
				{
					int nSum = GETITEMATB(pBagItem, AtkSpeed);
					if(nSum < GRID_MAX)
					{
						++nSum;
						SETITEMATB(pBagItem, AtkSpeed, nSum);

						PkgPlayerUpdateCostNtf ntf;
						ntf.uTargetId = GetID();
						ntf.nNumber = nSum;
						ntf.dwTag = pBagItem->tag;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendPlayerBuffer(g_xThreadBuffer);

						nRet = pBagItem->tag;
					}
				}
			}
		}
		else
		{
			//	2.other items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
					m_xBag[i].tag != ITEMTAG_INQUERY)
				{
					pBagItem = &m_xBag[i];
					break;
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					nRet = pBagItem->tag;
				}
			}
		}
	}

	return nRet;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AddItemNoBind(int _nAttribID)
{
	//	1. cost items
	ItemAttrib destItem;
	ZeroMemory(&destItem, sizeof(ItemAttrib));
	ItemAttrib* pBagItem = NULL;
	bool bRet = false;

	if(GetRecordInItemTable(_nAttribID, &destItem))
	{
		if(destItem.type == ITEM_COST)
		{
			//	1. cost items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				//	search the same item that already exists
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_COST &&
					GETITEMATB(&m_xBag[i], ID) == _nAttribID)
				{
					if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX &&
						TEST_FLAG_BOOL(GETITEMATB(&m_xBag[i], AtkPois), POIS_MASK_BIND))
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(pBagItem == NULL)
			{
				//	search empty item
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
						m_xBag[i].tag != ITEMTAG_INQUERY)
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					pBagItem->atkSpeed = 1;
					//SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
				}
				else if(GETITEMATB(pBagItem, Type) == ITEM_COST)
				{
					int nSum = GETITEMATB(pBagItem, AtkSpeed);
					if(nSum < GRID_MAX)
					{
						++nSum;
						SETITEMATB(pBagItem, AtkSpeed, nSum);

						PkgPlayerUpdateCostNtf ntf;
						ntf.uTargetId = GetID();
						ntf.nNumber = nSum;
						ntf.dwTag = pBagItem->tag;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendPlayerBuffer(g_xThreadBuffer);

						bRet = true;
					}
				}
			}
		}
		else
		{
			//	2.other items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
					m_xBag[i].tag != ITEMTAG_INQUERY)
				{
					pBagItem = &m_xBag[i];
					break;
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					//SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
				}
			}
		}
	}

	return bRet;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::AddItemNoBindReturnTag(int _nAttribID)
{
	//	1. cost items
	ItemAttrib destItem;
	ZeroMemory(&destItem, sizeof(ItemAttrib));
	ItemAttrib* pBagItem = NULL;
	bool bRet = false;
	int nRet = 0;

	if(GetRecordInItemTable(_nAttribID, &destItem))
	{
		if(destItem.type == ITEM_COST)
		{
			//	1. cost items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				//	search the same item that already exists
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_COST &&
					GETITEMATB(&m_xBag[i], ID) == _nAttribID)
				{
					if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX &&
						TEST_FLAG_BOOL(GETITEMATB(&m_xBag[i], AtkPois), POIS_MASK_BIND))
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(pBagItem == NULL)
			{
				//	search empty item
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
						m_xBag[i].tag != ITEMTAG_INQUERY)
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					pBagItem->atkSpeed = 1;
					//SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
					nRet = pBagItem->tag;
				}
				else if(GETITEMATB(pBagItem, Type) == ITEM_COST)
				{
					int nSum = GETITEMATB(pBagItem, AtkSpeed);
					if(nSum < GRID_MAX)
					{
						++nSum;
						SETITEMATB(pBagItem, AtkSpeed, nSum);

						PkgPlayerUpdateCostNtf ntf;
						ntf.uTargetId = GetID();
						ntf.nNumber = nSum;
						ntf.dwTag = pBagItem->tag;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendPlayerBuffer(g_xThreadBuffer);

						bRet = true;
						nRet = pBagItem->tag;
					}
				}
			}
		}
		else
		{
			//	2.other items
			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
					m_xBag[i].tag != ITEMTAG_INQUERY)
				{
					pBagItem = &m_xBag[i];
					break;
				}
			}

			if(NULL != pBagItem)
			{
				if(GETITEMATB(pBagItem, Type) == ITEM_NO)
				{
					memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
					pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
					//SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
					ObjectValid::EncryptAttrib(pBagItem);

					PkgPlayerGainItemNtf ntf;
					ntf.uTargetId = GetID();
					ntf.stItem = *pBagItem;
					ObjectValid::DecryptAttrib(&ntf.stItem);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					SendBuffer(GetUserIndex(), &g_xThreadBuffer);

					bRet = true;
					nRet = pBagItem->tag;
				}
			}
		}
	}

	return nRet;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AddBatchItem(int _nAttribID, int _nSum)
{
	//	1. cost items
	ItemAttrib destItem;
	ZeroMemory(&destItem, sizeof(ItemAttrib));
	ItemAttrib* pBagItem = NULL;
	bool bRet = false;

	if(GetRecordInItemTable(_nAttribID, &destItem))
	{
		if(destItem.type == ITEM_COST)
		{
			int nNeedGrid = _nSum / GRID_MAX;
			if(_nSum % GRID_MAX != 0)
			{
				++nNeedGrid;
			}

			if(GetBagEmptySum() < nNeedGrid)
			{
				return false;
			}

			int nItemSum = _nSum;

			while(nItemSum > 0)
			{
				if(nItemSum >= GRID_MAX)
				{
					nItemSum -= GRID_MAX;

					//	add grid max items
					pBagItem = NULL;

					for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
					{
						if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
							m_xBag[i].tag != ITEMTAG_INQUERY)
						{
							pBagItem = &m_xBag[i];
							break;
						}
					}

					if(NULL != pBagItem)
					{
						memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
						pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
						pBagItem->atkSpeed = GRID_MAX;
						SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
						ObjectValid::EncryptAttrib(pBagItem);

						PkgPlayerGainItemNtf ntf;
						ntf.uTargetId = GetID();
						ntf.stItem = *pBagItem;
						ObjectValid::DecryptAttrib(&ntf.stItem);
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendBuffer(GetUserIndex(), &g_xThreadBuffer);
					}
					else
					{
						return false;
					}
				}
				else
				{
					//	add left items
					pBagItem = NULL;

					for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
					{
						if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
							m_xBag[i].tag != ITEMTAG_INQUERY)
						{
							pBagItem = &m_xBag[i];
							break;
						}
					}

					if(NULL != pBagItem)
					{
						memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
						pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
						pBagItem->atkSpeed = nItemSum;
						SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
						ObjectValid::EncryptAttrib(pBagItem);

						PkgPlayerGainItemNtf ntf;
						ntf.uTargetId = GetID();
						ntf.stItem = *pBagItem;
						ObjectValid::DecryptAttrib(&ntf.stItem);
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendBuffer(GetUserIndex(), &g_xThreadBuffer);
					}
					else
					{
						return false;
					}

					nItemSum = 0;
				}
			}
		}
	}

	return bRet;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AddCostItem(int _nAttribID)
{
	return AddItem(_nAttribID);

	if(GetAllEmptySum() < 1)
	{
		return false;
	}
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
			m_xBag[i].tag != ITEMTAG_INQUERY)
		{
			DBOperationParam* pParam = new DBOperationParam;
			pParam->dwOperation = DO_QUERY_ITEMATTRIB;
			pParam->dwParam[1] = MAKELONG(GetMapID(), _nAttribID);
			pParam->dwParam[0] = (DWORD)&m_xBag[i];
			pParam->dwParam[2] = MAKELONG(IE_ADDPLAYERITEM, GetID());
			m_xBag[i].tag = ITEMTAG_INQUERY;
			//m_xBag[i].type = ITEM_INQUERY;
			DBThread::GetInstance()->AsynExecute(pParam);
			return true;
		}
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::AddMoney(int _nMoney)
{
	if(_nMoney > 0)
	{
		int nMoney = 0;
		if(IsEncrypt())
		{
			nMoney = DecryptValue(ATB_MONEY_MASK, m_nMoney);
		}
		else
		{
			nMoney = m_nMoney;
		}
		nMoney += _nMoney;
		if(nMoney > MAX_MONEY)
		{
			nMoney = MAX_MONEY;
		}

		if(IsEncrypt())
		{
			m_nMoney = EncryptValue(ATB_MONEY_MASK, nMoney);
		}
		else
		{
			m_nMoney = nMoney;
		}

		PkgPlayerUpdateAttribNtf uantf;
		uantf.bType = UPDATE_MONEY;
		uantf.dwParam = GetMoney();
		uantf.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << uantf;
		SendPlayerBuffer(g_xThreadBuffer);
	}
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::SyncDonateLeft()
{
	PkgPlayerUpdateAttribNtf uantf;
	uantf.bType = UPDATE_DONATELFET;
	uantf.dwParam = GetDonateLeft();
	uantf.uTargetId = GetID();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << uantf;
	SendPlayerBuffer(g_xThreadBuffer);
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::MinusMoney(int _nMoney)
{
	if(_nMoney > 0)
	{
		int nMoney = 0;
		if(IsEncrypt())
		{
			nMoney = DecryptValue(ATB_MONEY_MASK, m_nMoney);
		}
		else
		{
			nMoney = m_nMoney;
		}
		nMoney -= _nMoney;
		if(nMoney < 0)
		{
			nMoney = 0;
		}
		if(IsEncrypt())
		{
			m_nMoney = EncryptValue(ATB_MONEY_MASK, nMoney);
		}
		else
		{
			m_nMoney = nMoney;
		}

		PkgPlayerUpdateAttribNtf uantf;
		uantf.bType = UPDATE_MONEY;
		uantf.dwParam = GetMoney();
		uantf.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << uantf;
		SendPlayerBuffer(g_xThreadBuffer);
	}
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AddUserMagic(DWORD _dwMgcID)
{
	if(m_stData.bJob > 2)
	{
		return false;
	}
	else if(m_stData.bJob == 0)
	{
		if(_dwMgcID > MEFF_DC_BEGIN &&
			_dwMgcID < MEFF_DC_END)
		{
			m_xMagics[_dwMgcID - MEFF_DC_BEGIN - 1].bLevel = 1;
			m_xMagics[_dwMgcID - MEFF_DC_BEGIN - 1].pInfo = &g_xMagicInfoTable[_dwMgcID];
			return true;
		}
	}
	else if(m_stData.bJob == 1)
	{
		if(_dwMgcID > MEFF_MC_BEGIN &&
			_dwMgcID < MEFF_MC_END)
		{
			m_xMagics[_dwMgcID - MEFF_MC_BEGIN - 1].bLevel = 1;
			m_xMagics[_dwMgcID - MEFF_MC_BEGIN - 1].pInfo = &g_xMagicInfoTable[_dwMgcID];
			return true;
		}
	}
	else if(m_stData.bJob == 2)
	{
		if(_dwMgcID > MEFF_SC_BEGIN &&
			_dwMgcID < MEFF_SC_END)
		{
			m_xMagics[_dwMgcID - MEFF_SC_BEGIN - 1].bLevel = 1;
			m_xMagics[_dwMgcID - MEFF_SC_BEGIN - 1].pInfo = &g_xMagicInfoTable[_dwMgcID];
			return true;
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
const UserMagic* HeroObject::GetUserMagic(DWORD _dwMgcID)
{
	int nIndex = 0;
	if(m_stData.bJob == 0)
	{
		if(_dwMgcID > MEFF_DC_BEGIN &&
			_dwMgcID < MEFF_DC_END)
		{
			return &m_xMagics[_dwMgcID - MEFF_DC_BEGIN - 1];
		}
	}
	else if(m_stData.bJob == 1)
	{
		if(_dwMgcID > MEFF_MC_BEGIN &&
			_dwMgcID < MEFF_MC_END)
		{
			return &m_xMagics[_dwMgcID - MEFF_MC_BEGIN - 1];
		}
	}
	else if(m_stData.bJob == 2)
	{
		if(_dwMgcID > MEFF_SC_BEGIN &&
			_dwMgcID < MEFF_SC_END)
		{
			return &m_xMagics[_dwMgcID - MEFF_SC_BEGIN - 1];
		}
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::UpgradeUserMagic(DWORD _dwMgcID)
{
	const UserMagic* pMgc = GetUserMagic(_dwMgcID);
	if(NULL == pMgc)
	{
		return false;
	}
	if(pMgc->bLevel == 0 ||
		pMgc->pInfo == NULL)
	{
		return false;
	}

	UserMagic* pWriteMgc = const_cast<UserMagic*>(pMgc);
	++pWriteMgc->bLevel;
	if(pWriteMgc->bLevel > 6)
	{
		pWriteMgc->bLevel = 6;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::HaveSlave()
{
	for(int i = 0; i < MAX_SLAVE_SUM; ++i)
	{
		if(m_pSlaves[i] != NULL)
		{
			return true;
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::SwitchScene(DWORD _dwMapID, WORD _wPosX, WORD _wPosY)
{
	GameScene* pNextScene = GameSceneManager::GetInstance()->GetScene(_dwMapID);
	GameScene* pCurScene = GetLocateScene();

	if(_dwMapID == pCurScene->GetMapID())
	{
		return false;
	}

	if(pNextScene)
	{
		pCurScene->AddWaitRemove(this);
		pNextScene->AddWaitInsert(this);

		//if(!pCurScene->RemoveMappedObject(m_stData.wCoordX, m_stData.wCoordY, this))
		if(!pCurScene->RemoveMappedObject(GetValidPositionX(), GetValidPositionY(), this))
		{
			char szName[20];
			ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
			LOG(ERROR) << "Name:" << szName << " Coordinate x:" << GetUserData()->wCoordX << " y:" << GetUserData()->wCoordY
				<< "Valid pos x:" << GetValidPositionX() << " pos y:" << GetValidPositionY();
		}

		//	给其它玩家发送动画包
		if(IsNeedTransAni())
		{
			PkgPlayerPlayAniAck pppaa;
			pppaa.uTargetId = GetID();
			pppaa.wAniID = 1006;
			pppaa.xPos.push_back(MAKELONG(m_stData.wCoordX, m_stData.wCoordY));
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << pppaa;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer, GetID());
		}

		pCurScene->EraseTarget(this);
		m_stData.wCoordX = _wPosX;
		m_stData.wCoordY = _wPosY;
		// Test? [11/13/2013 yuanxj]
		m_stData.wMapID = _dwMapID;
		// end [11/13/2013 yuanxj]

		PkgPlayerChangeMapAck ack;
		ack.uTargetId = GetID();
		ack.wMapID = pNextScene->GetMapResID();
		ack.wPosY = m_stData.wCoordY;
		ack.wPosX = m_stData.wCoordX;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		SendPlayerBuffer(g_xThreadBuffer);

		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::GainExp(int _expr)
{
	int nExpr = GetObject_Expr();
	nExpr += _expr;
	SetObject_Expr(nExpr);

	if(GetObject_Expr() >= GetObject_MaxExpr())
	{
		if(GetObject_Level() >= MAX_LEVEL)
		{
			SetObject_Expr(GetObject_MaxExpr());
			return;
		}
		SetObject_Level(GetObject_Level() + 1);
		SetObject_Expr(nExpr - GetObject_MaxExpr());
//PROTECT_START
		//SetObject_MaxExpr(g_nExprTable[GetObject_Level() - 1]);
		SetObject_MaxExpr(GetGlobalExpr(GetObject_Level()));
//PROTECT_END
		//int nNextValue = g_nHPTable[GetObject_Level() - 1][m_stData.bJob];
		int nNextValue = GetGlobalHP(GetObject_Level(), m_stData.bJob);
		SetObject_HP(nNextValue);
		SetObject_MaxHP(nNextValue);
		//nNextValue = g_nMPTable[GetObject_Level() - 1][m_stData.bJob];
		nNextValue = GetGlobalMP(GetObject_Level(), m_stData.bJob);
		SetObject_MP(nNextValue);
		SetObject_MaxMP(nNextValue);
		m_pValid->SetHP(GetObject_MaxHP());
		m_pValid->SetMP(GetObject_MaxMP());
		PkgPlayerUpdateAttribNtf uantf;
		uantf.bType = UPDATE_LEVEL;
		uantf.uTargetId = GetID();
		uantf.dwParam = GetObject_Level();
		uantf.dwExtra = GetObject_Expr();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << uantf;
		SendPlayerBuffer(g_xThreadBuffer);

		if(GetObject_Level() % 7 == 0)
		{
			RefleshAttrib();
		}

		OnPlayerLevelUp(GetObject_Level());
	}
	else
	{
		PkgPlayerUpdateAttribNtf uantf;
		uantf.bType = UPDATE_EXP;
		uantf.uTargetId = GetID();
		uantf.dwParam = GetObject_Expr();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << uantf;
		SendPlayerBuffer(g_xThreadBuffer);
	}
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::CheckDoorEvent()
{
	MapCellInfo* pCellInfo = GetLocateScene()->GetMapData(m_stData.wCoordX, m_stData.wCoordY);
	if(NULL == pCellInfo)
	{
		return;
	}
	if(!pCellInfo->pCellObjects)
	{
		return;
	}
	bool bCanEnter = true;

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

				lua_getglobal(GetLocateScene()->GetLuaState(), "CanOpenDoor");
				lua_pushinteger(GetLocateScene()->GetLuaState(), (int)m_stData.wCoordX);
				lua_pushinteger(GetLocateScene()->GetLuaState(), (int)m_stData.wCoordY);
				tolua_pushusertype(GetLocateScene()->GetLuaState(), this, "HeroObject");

				if(0 != lua_pcall(GetLocateScene()->GetLuaState(), 3, 1, 0))
				{
					LOG(WARNING) << "CanOpenDoor function can't find:" << lua_tostring(GetLocateScene()->GetLuaState(), -1);
					lua_pop(GetLocateScene()->GetLuaState(), 1);
				}
				else
				{
					bCanEnter = lua_toboolean(GetLocateScene()->GetLuaState(), -1);
					lua_pop(GetLocateScene()->GetLuaState(), 1);
				}

				if(bCanEnter)
				{
					SetNeedTransAni(false);
					SwitchScene(pDoorEvt->wMapID, pDoorEvt->wPosX, pDoorEvt->wPoxY);
				}
				break;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::MakeSlave(int _slaveid)
{
	int nMakeX = 0;
	int nMakeY = 0;

	int nNextSlaveIndex = -1;
	for(int i = 0; i < MAX_SLAVE_SUM; ++i)
	{
		if(m_pSlaves[i] == NULL)
		{
			nNextSlaveIndex = i;
			break;
		}
	}
	if(nNextSlaveIndex == -1)
	{
		return false;
	}
	for(int i = 0; i < 8; ++i)
	{
		nMakeX = m_stData.wCoordX + g_nMoveOft[i * 2];
		nMakeY = m_stData.wCoordY + g_nMoveOft[i * 2 + 1];
		if(GetLocateScene()->CanThrough(nMakeX, nMakeY))
		{
			MonsterObject* pMons = NULL;
			if(_slaveid == MONSTER_SHENSHOU)
			{
				pMons = new ShenShouMonster;
			}
			else if(_slaveid == 62)
			{
				pMons = new SuperShenShouMonster;
			}
			else if(_slaveid == MONSTER_KULOU)
			{
				pMons = new KulouMonster;
			}
			else if(_slaveid == 99)
			{
				pMons = new BaiHuMonster;
			}
			else if(_slaveid == 144)
			{
				pMons = new BowManMonster;
			}
			else
			{
				pMons = new MonsterObject;
			}
			pMons->SetMaster(this);
			pMons->SetID(GameWorld::GetInstance().GenerateObjectID());
			pMons->GetUserData()->stAttrib.id = _slaveid;
			pMons->GetUserData()->wCoordX = nMakeX;
			pMons->GetUserData()->wCoordY = nMakeY;
			pMons->GetUserData()->wMapID = (WORD)m_stData.wMapID;
			m_pSlaves[nNextSlaveIndex] = pMons;

			int nCurSlaveCount = GetSlaveCount();

#ifdef _SYNC_CREATE
			if(GetRecordInMonsterTable(_slaveid, &pMons->GetUserData()->stAttrib))
			{
				if(_slaveid == 99)
				{
					const UserMagic* pMgc = GetUserMagic(MEFF_SUMMONTIGER);
					if(pMgc)
					{
						if(pMgc->bLevel > 3)
						{
							int nDcIncrease = pMgc->bLevel - 3;
							nDcIncrease *= 8;
							//pMons->SetObject_DC(pMons->GetObject_DC() + nDcIncrease);
							pMons->SetObject_MaxDC(pMons->GetObject_MaxDC() + nDcIncrease);

							DWORD dwDCGap = pMons->GetObject_MaxMC();
							if(dwDCGap > 1000)
							{
								dwDCGap -= 500;
							}
							pMons->SetObject_MaxMC(dwDCGap);
						}
					}
				}
				//	降低宠物属性
				if(nCurSlaveCount == 2)
				{
					/*if(_slaveid == 99 ||
						_slaveid == 62)
					{
						pMons->SetObject_DC(0);
						pMons->SetObject_MaxDC(pMons->GetObject_MaxDC() / 3);
						pMons->SetObject_AC(0);
						pMons->SetObject_MaxAC(pMons->GetObject_MaxAC() / 8);
						pMons->SetObject_MAC(0);
						pMons->SetObject_MaxMAC(pMons->GetObject_MaxMAC() / 8);
						pMons->SetObject_MC(pMons->GetObject_MC() + 0);
						pMons->SetObject_MaxMC(pMons->GetObject_MaxMC() + 350);
					}*/
				}


				pMons->EncryptObject();

				//	Never evolute

				if(pMons->GetLocateScene()->InsertNPC(pMons))
				{
#ifdef _DEBUG
					LOG(INFO) << "地图[" << GetMapID() << "]生成Slave[" << pMons->GetObject_ID() << "]成功";
#endif
					pMons->GetLocateScene()->OnMonsterCreated(_slaveid, pMons);
				}
				else
				{
					SAFE_DELETE(pMons);
				}
			}
			else
			{
				SAFE_DELETE(pMons);
			}
#else
			DBOperationParam* pParam = new DBOperationParam;
			pParam->dwOperation = DO_QUERY_MONSATTRIB;
			pParam->dwParam[0] = (DWORD)pMons;
			pParam->dwParam[1] = pMons->GetUserData()->stAttrib.id;
			pParam->dwParam[2] = ME_MAKESLAVE;
			DBThread::GetInstance()->AsynExecute(pParam);
#endif
			return true;
		}
	}
	
	return false;
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::KillAllSlave()
{
	MonsterObject* pSlave = NULL;
	PkgPlayerUpdateAttribNtf ntf;
	for(int i = 0; i < MAX_SLAVE_SUM; ++i)
	{
		pSlave = static_cast<MonsterObject*>(m_pSlaves[i]);
		if(pSlave)
		{
			pSlave->KilledByMaster();

			pSlave->SetMaster(NULL);
			pSlave->SetMasterIdBeforeDie(GetID());
			m_pSlaves[i] = NULL;

			g_xThreadBuffer.Reset();
			ntf.bType = UPDATE_MASTER;
			ntf.dwParam = 0;
			ntf.uTargetId = pSlave->GetID();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ntf;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::SetSlaveTarget(GameObject* _pTarget)
{
	MonsterObject* pSlave = NULL;
	for(int i = 0; i < MAX_SLAVE_SUM; ++i)
	{
		pSlave = static_cast<MonsterObject*>(m_pSlaves[i]);
		if(pSlave)
		{
			if(pSlave->GetTarget() == NULL)
			{
				if(_pTarget == NULL)
				{
					pSlave->SetTarget(_pTarget);
				}
				else
				{
					if(_pTarget->GetMapID() == pSlave->GetMapID())
					{
						pSlave->SetTarget(_pTarget);
					}
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::ClearSlave(GameObject* _pSlave)
{
	for(int i = 0; i < MAX_SLAVE_SUM; ++i)
	{
		if(m_pSlaves[i] == _pSlave)
		{
			m_pSlaves[i] = NULL;
			return true;
		}
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::GetMagicACDefence(int _damage)
{
	int nDefence = 0;
	if(m_stData.bJob == 0)
	{
		nDefence = 0;
	}
	else if(m_stData.bJob == 1)
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_SHIELD);
		if(NULL != pMgc)
		{
			if(pMgc->bLevel > 0 &&
				TEST_FLAG(m_dwHumEffFlag, MMASK_SHIELD))
			{
				float fMinDefence = 0.1f;
				float fMaxDefence = 0.0f;

				fMinDefence *= pMgc->bLevel;
				fMaxDefence = fMinDefence + 0.2f;

				float fActualDefence = fMinDefence;
				int nMC = GetRandomAbility(AT_MC);
				fActualDefence += (float)nMC / 128;
				if(fActualDefence > fMaxDefence)
				{
					fActualDefence = fMaxDefence;
				}
				nDefence = _damage * fActualDefence;
			}
		}
	}
	else if(m_stData.bJob == 2)
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_CHARMAC);
		{
			if(NULL != pMgc)
			{
				if(pMgc->bLevel > 0 &&
					TEST_FLAG(m_dwHumEffFlag, MMASK_CHARMAC))
				{
					nDefence = pMgc->bLevel * 2;
				}
			}
		}
	}
	return nDefence;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::GetMagicMACDefence(int _damage)
{
	int nDefence = 0;
	if(m_stData.bJob == 0)
	{
		nDefence = 0;
	}
	else if(m_stData.bJob == 1)
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_SHIELD);
		if(NULL != pMgc)
		{
			if(pMgc->bLevel > 0 &&
				TEST_FLAG(m_dwHumEffFlag, MMASK_SHIELD))
			{
				float fMinDefence = 0.1f;
				float fMaxDefence = 0.0f;

				fMinDefence *= pMgc->bLevel;
				fMaxDefence = fMinDefence + 0.2f;

				float fActualDefence = fMinDefence;
				int nMC = GetRandomAbility(AT_MC);
				fActualDefence += (float)nMC / 128;
				if(fActualDefence > fMaxDefence)
				{
					fActualDefence = fMaxDefence;
				}
				nDefence = _damage * fActualDefence;
			}
		}
	}
	else if(m_stData.bJob == 2)
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_CHARMAC);
		if(NULL != pMgc)
		{
			if(pMgc->bLevel > 0 &&
				TEST_FLAG(m_dwHumEffFlag, MMASK_CHARMAC))
			{
				nDefence = pMgc->bLevel * 2;
			}
		}
	}
	return nDefence;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::GetSheildDefence(int _damage)
{
	int nDefence = 0;

	if(m_stData.bJob == 1)
	{
		const UserMagic* pMgc = GetUserMagic(MEFF_SHIELD);
		if(NULL != pMgc)
		{
			if(pMgc->bLevel > 0 &&
				TEST_FLAG(m_dwHumEffFlag, MMASK_SHIELD))
			{
				float nSkillLevel = pMgc->bLevel;
				if(nSkillLevel > 3)
				{
					nSkillLevel = 3;
				}

				float fFactor = 0.18f;
				float fMinDefence = fFactor;
				float fMaxDefence = 0.0f;

				fMinDefence *= nSkillLevel;
				fMaxDefence = fMinDefence + fFactor;

				float fActualDefence = fMinDefence;
				int nMC = GetRandomAbility(AT_MC);
				fActualDefence += (float)nMC / 1024;
				if(fActualDefence > fMaxDefence)
				{
					fActualDefence = fMaxDefence;
				}
				nDefence = _damage * fActualDefence + 1;
			}
		}
	}

	return nDefence;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::IsEffectExist(DWORD _dwMgcID)
{
	if(_dwMgcID == MEFF_SHIELD)
	{
		return TEST_FLAG(m_dwHumEffFlag, MMASK_SHIELD) ? true : false;
	}
	else if(_dwMgcID == MEFF_CHARMAC)
	{
		return TEST_FLAG(m_dwHumEffFlag, MMASK_CHARMAC) ? true : false;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::ProcessSheild(int _defence)
{
	m_dwDefence += _defence;

	int nMaxDefence = 0;
	const UserMagic* pMgc = GetUserMagic(MEFF_SHIELD);
	if(pMgc)
	{
		if(pMgc->bLevel > 0 &&
			pMgc->bLevel <= 3)
		{
			nMaxDefence = 500 * pMgc->bLevel;
		}
		else if(pMgc->bLevel > 3)
		{
			nMaxDefence = 500 * 3 + (pMgc->bLevel - 3) * 1000;
		}
	}
	if(m_dwDefence > nMaxDefence)
	{
		//	erase this effect
		RESET_FLAG(m_dwHumEffFlag, MMASK_SHIELD);
		m_dwHumEffTime[MMASK_SHIELD_INDEX] = 0;
		m_dwDefence = 0;

		PkgPlayerSetEffectAck ack;
		ack.uTargetId = GetID();
		ack.dwMgcID = MMASK_SHIELD;
		SendPacket(ack);
	}
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AddBigStoreItem(ItemAttrib* _pItem)
{
	int nIdx = -1;

	if(GETITEMATB(_pItem, Type) == ITEM_NO)
	{
		return false;
	}

	for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
	{
		if(GETITEMATB(&m_stBigStore[i], Type) == ITEM_NO)
		{
			nIdx = i;
			break;
		}
	}

	if(nIdx != -1)
	{
		memcpy(&m_stBigStore[nIdx], _pItem, sizeof(ItemAttrib));
		return true;
	}
	return false;
}

ItemAttrib* HeroObject::GetBigStoreItem(DWORD _dwTag)
{
	for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
	{
		if(m_stBigStore[i].tag == _dwTag &&
			GETITEMATB(&m_stBigStore[i], Type) != ITEM_NO)
		{
			return &m_stBigStore[i];
		}
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::AddStoreItem(ItemAttrib* _pItem)
{
	int nIdx = -1;

	if(GETITEMATB(_pItem, Type) == ITEM_NO)
	{
		return false;
	}

	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		if(GETITEMATB(&m_stStore[i], Type) == ITEM_NO)
		{
			nIdx = i;
			break;
		}
	}

	if(nIdx != -1)
	{
		memcpy(&m_stStore[nIdx], _pItem, sizeof(ItemAttrib));
		return true;
	}
	return false;
}

bool HeroObject::IsStoreItemExist(DWORD _dwTag)
{
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		if(m_stStore[i].tag == _dwTag &&
			GETITEMATB(&m_stStore[i], Type) != ITEM_NO)
		{
			return true;
		}
	}
	return false;
}

ItemAttrib* HeroObject::GetStoreItem(DWORD _dwTag)
{
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		if(m_stStore[i].tag == _dwTag &&
			GETITEMATB(&m_stStore[i], Type) != ITEM_NO)
		{
			return &m_stStore[i];
		}
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
USHORT HeroObject::GetHeroWalkInterval()
{
	return 0;
}

USHORT HeroObject::GetHeroSpellInterval()
{
	DWORD dwInterval = 80 * 6 + 300;

	int nExtraInterval = 300 - 37 * GetObject_AtkSpeed();
#ifdef _DEBUG
	//nExtraInterval = 0;
#endif
	if(nExtraInterval < 0)
	{
		nExtraInterval = 0;
	}
	return dwInterval + nExtraInterval;
}

USHORT HeroObject::GetHeroRunInterval()
{
	return 0;
}

USHORT HeroObject::GetHeroAttackInterval()
{
	/*float fAtkInterval = ((float)MAX_ATTACK_INTERVAL - (float)m_stData.stAttrib.atkSpeed * 5) / 1000;
	if(fAtkInterval < 0.0f)
	{
		fAtkInterval = 0.05f;
	}
	return fAtkInterval * 1000 * 6;*/

	DWORD dwInterval = 80 * 6 + 300;
	int nExtraInterval = 300 - 37 * GetObject_AtkSpeed();
#ifdef _DEBUG
	//nExtraInterval = 0;
#endif
	if(nExtraInterval < 0)
	{
		nExtraInterval = 0;
	}
	return dwInterval + nExtraInterval;
}

//////////////////////////////////////////////////////////////////////////
//bool HeroObject::ReceiveDamage(int _nDmg)
//{
	/*int nAC = 0;
	int nDC = 0;
	bool bMissed = false;

	if(pHero->IsEffectExist(MEFF_SHIELD))
	{
		int nDefence = pHero->GetSheildDefence(_nDmg);
		_nDmg -= nDefence;
		if(nDamage < 0)
		{
			_nDmg = 0;
		}
		pHero->ProcessSheild(nDefence);
	}

	if(IsMagicAttackMode())
	{
		nAC = pHero->GetRandomAbility(AT_MAC);
	}
	else
	{
		nAC = pHero->GetRandomAbility(AT_AC);
		bMissed = IsMissed(pHero);
	}

	nDamage = nDC - nAC;
	if(nDamage > 0 &&
		!bMissed)
	{
		if(!pHero->DecHP(nDamage))
		{
			//	Dead
			not.uTargetId = pHero->GetID();
			not.uAction = ACTION_DEAD;
			not.uParam0 = MAKE_POSITION_DWORD(pHero);
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			pHero->GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

			m_stData.eGameState = OS_DEAD;
			pHero->KillAllSlave();
			pHero->GetLocateScene()->EraseTarget(pHero);
		}
		else
		{
			//	update hp value
			ntf.bType = UPDATE_HP;
			ntf.dwParam = pHero->GetUserData()->stAttrib.HP;
			ntf.uTargetId = pHero->GetID();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ntf;
			pHero->GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		}
	}
	else
	{
		bReqStruck = false;
	}

	if(pHero->GetHeroJob() == 0)
	{
		pMgc = pHero->GetUserMagic(MEFF_MOUNTAIN);
		if(NULL != pMgc)
		{
			if(pMgc->bLevel > 0)
			{
				int nRandom = 0;
				if(pMgc->bLevel == 1)
				{
					nRandom = 20;
				}
				else if(pMgc->bLevel == 2)
				{
					nRandom = 40;
				}
				else if(pMgc->bLevel == 3)
				{
					nRandom = 60;
				}

				if(nRandom > rand() % 100)
				{
					bReqStuck = false;
				}
			}
		}
	}*/
	//return false;
//}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::CanStruck(int _nDamage)
{
	const UserMagic* pMgc = NULL;
	bool bReqStruck = true;

	if(_nDamage <= 0)
	{
		return false;
	}

	int nRandom = 0;

	if(GetHeroJob() == 0)
	{
		pMgc = GetUserMagic(MEFF_MOUNTAIN);
		if(NULL != pMgc)
		{
			if(pMgc->bLevel > 0)
			{
				if(pMgc->bLevel == 1)
				{
					nRandom = 10;
				}
				else if(pMgc->bLevel == 2)
				{
					nRandom = 25;
				}
				else if(pMgc->bLevel == 3)
				{
					nRandom = 40;
				}

				/*if(GETITEMATB(&m_stEquip[PLAYER_ITEM_HELMET], Type) != ITEM_NO)
				{
					if(GETITEMATB(&m_stEquip[PLAYER_ITEM_HELMET], ID) == 344)
					{
						nRandom += 10;
					}
					else if(GETITEMATB(&m_stEquip[PLAYER_ITEM_HELMET], ID) == 104)
					{
						nRandom += 20;
					}
				}*/
			}
		}
	}

	nRandom += m_xStates.GetDefStruckAddition() * 10;

	if(nRandom == 0)
	{
		return true;
	}

	if(nRandom >= 100)
	{
		bReqStruck = false;
	}
	else
	{
		if(nRandom > rand() % 100)
		{
			bReqStruck = false;
		}
	}

	return bReqStruck;
}
//////////////////////////////////////////////////////////////////////////
bool HeroObject::RemoveItem(int _nTag)
{
	ItemAttrib* pItem = GetItemByTag(_nTag);
	if(NULL != pItem)
	{
		ZeroMemory(pItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(pItem);

		PkgPlayerLostItemAck ack;
		ack.uTargetId = GetID();
		ack.dwTag = _nTag;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		SendBuffer(GetUserIndex(), &g_xThreadBuffer);
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
int HeroObject::ItemTagToAttribID(int _nTag)
{
	ItemAttrib* pItem = GetItemByTag(_nTag);
	if(NULL != pItem)
	{
		return GETITEMATB(pItem, ID);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void HeroObject::OnItemDataLoaded(ItemAttrib* _pItem)
{
	//	Update item suit attrib...
	if(GetVersion() == BACKMIR_VERSION200 ||
		GetVersion() == BACKMIR_VERSION201 ||
		GetVersion() == BACKMIR_VERSION202 ||
		GetVersion() == BACKMIR_VERSION203 ||
		GetVersion() == BACKMIR_VERSION204 ||
		GetVersion() == BACKMIR_VERSION205 ||
		GetVersion() == BACKMIR_VERSION206 ||
		GetVersion() == BACKMIR_VERSION207 ||
		GetVersion() == BACKMIR_VERSION208 ||
		GetVersion() == BACKMIR_VERSION209)
	{
		ItemAttrib item;
		if(GetRecordInItemTable(_pItem->id, &item))
		{
			_pItem->atkPalsy = item.atkPalsy;
		}

		if(_pItem->id == 858)
		{
			_pItem->atkSpeed = 1;
		}

		if(_pItem->id == 312)
		{
			_pItem->hide = 4;
		}
		if(_pItem->id == 16)
		{
			_pItem->hide = 4;
		}

		if(_pItem->type == ITEM_BALE)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
		}
		else if(_pItem->type == ITEM_COST &&
			_pItem->atkSpeed == 0)
		{
			_pItem->atkSpeed = 1;
		}

		//	Error item's name
		ZeroMemory(_pItem->name, sizeof(_pItem->name));
		strcpy(_pItem->name, item.name);
	}

	if(GetVersion() == BACKMIR_VERSION200 ||
		GetVersion() == BACKMIR_VERSION201 ||
		GetVersion() == BACKMIR_VERSION202 ||
		GetVersion() == BACKMIR_VERSION203 ||
		GetVersion() == BACKMIR_VERSION204 ||
		GetVersion() == BACKMIR_VERSION205 ||
		GetVersion() == BACKMIR_VERSION206 ||
		GetVersion() == BACKMIR_VERSION207 ||
		GetVersion() == BACKMIR_VERSION208 ||
		GetVersion() == BACKMIR_VERSION209 ||
		GetVersion() == BACKMIR_VERSION)
	{
		//	Delete all cheated items
		if(
			/*_pItem->id == 206 ||*/
			_pItem->id == 171 ||
			_pItem->id == 521)
		{
			ZeroMemory(_pItem, sizeof(ItemAttrib));
		}
	}

	if(GetVersion() <= BACKMIR_VERSION208)
	{
		if(_pItem->id == 85 ||
			_pItem->id == 86 ||
			_pItem->id == 87 ||
			_pItem->id == 88 ||
			_pItem->id == 89 ||
			_pItem->id == 90)
		{
			_pItem->atkPalsy = 0;
		}
	}

	if(GetVersion() == BACKMIR_VERSION209)
	{
		if(_pItem->id == 828 &&
			_pItem->level == 0)
		{
			ItemAttrib atr;
			GetRecordInItemTable(828, &atr);

			_pItem->maxDC = atr.maxDC;
			_pItem->maxMC = atr.maxMC;
			_pItem->maxSC = atr.maxSC;
		}
		else if(_pItem->id == 601 ||
			_pItem->id == 602 ||
			_pItem->id == 607 ||
			_pItem->id == 608)
		{
			_pItem->accuracy = 2;

			if(_pItem->id == 602)
			{
				if(_pItem->maxAC == 45)
				{
					int nTag = _pItem->tag;
					GetRecordInItemTable(602, _pItem);
					_pItem->tag = nTag;
				}
			}
		}
	}

	if (GetVersion() == BACKMIR_VERSION210) {
		// fuzhu type to consume type (11)
		if (_pItem->id >= 784 &&
			_pItem->id <= 790) {
				if (GETITEMATB(_pItem, Type) == ITEM_OTHER) {
					GetRecordInItemTable(_pItem->id, _pItem);
					SETITEMATB(_pItem, AtkSpeed, 1);
				}
		}
	}
}

void HeroObject::ClearAllItem()
{
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		//m_xBag[i].id = INVALID_ITEM_ID;
		//m_xBag[i].type = ITEM_NO;
		//m_xBag[i].tag = 0;
		ZeroMemory(&m_xBag[i], sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(&m_xBag[i]);
	}
	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		//m_stEquip[i].id = 0;
		//m_stEquip[i].type = ITEM_NO;
		//m_stEquip[i].tag = 0;
		ZeroMemory(&m_stEquip[i], sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(&m_stEquip[i]);
	}

	ZeroMemory(m_stStore, sizeof(m_stStore));
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		ObjectValid::EncryptAttrib(&m_stStore[i]);
	}

	ZeroMemory(m_stBigStore, sizeof(m_stBigStore));
	for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
	{
		ObjectValid::EncryptAttrib(&m_stBigStore[i]);
	}

	PkgPlayerClearAllItemAck ack;
	ack.uTargetId = GetID();

	SendPacket(ack);
}

void HeroObject::FlyToHome()
{
	GameScene* pScene = GetHomeScene();
	if(NULL != pScene)
	{
		WORD wPosX = pScene->GetCityCenterX();
		WORD wPosY = pScene->GetCityCenterY();

		if(pScene->GetMapID() != GetLocateScene()->GetMapID())
		{
			//
			FlyToMap(wPosX, wPosY, pScene->GetMapID());
		}
		else
		{
			FlyTo(wPosX, wPosY);
		}
	}
}

void HeroObject::SlavesFlyToMaster()
{
	for (int i = 0; i < MAX_SLAVE_SUM; ++i)
	{
		GameObject* pSlave = m_pSlaves[i];
		if (NULL == pSlave)
		{
			continue;
		}

		if (pSlave->GetType() != SOT_MONSTER)
		{
			continue;
		}
		MonsterObject* pMonster = (MonsterObject*)pSlave;

		if (pSlave->GetUserData()->wMapID != GetUserData()->wMapID)
		{
			int nFlyX = 0;
			int nFlyY = 0;
			for(int i = 0; i < 8; ++i)
			{
				nFlyX = GetUserData()->wCoordX + g_nMoveOft[i * 2];
				nFlyY = GetUserData()->wCoordY + g_nMoveOft[i * 2 + 1];
				if(GetLocateScene()->CanThrough(nFlyX, nFlyY))
				{
					pMonster->FlyToMap(nFlyX, nFlyY, GetUserData()->wMapID);
					break;
				}
			}
		}
	}
}

void HeroObject::FlyToPrison()
{
	return;
	m_pValid->UpdateAllAttrib();

	if(GetMapID() == PRISON_MAP_ID)
	{
		return;
	}

	FlyToMap(12, 10, PRISON_MAP_ID);
}

void HeroObject::GainExpEx(int _nExp)
{
	if(GetTeamID() == 0)
	{
		GainExp(_nExp);
	}
	else
	{
		GameTeam* pTeam = GameTeamManager::GetInstance()->GetTeam(GetTeamID());

		if(pTeam)
		{
			pTeam->AddTeamExpr(this, _nExp);
		}
		else
		{

		}
	}
}

void HeroObject::GainExpExDelay(int _nExp)
{
	
}

void HeroObject::SyncQuestData(int _nStage)
{
	if(_nStage < 0 ||
		_nStage >= MAX_QUEST_NUMBER)
	{
		return;
	}

	PkgPlayerSyncQuestNtf ppsqn;
	ppsqn.uTargetId = GetID();
	ppsqn.bStage = _nStage;
	ppsqn.bCounter = m_xQuest.GetQuestCounter(_nStage);
	ppsqn.bStep = m_xQuest.GetQuestStep(_nStage);
	SendPacket(ppsqn);
}

HeroObject* HeroObject::GetTeamMate(int _idx)
{
	return NULL;
}

bool HeroObject::IsTeamLeader()
{
	if(GetTeamID() != 0)
	{
		GameTeam* pTeam = GameTeamManager::GetInstance()->GetTeam(GetTeamID());
		if(NULL != pTeam)
		{
			if(pTeam->GetTeamLeader() == this)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

void HeroObject::TeamMateFlyToInstanceMap(int _x, int _y, GameInstanceScene* _pInsScene)
{
	if(GetTeamID() != 0)
	{
		GameTeam* pTeam = GameTeamManager::GetInstance()->GetTeam(GetTeamID());
		if(NULL != pTeam)
		{
			TeammatesVector& xAll = pTeam->GetAllPlayer();

			int nSize = xAll.size();

			if(nSize != 0)
			{
				for(int i = 0; i < nSize; ++i)
				{
					if(xAll[i] != NULL &&
						xAll[i] != this)
					{
						xAll[i]->FlyToInstanceMap(_x, _y, _pInsScene);
					}
				}
			}
		}
	}
}

void HeroObject::TeamMateFlyToMap(int _x, int _y, int _nMapID)
{
	if(GetTeamID() != 0)
	{
		GameTeam* pTeam = GameTeamManager::GetInstance()->GetTeam(GetTeamID());
		if(NULL != pTeam)
		{
			TeammatesVector& xAll = pTeam->GetAllPlayer();

			int nSize = xAll.size();

			if(nSize != 0)
			{
				for(int i = 0; i < nSize; ++i)
				{
					if(xAll[i] != NULL &&
						xAll[i] != this)
					{
						HeroObject* pHero = xAll[i];
						pHero->FlyToMap(_x, _y, _nMapID);
					}
				}
			}
		}
	}
}

bool HeroObject::IsTeamMateAround(int _nOffset)
{
	if(GetTeamID() != 0)
	{
		GameTeam* pTeam = GameTeamManager::GetInstance()->GetTeam(GetTeamID());
		if(NULL != pTeam)
		{
			TeammatesVector& xAll = pTeam->GetAllPlayer();

			int nSize = xAll.size();

			if(nSize != 0)
			{
				for(int i = 0; i < nSize; ++i)
				{
					if(xAll[i] != NULL &&
						xAll[i] != this)
					{
						HeroObject* pHero = xAll[i];

						if(pHero->GetMapID() != GetMapID())
						{
							return false;
						}
						
						if(abs(pHero->GetCoordX() - GetCoordX()) >= _nOffset ||
							abs(pHero->GetCoordY() - GetCoordY()) >= _nOffset)
						{
							return false;
						}
					}
				}
			}
		}

		return true;
	}

	return false;
}

void HeroObject::SendHumDataV2(bool _bSendToClient, bool _bSendToLogin)
{
	/*	永远可存档，单机模式下，绑定所有物品，并且无法保存物品不会进入存档数据
		联网模式下，自动存档不会绑定任何物品，也不会保存无法保存的数据，主动存档
		绑定所有物品，不保存无法保存的物品

		战网的数据发到单机不会保存装备的鉴定属性
	*/
	if(CMainServer::GetInstance()->GetServerMode() == GM_NORMAL)
	{
		//	绑定所有物品
		for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
		{
			ObjectValid::DecryptAttrib(&m_xBag[i]);
			if(&m_xBag[i].type != ITEM_NO)
			{
				//m_xBag[i].atkPois = 1;
				SET_FLAG(m_xBag[i].atkPois, POIS_MASK_BIND);
			}
			ObjectValid::EncryptAttrib(&m_xBag[i]);
		}

		//	on body
		for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
		{
			ObjectValid::DecryptAttrib(&m_stEquip[i]);
			if(m_stEquip[i].type != ITEM_NO)
			{
				SET_FLAG(m_stEquip[i].atkPois, POIS_MASK_BIND);
			}
			ObjectValid::EncryptAttrib(&m_stEquip[i]);
		}

		//	仓库
		for(int i = 0; i < MAX_STORE_NUMBER; ++i)
		{
			ObjectValid::DecryptAttrib(&m_stStore[i]);
			if(m_stStore[i].type != ITEM_NO)
			{
				SET_FLAG(m_stStore[i].atkPois, POIS_MASK_BIND);
			}
			ObjectValid::EncryptAttrib(&m_stStore[i]);
		}

		//	大仓库
		for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
		{
			ObjectValid::DecryptAttrib(&m_stBigStore[i]);
			if(m_stBigStore[i].type != ITEM_NO)
			{
				SET_FLAG(m_stBigStore[i].atkPois, POIS_MASK_BIND);
			}
			ObjectValid::EncryptAttrib(&m_stBigStore[i]);
		}
	}

	//	发送给单机
	bool bSingleSaveOk = false;
	if(_bSendToClient)
	{
		bool bHideIdentify = false;
		if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
		{
			bHideIdentify = true;
		}

		PkgSystemUserDataAck ack;
		if(!WriteHumDataToBuffer(ack.xData, bHideIdentify))
		{
			PkgSystemUserDataAck nack;
			SendPacket(nack);
			//return;
		}
		else
		{
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ack;
			SendPlayerBuffer(g_xThreadBuffer);
			bSingleSaveOk = true;
		}

		//	扩展信息
		if (IsNewPlayer() ||
			(!IsNewPlayer() && m_bBigStoreReceived))
		{
			//	1.新玩家，则发送仓库数据 初始化存档
			//	2.老玩家 则收到了仓库数据 才会发送仓库数据进行存档
			PkgSystemExtUserDataAck extack;
			extack.nExtIndex = 0;
			if(WriteHumBigStoreDataToBuffer(extack.xData, bHideIdentify))
			{
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << extack;
				SendPlayerBuffer(g_xThreadBuffer);
			}
		}
	}

	//	发给登陆服务器
	if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN &&
		_bSendToLogin)
	{
		std::vector<char> xHumData;
		if(!WriteHumDataToBuffer(xHumData, false, true))
		{
			LOG(ERROR) << "failed to get hum binary data and send to login server.";
			return;
		}

		// get player name
		char szName[20];
		ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
		DWORD dwLSIndex = CMainServer::GetInstance()->GetLSConnIndex();

		if (GetProtoType() == ProtoType_ByteBuffer)
		{
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << (int)0;
			g_xThreadBuffer << (int)PKG_LOGIN_ROLEDATAFROMGS_NOT;
			g_xThreadBuffer << GetLSIndex();
			g_xThreadBuffer << GetUID();
			g_xThreadBuffer << (char)strlen(szName);
			g_xThreadBuffer.Write(szName, strlen(szName));
			g_xThreadBuffer << (short)GetObject_Level();
			g_xThreadBuffer << xHumData;
			DWORD dwLSIndex = CMainServer::GetInstance()->GetLSConnIndex();
			g_xConsole.CPrint("LS Index: %d User LS Index: %d", dwLSIndex, GetLSIndex());
			SendBufferToServer(dwLSIndex, &g_xThreadBuffer);
		}
		else
		{
			// using protobuf
			protocol::MSavePlayerDataReq req;
			req.set_lid(GetLSIndex());
			req.set_uid(GetUID());
			req.set_name(GetName());
			req.set_level(GetObject_Level());
			req.set_serverid(0);
			req.set_data(&xHumData[0], xHumData.size());

			g_xConsole.CPrint("LS Index: %d User LS Index: %d , send hum data", dwLSIndex, GetLSIndex());
			SendProtoToServer(dwLSIndex, protocol::SavePlayerDataReq, req);
		}

		xHumData.clear();

		//	发送大仓库数据
		if (IsNewPlayer() ||
			(!IsNewPlayer() && m_bBigStoreReceived))
		{
			if(!WriteHumBigStoreDataToBuffer(xHumData, false, true))
			{
				LOG(ERROR) << "failed to get hum binary extend data and send to login server.";
				return;
			}

			if (GetProtoType() == ProtoType_ByteBuffer)
			{
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << (int)0;
				g_xThreadBuffer << (int)PKG_LOGIN_EXTROLEDATAFROMGS_NOT;
				g_xThreadBuffer << GetLSIndex();
				g_xThreadBuffer << GetUID();
				g_xThreadBuffer << (char)strlen(szName);
				g_xThreadBuffer.Write(szName, strlen(szName));
				g_xThreadBuffer << (short)0;
				g_xThreadBuffer << xHumData;
				SendBufferToServer(dwLSIndex, &g_xThreadBuffer);
			}
			else
			{
				protocol::MSavePlayerExtDataReq req;
				req.set_lid(GetLSIndex());
				req.set_uid(GetUID());
				req.set_name(GetName());
				req.set_extindex(0);
				req.set_serverid(0);
				req.set_data(&xHumData[0], xHumData.size());

				g_xConsole.CPrint("LS Index: %d User LS Index: %d , send hum ext data", dwLSIndex, GetLSIndex());
				SendProtoToServer(dwLSIndex, protocol::SavePlayerExtDataReq, req);
			}
		}
	}

#ifdef _DEBUG
	LOG(INFO) << GetName() << " Request user data" << (bSingleSaveOk ? " fail" : " success");
#endif
}

/*void HeroObject::SendHumData(bool _bSendToClient, bool _bSendToLogin)
{
	//	can save??
	bool bCanSave = true;

#ifdef _DEBUG
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(GETITEMATB(&m_xBag[i], Type) != ITEM_NO)
		{
			if(TEST_FLAG_BOOL(GETITEMATB(&m_xBag[i], Expr), EXPR_MASK_NOSAVE))
			{
				if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
				{
					ZeroMemory(&m_xBag[i], sizeof(ItemAttrib));
					ObjectValid::EncryptAttrib(&m_xBag[i]);
				}
				bCanSave = false;
			}
		}
	}
	if(bCanSave)
	{
		for(int i = 0; i < MAX_STORE_NUMBER; ++i)
		{
			if(GETITEMATB(&m_stStore[i], Type) != ITEM_NO)
			{
				if(TEST_FLAG_BOOL(GETITEMATB(&m_stStore[i], Expr), EXPR_MASK_NOSAVE))
				{
					if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
					{
						ZeroMemory(&m_stStore[i], sizeof(ItemAttrib));
						ObjectValid::EncryptAttrib(&m_stStore[i]);
					}
					bCanSave = false;
				}
			}
		}
	}
#else
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(GETITEMATB(&m_xBag[i], Type) != ITEM_NO)
		{
			if(TEST_FLAG_BOOL(GETITEMATB(&m_xBag[i], Expr), EXPR_MASK_NOSAVE))
			{
				if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
				{
					ZeroMemory(&m_xBag[i], sizeof(ItemAttrib));
					ObjectValid::EncryptAttrib(&m_xBag[i]);
				}
				bCanSave = false;
				if(!bCanSave)
				{
					if(CMainServer::GetInstance()->GetServerMode() == GM_NORMAL)
					{
						break;
					}
				}
			}
		}
	}
	if(bCanSave)
	{
		for(int i = 0; i < MAX_STORE_NUMBER; ++i)
		{
			if(GETITEMATB(&m_stStore[i], Type) != ITEM_NO)
			{
				if(TEST_FLAG_BOOL(GETITEMATB(&m_stStore[i], Expr), EXPR_MASK_NOSAVE))
				{
					if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
					{
						ZeroMemory(&m_stStore[i], sizeof(ItemAttrib));
						ObjectValid::EncryptAttrib(&m_stStore[i]);
					}
					bCanSave = false;
					if(!bCanSave)
					{
						if(CMainServer::GetInstance()->GetServerMode() == GM_NORMAL)
						{
							break;
						}
					}
				}
			}
		}
	}
#endif
	if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
	{
		bCanSave = true;
	}
	if(!bCanSave)
	{
		PkgSystemUserDataAck ack;
		SendPacket(ack);
		return;
	}
	//	request the player's data and save to local save file
#define MAX_DATA_SIZE 20480
	//ByteBuffer xBuf(MAX_DATA_SIZE);
	//xBuf.Reset();
	g_xThreadBuffer.Reset();
	//	First is version
	//g_xThreadBuffer << (USHORT)BACKMIR_VERSION111;
	//g_xThreadBuffer << (USHORT)BACKMIR_VERSION112;
	g_xThreadBuffer << (USHORT)BACKMIR_VERSION;
	//	level
	g_xThreadBuffer << (USHORT)GetObject_Level();
	//	Job
	g_xThreadBuffer << (BYTE)m_stData.bJob;
	//	Name
	//g_xThreadBuffer << m_stData.stAttrib.name;
	//	Sex
	//g_xThreadBuffer << m_stData.stAttrib.sex;
	//	Current map?
	g_xThreadBuffer << (USHORT)m_stData.wMapID;
	//	Last city map
	g_xThreadBuffer << (WORD)m_dwLastCityMap;
	//	Current pos?
	g_xThreadBuffer << (DWORD)(MAKELONG(m_stData.wCoordX, m_stData.wCoordY));
	//	HP MP
	g_xThreadBuffer << (DWORD)MAKELONG(GetObject_HP(), GetObject_MP());
	//	EXPR
	g_xThreadBuffer << (DWORD)GetObject_Expr();
	//	quest?
	g_xThreadBuffer << m_xQuest;
	//	money
	g_xThreadBuffer << (DWORD)GetMoney();
	//	bag items
	int nCounter = 0;

	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		ObjectValid::DecryptAttrib(&m_xBag[i]);
		if(GETITEMATB(&m_xBag[i], Type) != ITEM_NO)
		{
			//m_xBag[i].atkPois = 1;
			SET_FLAG(m_xBag[i].atkPois, POIS_MASK_BIND);
			++nCounter;
		}
	}
	g_xThreadBuffer << (BYTE)nCounter;
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(m_xBag[i].type != ITEM_NO)
		{
			g_xThreadBuffer << m_xBag[i];
		}
		ObjectValid::EncryptAttrib(&m_xBag[i]);
	}
	//	on body
	nCounter = 0;
	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		ObjectValid::DecryptAttrib(&m_stEquip[i]);
		if(GETITEMATB(&m_stEquip[i], Type) != ITEM_NO)
		{
			++nCounter;
			//m_stEquip[i].atkPois = 1;
			SET_FLAG(m_stEquip[i].atkPois, POIS_MASK_BIND);
		}
	}
	g_xThreadBuffer << (BYTE)nCounter;
	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		if(m_stEquip[i].type != ITEM_NO)
		{
			g_xThreadBuffer << (BYTE)i;
			g_xThreadBuffer << m_stEquip[(PLAYER_ITEM_TYPE)i];
		}
		ObjectValid::EncryptAttrib(&m_stEquip[(PLAYER_ITEM_TYPE)i]);
	}
	//	Magic
	nCounter = 0;
	for(int i = 0; i < USER_MAGIC_NUM; ++i)
	{
		if(m_xMagics[i].bLevel != 0)
		{
			++nCounter;
		}
	}
	g_xThreadBuffer << (BYTE)nCounter;
	for(int i = 0; i < USER_MAGIC_NUM; ++i)
	{
		if(m_xMagics[i].bLevel != 0)
		{
			if(m_stData.bJob == 0)
			{
				g_xThreadBuffer << (WORD)(i + MEFF_DC_BEGIN + 1) << (BYTE)m_xMagics[i].bLevel;
			}
			else if(m_stData.bJob == 1)
			{
				g_xThreadBuffer << (WORD)(i + MEFF_MC_BEGIN + 1) << (BYTE)m_xMagics[i].bLevel;
			}
			else if(m_stData.bJob == 2)
			{
				g_xThreadBuffer << (WORD)(i + MEFF_SC_BEGIN + 1) << (BYTE)m_xMagics[i].bLevel;
			}
		}
	}
	//	Storage
	nCounter = 0;
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		ObjectValid::DecryptAttrib(&m_stStore[i]);
		if(GETITEMATB(&m_stStore[i], Type) != ITEM_NO)
		{
			++nCounter;
			//m_stStore[i].atkPois = 1;
			SET_FLAG(m_stStore[i].atkPois, POIS_MASK_BIND);
		}
	}
	g_xThreadBuffer << (BYTE)nCounter;
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		if(m_stStore[i].type != ITEM_NO)
		{
			g_xThreadBuffer << m_stStore[i];
		}
		ObjectValid::EncryptAttrib(&m_stStore[i]);
	}

	//	117	Add	reserve
	g_xThreadBuffer << (DWORD)0;

	//	220 Add reserver
	g_xThreadBuffer << (DWORD)0;
	
	//	Now compress it
	static char* s_pData = new char[MAX_DATA_SIZE];
	uLongf cmpsize = MAX_DATA_SIZE;
	uLongf srcsize = g_xThreadBuffer.GetLength();

	int nRet = compress((Bytef*)s_pData, &cmpsize, (const Bytef*)g_xThreadBuffer.GetBuffer(), srcsize);

	PkgSystemUserDataAck ack;
	if(nRet == Z_OK)
	{
		s_pData[cmpsize] = 0;
		//ack.xData = pData;
		ack.xData.clear();
		ack.xData.resize(cmpsize);

		memcpy((char*)(&ack.xData[0]), s_pData, cmpsize);
	}
	g_xThreadBuffer.Reset();
	if(_bSendToClient)
	{
		g_xThreadBuffer << ack;
		SendPlayerBuffer(g_xThreadBuffer);
	}

	//	发给登陆服务器
	if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN &&
		_bSendToLogin)
	{
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << (int)0;
		g_xThreadBuffer << (int)PKG_LOGIN_ROLEDATAFROMGS_NOT;
		g_xThreadBuffer << GetLSIndex();
		g_xThreadBuffer << GetUID();
		char szName[20];
		ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
		g_xThreadBuffer << (char)strlen(szName);
		g_xThreadBuffer.Write(szName, strlen(szName));
		g_xThreadBuffer << (short)GetObject_Level();
		g_xThreadBuffer << ack.xData;
		DWORD dwLSIndex = CMainServer::GetInstance()->GetLSConnIndex();
		g_xConsole.CPrint("LS Index: %d User LS Index: %d", dwLSIndex, GetLSIndex());
		SendBufferToServer(dwLSIndex, &g_xThreadBuffer);
	}

#ifdef _DEBUG
	char szName[20];
	ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
	LOG(INFO) << szName << " Request user data" << (ack.xData.empty() ? " fail" : " success");
#endif

	//delete[] pData;
}*/

//////////////////////////////////////////////////////////////////////////
void HeroObject::GetStatusInfo(PkgPlayerGStatusNtf& ntf)
{
	PkgPlayerGStatusNtf& statusNtf = ntf;

	statusNtf.uTargetId = GetID();

	DWORD dwCurTick = GetTickCount();

	//	charm ac
	if(TEST_FLAG(m_dwHumEffFlag, MMASK_CHARMAC))
	{
		if(m_dwHumEffTime[MMASK_CHARMAC_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_CHARMAC);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_CHARMAC_INDEX] - dwCurTick);
		}
	}

	//	poison
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_LVDU))
	{
		if(m_dwHumEffTime[MMASK_LVDU_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_POISON);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_LVDU_INDEX] - dwCurTick);
		}
	}

	//	shield
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_SHIELD))
	{
		if(m_dwHumEffTime[MMASK_SHIELD_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_SHIELD);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_SHIELD_INDEX] - dwCurTick);
		}
	}

	//	reset energy shield
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_ENERGYSHIELD))
	{
		if(m_dwHumEffTime[MMASK_ENERGYSHIELD_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_ENERGYSHIELD);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_ENERGYSHIELD_INDEX] - dwCurTick);
		}
	}

	//	double drop
	if(m_dwDoubleDropExpireTime != 0)
	{
		if(m_dwDoubleDropExpireTime > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_DOUBLEDROP);
			statusNtf.xTimes.push_back(m_dwDoubleDropExpireTime - dwCurTick);
		}
	}

	//	jingang
	if(m_dwJinGangExpireTime != 0)
	{
		if(m_dwJinGangExpireTime > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_JINGANG);
			statusNtf.xTimes.push_back(m_dwJinGangExpireTime - dwCurTick);
		}
	}

	//	extra suit add
	if (0 != m_nExtraSuitType)
	{
		statusNtf.xStatus.push_back(GSTATUS_SUITSAMELEVEL);
		statusNtf.xTimes.push_back(MAKELONG(m_nExtraSuitType, 0xffff));
	}
}

void HeroObject::SendStatusInfo()
{
	//	status
	PkgPlayerGStatusNtf statusNtf;
	statusNtf.uTargetId = GetID();
	GetStatusInfo(statusNtf);
	DWORD dwCurTick = GetTickCount();

	/*
	//	charm ac
	if(TEST_FLAG(m_dwHumEffFlag, MMASK_CHARMAC))
	{
		if(m_dwHumEffTime[MMASK_CHARMAC_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_CHARMAC);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_CHARMAC_INDEX] - dwCurTick);
		}
	}

	//	poison
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_LVDU))
	{
		if(m_dwHumEffTime[MMASK_LVDU_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_POISON);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_LVDU_INDEX] - dwCurTick);
		}
	}

	//	shield
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_SHIELD))
	{
		if(m_dwHumEffTime[MMASK_SHIELD_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_SHIELD);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_SHIELD_INDEX] - dwCurTick);
		}
	}

	//	reset energy shield
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_ENERGYSHIELD))
	{
		if(m_dwHumEffTime[MMASK_ENERGYSHIELD_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_ENERGYSHIELD);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_ENERGYSHIELD_INDEX] - dwCurTick);
		}
	}

	//	double drop
	if(m_dwDoubleDropExpireTime != 0)
	{
		if(m_dwDoubleDropExpireTime > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_DOUBLEDROP);
			statusNtf.xTimes.push_back(m_dwDoubleDropExpireTime - dwCurTick);
		}
	}

	//	jingang
	if(m_dwJinGangExpireTime != 0)
	{
		if(m_dwJinGangExpireTime > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_JINGANG);
			statusNtf.xTimes.push_back(m_dwJinGangExpireTime - dwCurTick);
		}
	}
	*/

	SendPacket(statusNtf);
}

//////////////////////////////////////////////////////////////////////////
void HeroObject::BroadcastStatusInfo()
{
	//	status
	PkgPlayerGStatusNtf statusNtf;
	statusNtf.uTargetId = GetID();
	GetStatusInfo(statusNtf);

	/*DWORD dwCurTick = GetTickCount();

	//	charm ac
	if(TEST_FLAG(m_dwHumEffFlag, MMASK_CHARMAC))
	{
		if(m_dwHumEffTime[MMASK_CHARMAC_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_CHARMAC);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_CHARMAC_INDEX] - dwCurTick);
		}
	}

	//	poison
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_LVDU))
	{
		if(m_dwHumEffTime[MMASK_LVDU_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_POISON);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_LVDU_INDEX] - dwCurTick);
		}
	}

	//	shield
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_SHIELD))
	{
		if(m_dwHumEffTime[MMASK_SHIELD_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_SHIELD);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_SHIELD_INDEX] - dwCurTick);
		}
	}

	//	reset energy shield
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_ENERGYSHIELD))
	{
		if(m_dwHumEffTime[MMASK_ENERGYSHIELD_INDEX] > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_ENERGYSHIELD);
			statusNtf.xTimes.push_back(m_dwHumEffTime[MMASK_ENERGYSHIELD_INDEX] - dwCurTick);
		}
	}

	//	double drop
	if(m_dwDoubleDropExpireTime != 0)
	{
		if(m_dwDoubleDropExpireTime > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_DOUBLEDROP);
			statusNtf.xTimes.push_back(m_dwDoubleDropExpireTime - dwCurTick);
		}
	}

	//	jingang
	if(m_dwJinGangExpireTime != 0)
	{
		if(m_dwJinGangExpireTime > dwCurTick)
		{
			statusNtf.xStatus.push_back(GSTATUS_JINGANG);
			statusNtf.xTimes.push_back(m_dwJinGangExpireTime - dwCurTick);
		}
	}*/

	g_xThreadBuffer.Reset();
	g_xThreadBuffer << statusNtf;
	GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
}

void HeroObject::SendQuickMessage(int _nMsgCode, int _nParam /* = 0 */)
{
	PkgPlayerQuickMsgNtf ntf;
	ntf.uTargetId = GetID();
	ntf.nMsgID = _nMsgCode;
	ntf.nParam = _nParam;
	SendPacket(ntf);
}

void HeroObject::IncWeaponGrow()
{
	ItemAttrib* pItem = GetEquip(PLAYER_ITEM_WEAPON);

	if(NULL != pItem &&
		ITEM_NO != GETITEMATB(pItem, Type))
	{
		int nPreHP = GETITEMATB(pItem, HP);
#ifdef _DEBUG
		DWORD dwData = GetIncGrowDWORD((DWORD)nPreHP, 300);
#else
		DWORD dwData = GetIncGrowDWORD((DWORD)nPreHP, 1);
#endif
		SETITEMATB(pItem, HP, dwData);

		int nPreLevel = GetGrowLevelFromDWORD(nPreHP);
		int nNowLevel = GetGrowLevelFromDWORD(dwData);

		if(nPreLevel != nNowLevel)
		{
			RefleshAttrib();

			PkgPlayerUpdateWeaponGrowNtf ntf;
			ntf.uTargetId = GetID();
			ntf.dwTag = pItem->tag;
			ntf.dwData = dwData;
			SendPacket(ntf);
		}
	}
}

void HeroObject::IncWeaponGrowWithExp(DWORD _dwExp)
{
	ItemAttrib* pItem = GetEquip(PLAYER_ITEM_WEAPON);

	if(NULL != pItem &&
		ITEM_NO != GETITEMATB(pItem, Type))
	{
		int nPreHP = GETITEMATB(pItem, HP);

		int nGroupNumber = GetIncNumberFromExpr(_dwExp);
#ifdef _DEBUG
		DWORD dwData = GetIncGrowDWORD((DWORD)nPreHP, nGroupNumber * 100);
#else
		DWORD dwData = GetIncGrowDWORD((DWORD)nPreHP, nGroupNumber);
#endif
		SETITEMATB(pItem, HP, dwData);

		int nPreLevel = GetGrowLevelFromDWORD(nPreHP);
		int nNowLevel = GetGrowLevelFromDWORD(dwData);

		if(nPreLevel != nNowLevel)
		{
			RefleshAttrib();

			PkgPlayerUpdateWeaponGrowNtf ntf;
			ntf.uTargetId = GetID();
			ntf.dwTag = pItem->tag;
			ntf.dwData = dwData;
			SendPacket(ntf);
		}
	}
}

int HeroObject::GetChallengeItemID()
{
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(GETITEMATB(&m_xBag[i], Type) != ITEM_NO)
		{
			ItemAttrib* pItem = &m_xBag[i];

			if(GETITEMATB(pItem, Type) == ITEM_COST)
			{
				if(GETITEMATB(pItem, Curse) == 7)
				{
					return GETITEMATB(pItem, Lucky);
				}
			}
		}
	}

	return 0;
}

bool HeroObject::UseChallengeItem(int _nClgID)
{
	bool bUsed = false;

	ItemAttrib* pClgItem = NULL;

	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(GETITEMATB(&m_xBag[i], Type) != ITEM_NO)
		{
			ItemAttrib* pItem = &m_xBag[i];

			if(GETITEMATB(pItem, Type) == ITEM_COST)
			{
				if(GETITEMATB(pItem, Curse) == 7)
				{
					if(GETITEMATB(pItem, Lucky) == _nClgID)
					{
						pClgItem = pItem;
						break;
					}
				}
			}
		}
	}

	if(NULL == pClgItem)
	{
		return false;
	}

	if(GETITEMATB(pClgItem, AtkSpeed) == 0)
	{
		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = pClgItem->tag;
		ntf.nNumber = 0;
		SendPacket(ntf);

		ZeroMemory(pClgItem, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(pClgItem);

		return false;
	}

	bUsed = true;

	if(bUsed)
	{
		int nLeftSum = GETITEMATB(pClgItem, AtkSpeed);
		--nLeftSum;

		PkgPlayerUpdateCostNtf ntf;
		ntf.uTargetId = GetID();
		ntf.dwTag = pClgItem->tag;
		ntf.nNumber = nLeftSum;
		SendPacket(ntf);

		if(0 == nLeftSum)
		{
			ZeroMemory(pClgItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(pClgItem);
		}
		else
		{
			SETITEMATB(pClgItem, AtkSpeed, nLeftSum);
		}
	}

	return bUsed;
}

bool HeroObject::IsGiftIDExist(int _nGiftID)
{
	size_t uSize = m_xGiftItemIDs.size();

	for(size_t i = 0; i < uSize; ++i)
	{
		if(m_xGiftItemIDs[i] == _nGiftID)
		{
			return true;
		}
	}

	return false;
}

int HeroObject::GetBagStoreBodyItemCount(int _nAttribID)
{
	int nCount = 0;

	//	先检查身上的
	for(int i = PLAYER_ITEM_WEAPON; i < PLAYER_ITEM_TOTAL; ++i)
	{
		ItemAttrib* pItem = GetEquip(PLAYER_ITEM_TYPE(i));
		int nAttribID = GETITEMATB(pItem, ID);

		if(nAttribID == _nAttribID)
		{
			++nCount;
		}
	}

	//	检查背包
	for(int i = 0; i < m_xBag.size(); ++i)
	{
		ItemAttrib* pItem = &m_xBag[i];
		int nAttribID = GETITEMATB(pItem, ID);

		if(nAttribID == _nAttribID)
		{
			++nCount;
		}
	}

	//	检查仓库
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		ItemAttrib* pItem = &m_stStore[i];
		int nAttribID = GETITEMATB(pItem, ID);

		if(nAttribID == _nAttribID)
		{
			++nCount;
		}
	}

	//	检查大仓库
	for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
	{
		ItemAttrib* pItem = &m_stBigStore[i];
		int nAttribID = GETITEMATB(pItem, ID);

		if(nAttribID == _nAttribID)
		{
			++nCount;
		}
	}

	return nCount;
}

bool HeroObject::WriteHumBigStoreDataToBuffer(std::vector<char>& _refCharVector, bool _bHideIdentify /* = false */, bool _bSaveCannotSaveItem /* = false */)
{
#define MAX_DATA_SIZE 20480
	//	big Storage
	int nCounter = 0;
	ItemAttrib item = {0};
	g_xThreadBuffer.Reset();

	//	write ext index
	g_xThreadBuffer << (char)0;

	//	write name
	std::string xName = GetName();
	g_xThreadBuffer << xName;

	for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
	{
		item = m_stBigStore[i];
		ObjectValid::DecryptAttrib(&item);

		if(item.type != ITEM_NO)
		{
			if(!TEST_FLAG_BOOL(item.EXPR, EXPR_MASK_NOSAVE) || _bSaveCannotSaveItem)
			{
				++nCounter;
			}
		}
	}
	g_xThreadBuffer << (BYTE)nCounter;
	for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
	{
		item = m_stBigStore[i];
		ObjectValid::DecryptAttrib(&item);

		if(item.type != ITEM_NO)
		{
			if(!TEST_FLAG_BOOL(item.EXPR, EXPR_MASK_NOSAVE) || _bSaveCannotSaveItem)
			{
				if(_bHideIdentify &&
					IsEquipItem(item.type))
				{
					item.maxMP = 0;
				}
				SET_FLAG(item.atkPois, POIS_MASK_BIND);
				g_xThreadBuffer << item;
			}
		}
	}

	static char* s_pData = new char[MAX_DATA_SIZE];
	GlobalAllocRecord::GetInstance()->RecordArray(s_pData);
	uLongf cmpsize = MAX_DATA_SIZE;
	uLongf srcsize = g_xThreadBuffer.GetLength();
	bool bRet = false;

	int nRet = compress((Bytef*)s_pData, &cmpsize, (const Bytef*)g_xThreadBuffer.GetBuffer(), srcsize);

	if(nRet == Z_OK)
	{
		bRet = true;
		s_pData[cmpsize] = 0;
		_refCharVector.clear();
		_refCharVector.resize(cmpsize);

		char* pDest = &_refCharVector[0];
		memcpy(pDest, s_pData, cmpsize);
	}

	return bRet;
}

bool HeroObject::WriteHumDataToBuffer(std::vector<char>& _refCharVector, bool _bHideIdentify /* = false */, bool _bSaveCannotSaveItem /* = false */)
{
	//	request the player's data and save to local save file
#define MAX_DATA_SIZE 20480
	ItemAttrib item;
	ZeroMemory(&item, sizeof(ItemAttrib));
	//ByteBuffer xBuf(MAX_DATA_SIZE);
	//xBuf.Reset();
	g_xThreadBuffer.Reset();
	//	First is version
	g_xThreadBuffer << (USHORT)BACKMIR_VERSION;
	//	level
	g_xThreadBuffer << (USHORT)GetObject_Level();
	//	Job
	g_xThreadBuffer << (BYTE)m_stData.bJob;
	//	Name
	//g_xThreadBuffer << m_stData.stAttrib.name;
	//	Sex
	//g_xThreadBuffer << m_stData.stAttrib.sex;
	//	Current map?
	g_xThreadBuffer << (USHORT)m_stData.wMapID;
	//	Last city map
	g_xThreadBuffer << (WORD)m_dwLastCityMap;
	//	Current pos?
	g_xThreadBuffer << (DWORD)(MAKELONG(m_stData.wCoordX, m_stData.wCoordY));
	//	HP MP
	g_xThreadBuffer << (DWORD)MAKELONG(GetObject_HP(), GetObject_MP());
	//	EXPR
	g_xThreadBuffer << (DWORD)GetObject_Expr();
	//	quest?
	g_xThreadBuffer << m_xQuest;
	//	money
	g_xThreadBuffer << (DWORD)GetMoney();
	//	bag items
	int nCounter = 0;

	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		item = m_xBag[i];
		ObjectValid::DecryptAttrib(&item);

		if(item.type != ITEM_NO)
		{
			if(!TEST_FLAG_BOOL(item.EXPR, EXPR_MASK_NOSAVE) || _bSaveCannotSaveItem)
			{
				++nCounter;
			}
		}
	}
	g_xThreadBuffer << (BYTE)nCounter;
	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		item = m_xBag[i];
		ObjectValid::DecryptAttrib(&item);

		if(item.type != ITEM_NO)
		{
			if(!TEST_FLAG_BOOL(item.EXPR, EXPR_MASK_NOSAVE) || _bSaveCannotSaveItem)
			{
				if(_bHideIdentify &&
					IsEquipItem(item.type))
				{
					item.maxMP = 0;
				}
				SET_FLAG(item.atkPois, POIS_MASK_BIND);
				g_xThreadBuffer << item;
			}
		}
	}
	//	on body
	nCounter = 0;
	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		item = m_stEquip[i];
		ObjectValid::DecryptAttrib(&item);

		if(item.type != ITEM_NO)
		{
			if(!TEST_FLAG_BOOL(item.EXPR, EXPR_MASK_NOSAVE) || _bSaveCannotSaveItem)
			{
				++nCounter;
			}
		}
	}
	g_xThreadBuffer << (BYTE)nCounter;
	for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
	{
		item = m_stEquip[i];
		ObjectValid::DecryptAttrib(&item);

		if(item.type != ITEM_NO)
		{
			if(!TEST_FLAG_BOOL(item.EXPR, EXPR_MASK_NOSAVE) || _bSaveCannotSaveItem)
			{
				g_xThreadBuffer << (BYTE)i;

				if(_bHideIdentify &&
					IsEquipItem(item.type))
				{
					item.maxMP = 0;
				}
				SET_FLAG(item.atkPois, POIS_MASK_BIND);

				g_xThreadBuffer << item;
			}
		}
	}
	//	Magic
	nCounter = 0;
	for(int i = 0; i < USER_MAGIC_NUM; ++i)
	{
		if(m_xMagics[i].bLevel != 0)
		{
			++nCounter;
		}
	}
	g_xThreadBuffer << (BYTE)nCounter;
	for(int i = 0; i < USER_MAGIC_NUM; ++i)
	{
		if(m_xMagics[i].bLevel != 0)
		{
			if(m_stData.bJob == 0)
			{
				g_xThreadBuffer << (WORD)(i + MEFF_DC_BEGIN + 1) << (BYTE)m_xMagics[i].bLevel;
			}
			else if(m_stData.bJob == 1)
			{
				g_xThreadBuffer << (WORD)(i + MEFF_MC_BEGIN + 1) << (BYTE)m_xMagics[i].bLevel;
			}
			else if(m_stData.bJob == 2)
			{
				g_xThreadBuffer << (WORD)(i + MEFF_SC_BEGIN + 1) << (BYTE)m_xMagics[i].bLevel;
			}
		}
	}
	//	Storage
	nCounter = 0;
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		item = m_stStore[i];
		ObjectValid::DecryptAttrib(&item);

		if(item.type != ITEM_NO)
		{
			if(!TEST_FLAG_BOOL(item.EXPR, EXPR_MASK_NOSAVE) || _bSaveCannotSaveItem)
			{
				++nCounter;
			}
		}
	}
	g_xThreadBuffer << (BYTE)nCounter;
	for(int i = 0; i < MAX_STORE_NUMBER; ++i)
	{
		item = m_stStore[i];
		ObjectValid::DecryptAttrib(&item);

		if(item.type != ITEM_NO)
		{
			if(!TEST_FLAG_BOOL(item.EXPR, EXPR_MASK_NOSAVE) || _bSaveCannotSaveItem)
			{
				if(_bHideIdentify &&
					IsEquipItem(item.type))
				{
					item.maxMP = 0;
				}
				SET_FLAG(item.atkPois, POIS_MASK_BIND);
				g_xThreadBuffer << item;
			}
		}
	}

	//	117	Add	reserve
	g_xThreadBuffer << (DWORD)0;

	//	220 Add reserver
	g_xThreadBuffer << (DWORD)0;

	//	2.10.00 save extend bytes
	{
		//	write save extend bytes
		WriteSaveExtendField(g_xThreadBuffer);
	}

	//	Now compress it
	static char* s_pData = new char[MAX_DATA_SIZE];
	GlobalAllocRecord::GetInstance()->RecordArray(s_pData);
	uLongf cmpsize = MAX_DATA_SIZE;
	uLongf srcsize = g_xThreadBuffer.GetLength();
	bool bRet = false;

	int nRet = compress((Bytef*)s_pData, &cmpsize, (const Bytef*)g_xThreadBuffer.GetBuffer(), srcsize);

	if(nRet == Z_OK)
	{
		bRet = true;
		s_pData[cmpsize] = 0;
		//ack.xData = pData;
		_refCharVector.clear();
		_refCharVector.resize(cmpsize);
		/*
		for(int i = 0; i < cmpsize; ++i)
		{
			_refCharVector[i] = pData[i];
		}*/
		char* pDest = &_refCharVector[0];
		memcpy(pDest, s_pData, cmpsize);
	}

	//delete[] pData;
	return bRet;
}

void HeroObject::SyncItemAttrib(int _nTag)
{
	ItemAttrib* pItem = GetItemByTag(_nTag);
	if(NULL == pItem)
	{
		return;
	}

	PkgPlayerUpdateItemNtf ppuin;
	ppuin.uTargetId = GetID();
	ppuin.stItem = *pItem;
	ObjectValid::DecryptAttrib(&ppuin.stItem);
	SendPacket(ppuin);
}

void HeroObject::SyncItemAttribHideIdentAttr(int _nTag)
{
	
}

bool HeroObject::ReceiveGift(int _nGiftID)
{
	if(CMainServer::GetInstance()->GetServerMode() != GM_LOGIN)
	{
		int nHuFuSum = GetBagStoreBodyItemCount(_nGiftID);
		if(0 == nHuFuSum)
		{
			AddItem(_nGiftID);
			return true;
		}
	}
	else
	{
		if(!m_xGiftItemIDs.empty())
		{
			size_t uSize = m_xGiftItemIDs.size();

			for(size_t i = 0; i < uSize; ++i)
			{
				int nItemID = m_xGiftItemIDs[i];
				int nItemCount = GetBagStoreBodyItemCount(nItemID);

				if(nItemID == _nGiftID &&
					0 == nItemCount)
				{
					ItemAttrib item;

					if(GetRecordInItemTable(nItemID, &item))
					{
						AddItem(nItemID);
						/*
						char szMsg[MAX_PATH];
												sprintf(szMsg, "系统赠送给您一件[%s]", item.name);
												SendSystemMessage(szMsg);*/
						return true;
						
					}
				}
			}
		}
	}

	return false;
}

DWORD HeroObject::GetLastDrugUseTime(int _nItemID)
{
	DrugUseTimeMap::const_iterator fndIter = m_xDrugUseTime.find(_nItemID);
	if(fndIter == m_xDrugUseTime.end())
	{
		return 0;
	}
	else
	{
		return fndIter->second;
	}
}

void HeroObject::SetLastDrugUseTime(int _nItemID, DWORD _dwTime)
{
	DrugUseTimeMap::iterator fndIter = m_xDrugUseTime.find(_nItemID);
	if(fndIter == m_xDrugUseTime.end())
	{
		m_xDrugUseTime.insert(std::make_pair(_nItemID, _dwTime));
	}
	else
	{
		fndIter->second = _dwTime;
	}
}

bool HeroObject::CanUseCoolDownDrug(int _nItemID, DWORD _dwCoolDownTime)
{
	DWORD dwLastUseTime = GetLastDrugUseTime(_nItemID);

	if(GetTickCount() > dwLastUseTime + _dwCoolDownTime)
	{
		return true;
	}
	return false;
}

void HeroObject::CheckServerNetDelay()
{
	if(!m_bClientLoaded)
	{
		return;
	}

	DWORD dwCurrentTick = GetTickCount();
	if(dwCurrentTick - m_dwLastCheckServerNetDelay > 20 * 1000)
	{
		m_dwLastCheckServerNetDelay = dwCurrentTick;

		PkgPlayerServerDelayReq req;
		req.uTargetId = GetID();
		req.nSeq = ++m_nServerNetDelaySeq;
		req.dwTimeStamp = dwCurrentTick;

		g_xThreadBuffer.Reset();
		g_xThreadBuffer << req;
		SendPacket(req);
	}
}

void HeroObject::SyncRandSeedNormalAtk()
{
	PkgPlayerUpdateRandSeedNot not;
	not.nType = RANDSEED_NORMALATK;
	not.dwSeed = m_xNormalAttackRand.GetSeed();
	not.uTargetId = GetID();
	SendPacket(not);
}

void HeroObject::LoginSvr_UpdatePlayerRank()
{
	if(CMainServer::GetInstance()->GetServerMode() != GM_LOGIN)
	{
		return;
	}

	DWORD dwLsIndex = CMainServer::GetInstance()->GetLSConnIndex();
	if(dwLsIndex == 0)
	{
		return;
	}

	if(IsGmHide())
	{
		//	GM不进入排行榜
		return;
	}

	if (GetProtoType() == ProtoType_ByteBuffer)
	{
		PkgLoginUpdateHumRankReq req;
		req.nUid = GetUID();
		req.cJob = GetUserData()->bJob;
		req.nLevel = GetObject_Level();
		char szName[20] = {0};
		if(IsEncrypt())
		{
			ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
		}
		else
		{
			strcpy(szName, GetUserData()->stAttrib.name);
		}

		if(strlen(szName) == 0)
		{
			return;
		}

		req.xName = szName;

		switch(GetUserData()->bJob)
		{
		case 0:
			{
				req.nPower = GetObject_MaxDC();
			}break;
		case 1:
			{
				req.nPower = GetObject_MaxMC();
			}break;
		case 2:
			{
				req.nPower = GetObject_MaxSC();
			}break;
		default:
			{
				req.nPower = 0;
			}break;
		}

		g_xConsole.CPrint("Update rank:%s uid:%d job:%d level:%d power:%d",
			req.xName.c_str(), req.nUid, req.cJob, req.nLevel, req.nPower);

		g_xThreadBuffer.Reset();
		g_xThreadBuffer << req;
		SendBufferToServer(dwLsIndex, &g_xThreadBuffer);
	}
	else
	{
		protocol::MUpdatePlayerRankReq req;
		req.set_uid(GetUID());
		req.set_job(GetUserData()->bJob);
		req.set_level(GetObject_Level());
		req.set_name(GetName());
		req.set_serverid(GetServerID());

		switch(GetUserData()->bJob)
		{
		case 0:
			{
				req.set_power(GetObject_MaxDC());
			}break;
		case 1:
			{
				req.set_power(GetObject_MaxMC());
			}break;
		case 2:
			{
				req.set_power(GetObject_MaxSC());
			}break;
		default:
			{
				req.set_power(0);
			}break;
		}

		g_xConsole.CPrint("Update rank:%s uid:%d job:%d level:%d power:%d",
			req.name(), req.uid(), req.job(), req.level(), req.power());

		SendProtoToServer(dwLsIndex, protocol::UpdatePlayerRankReq, req);
	}
}

//	extend json
void HeroObject::WriteExtendJson(string& _refData)
{
	
}

void HeroObject::ReadExtendJson(string& _refData)
{
	
}

//	get extend attrib
void HeroObject::GetHeroExtendAttrib(ExtendAttribVector& _refVec, bool _bIsSelf /* = true */)
{
	static ByteBuffer s_xExtAttribBuf;

	UINT uCounter = 0;
	s_xExtAttribBuf.Reset();

	//	write hair style
	if(0 != m_stExtAttrib.uHair)
	{
		s_xExtAttribBuf << (USHORT)kExtendAttrib_Hair;
		s_xExtAttribBuf << (int)m_stExtAttrib.uHair;
		++uCounter;
	}

	//	wing
	if(0 != m_stExtAttrib.uWing)
	{
		s_xExtAttribBuf << (USHORT)kExtendAttrib_Wing;
		s_xExtAttribBuf << (int)m_stExtAttrib.uWing;
		++uCounter;
	}

	//	cloth look
	if(0 != m_stExtAttrib.uClothLook)
	{
		s_xExtAttribBuf << (USHORT)kExtendAttrib_ClothLook;
		s_xExtAttribBuf << (int)m_stExtAttrib.uClothLook;
		++uCounter;
	}

	//	weapon look
	if(0 != m_stExtAttrib.uWeaponLook)
	{
		s_xExtAttribBuf << (USHORT)kExtendAttrib_WeaponLook;
		s_xExtAttribBuf << (int)m_stExtAttrib.uWeaponLook;
		++uCounter;
	}

	//	name frame
	if(0 != m_stExtAttrib.uNameFrame)
	{
		s_xExtAttribBuf << (USHORT)kExtendAttrib_NameFrame;
		s_xExtAttribBuf << (int)m_stExtAttrib.uNameFrame;
		++uCounter;
	}

	//	chat frame
	if(0 != m_stExtAttrib.uChatFrame)
	{
		s_xExtAttribBuf << (USHORT)kExtendAttrib_ChatFrame;
		s_xExtAttribBuf << (int)m_stExtAttrib.uChatFrame;
		++uCounter; 
	}

	//	只发给自己的
	if(_bIsSelf)
	{
		if(0 != m_stExtAttrib.uSmeltOreLevel)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_SmeltOreLevel;
			s_xExtAttribBuf << (int)m_stExtAttrib.uSmeltOreLevel;
			++uCounter;
		}
		if(0 != m_stExtAttrib.uSmeltOreExp)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_SmeltOreExp;
			s_xExtAttribBuf << (int)m_stExtAttrib.uSmeltOreExp;
			++uCounter;
		}

		if(0 != m_stExtAttrib.uSmeltWoodLevel)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_SmeltWoodLevel;
			s_xExtAttribBuf << (int)m_stExtAttrib.uSmeltWoodLevel;
			++uCounter;
		}
		if(0 != m_stExtAttrib.uSmeltWoodExp)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_SmeltWoodExp;
			s_xExtAttribBuf << (int)m_stExtAttrib.uSmeltWoodExp;
			++uCounter;
		}

		if(0 != m_stExtAttrib.uSmeltClothLevel)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_SmeltClothLevel;
			s_xExtAttribBuf << (int)m_stExtAttrib.uSmeltClothLevel;
			++uCounter;
		}
		if(0 != m_stExtAttrib.uSmeltClothExp)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_SmeltClothExp;
			s_xExtAttribBuf << (int)m_stExtAttrib.uSmeltClothExp;
			++uCounter;
		}

		if(0 != m_stExtAttrib.uSmeltGemLevel)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_SmeltGemLevel;
			s_xExtAttribBuf << (int)m_stExtAttrib.uSmeltGemLevel;
			++uCounter;
		}
		if(0 != m_stExtAttrib.uSmeltGemExp)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_SmeltGemExp;
			s_xExtAttribBuf << (int)m_stExtAttrib.uSmeltGemExp;
			++uCounter;
		}

		if(0 != m_stExtAttrib.uMakeEquipLevel)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_MakeEquipLevel;
			s_xExtAttribBuf << (int)m_stExtAttrib.uMakeEquipLevel;
			++uCounter;
		}
		if(0 != m_stExtAttrib.uMakeEquipExp)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_MakeEquipExp;
			s_xExtAttribBuf << (int)m_stExtAttrib.uMakeEquipExp;
			++uCounter;
		}

		if(0 != m_stExtAttrib.uChatColor)
		{
			s_xExtAttribBuf << (USHORT)kExtendAttrib_ChatColor;
			s_xExtAttribBuf << (int)m_stExtAttrib.uChatColor;
			++uCounter;
		}
	}

	if(0 != uCounter)
	{
		_refVec.resize(uCounter);

		for(int i = 0; i < uCounter; ++i)
		{
			ExtendAttribItem item;
			s_xExtAttribBuf >> item;
			_refVec[i] = item;
		}
	}
}

//	save extend bytes
void HeroObject::WriteSaveExtendField(ByteBuffer &_refBuffer)
{
	ExtendAttribVector xExtAttrib;
	GetHeroExtendAttrib(xExtAttrib);
	_refBuffer << xExtAttrib;
}

bool HeroObject::ReadSaveExtendField(ByteBuffer& _refBuffer)
{
	//	read counter
	if(_refBuffer.GetLength() < sizeof(UINT))
	{
		return false;
	}

	UINT uCounter = 0;
	_refBuffer >> uCounter;

	if(_refBuffer.GetLength() < uCounter * (sizeof(USHORT) + sizeof(int)))
	{
		return false;
	}

	//	read specified attribute
	for(USHORT i = 0; i < uCounter; ++i)
	{
		ExtendAttribItem item;
		_refBuffer >> item;

		switch(item.uType)
		{
		case kExtendAttrib_Hair:
			{
				m_stExtAttrib.uHair = item.nValue;
			}break;
		case kExtendAttrib_Wing:
			{
				m_stExtAttrib.uWing = item.nValue;
			}break;
		case kExtendAttrib_SmeltOreLevel:
			{
				m_stExtAttrib.uSmeltOreLevel = item.nValue;
			}break;
		case kExtendAttrib_SmeltOreExp:
			{
				m_stExtAttrib.uSmeltOreExp = item.nValue;
			}break;
		case kExtendAttrib_SmeltWoodLevel:
			{
				m_stExtAttrib.uSmeltWoodLevel = item.nValue;
			}break;
		case kExtendAttrib_SmeltWoodExp:
			{
				m_stExtAttrib.uSmeltWoodExp = item.nValue;
			}break;
		case kExtendAttrib_SmeltClothLevel:
			{
				m_stExtAttrib.uSmeltClothLevel = item.nValue;
			}break;
		case kExtendAttrib_SmeltClothExp:
			{
				m_stExtAttrib.uSmeltClothExp = item.nValue;
			}break;
		case kExtendAttrib_SmeltGemLevel:
			{
				m_stExtAttrib.uSmeltGemLevel = item.nValue;
			}break;
		case kExtendAttrib_SmeltGemExp:
			{
				m_stExtAttrib.uSmeltGemExp = item.nValue;
			}break;
		case kExtendAttrib_MakeEquipLevel:
			{
				m_stExtAttrib.uMakeEquipLevel = item.nValue;
			}break;
		case kExtendAttrib_MakeEquipExp:
			{
				m_stExtAttrib.uMakeEquipExp = item.nValue;
			}break;
		case kExtendAttrib_ChatColor:
			{
				m_stExtAttrib.uChatColor = item.nValue;
			}break;
		case kExtendAttrib_ClothLook:
			{
				m_stExtAttrib.uClothLook = item.nValue;
			}break;
		case kExtendAttrib_WeaponLook:
			{
				m_stExtAttrib.uWeaponLook = item.nValue;
			}break;
		case kExtendAttrib_NameFrame:
			{
				m_stExtAttrib.uNameFrame = item.nValue;
			}break;
		case kExtendAttrib_ChatFrame:
			{
				m_stExtAttrib.uChatFrame = item.nValue;
			}break;
		default:
			{
				LOG(ERROR) << "Invalid extend attribute type:" << item.uType << " value:" << item.nValue;
			}break;
		}
	}

	return true;
}

void HeroObject::SendPlayerDataTo(HeroObject *_pRecvHero)
{
	//	vip等级
	int nVipLevel = GetDonateLevel();
	if(0 != nVipLevel)
	{
		PkgPlayerVipNot ppvn;
		ppvn.dwID = GetID();
		ppvn.nVipLevel = nVipLevel;
		ppvn.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ppvn;
		_pRecvHero->SendPlayerBuffer(g_xThreadBuffer);
	}

	//	该玩家的状态
	PkgPlayerGStatusNtf statusNtf;
	GetStatusInfo(statusNtf);
	statusNtf.uTargetId = GetID();
	if(!statusNtf.xStatus.empty())
	{
		_pRecvHero->SendPacket(statusNtf);
	}

	//	玩家的外观
	PkgPlayerExtendAttribNot pean;
	pean.uTargetId = GetID();
	GetHeroExtendAttrib(pean.xAttrib, _pRecvHero == this ? true : false);
	_pRecvHero->SendPacket(pean);
}

void HeroObject::SetEnterTimeLimitScene(GameScene* _pScene)
{
	m_nTimeLimitID = _pScene->GetTimeLimitID();
	if(m_nTimeLimitID == 0)
	{
		m_nEnterTimeLimitMapTime = 0;
		return;
	}

	m_nEnterTimeLimitMapTime = GetTickCount();
}

void HeroObject::OnHeroKilledMonster(MonsterObject* _pMons)
{

}

void HeroObject::UpdateLifeSkillLevel(LifeSkillType _eType, int _nLevel)
{
	if(_eType == kLifeSkill_SmeltOre)
	{
		m_stExtAttrib.uSmeltOreLevel = _nLevel;
	}
	else if(_eType == kLifeSkill_SmeltWood)
	{
		m_stExtAttrib.uSmeltWoodLevel = _nLevel;
	}
	else if(_eType == kLifeSkill_SmeltCloth)
	{
		m_stExtAttrib.uSmeltClothLevel = _nLevel;
	}
	else if(_eType == kLifeSkill_SmeltGem)
	{
		m_stExtAttrib.uSmeltGemLevel = _nLevel;
	}
	else if(_eType == kLifeSkill_SmeltEquip)
	{
		m_stExtAttrib.uMakeEquipLevel = _nLevel;
	}

	PkgPlayerExtendAttribNot pean;
	pean.uTargetId = GetID();
	GetHeroExtendAttrib(pean.xAttrib);
	SendPacket(pean);

}

int HeroObject::TransferIdentifyAttrib(int _nItemTag0, int _nItemTag1, int _nCount)
{
	ItemAttrib* pItem0 = GetItemByTag(_nItemTag0);
	ItemAttrib* pItem1 = GetItemByTag(_nItemTag1);

	if(NULL == pItem0 ||
		NULL == pItem1)
	{
		return 1;
	}

	if(!IsEquipItem(GETITEMATB(pItem0, Type)) ||
		!IsEquipItem(GETITEMATB(pItem1, Type)))
	{
		return 2;
	}

	int nHideCount0 = HideAttribHelper::GetAllAttribCount(GETITEMATB(pItem0, MaxMP));
	int nHideCount1 = HideAttribHelper::GetAllAttribCount(GETITEMATB(pItem1, MaxMP));

	if(0 == nHideCount0 &&
		0 == nHideCount1)
	{
		return 3;
	}

	if(0 != nHideCount0 &&
		0 != nHideCount1)
	{
		return 4;
	}

	//	开始转移属性 随机的
	int nTransferAttribCount = _nCount;
	ItemAttrib* pTransferItem = NULL;
	ItemAttrib* pWaitTransferItem = NULL;

	if(0 != nHideCount0 &&
		nHideCount0 < _nCount)
	{
		nTransferAttribCount = nHideCount0;
	}
	else if(0 != nHideCount1 &&
		nHideCount1 < _nCount)
	{
		nTransferAttribCount = nHideCount1;
	}

	if(0 != nHideCount0)
	{
		pTransferItem = pItem0;
		pWaitTransferItem = pItem1;
	}
	else if(0 != nHideCount1)
	{
		pTransferItem = pItem1;
		pWaitTransferItem = pItem0;
	}
	else
	{
		return -1;
	}

	//	检查是否有未鉴定的
	unsigned int uHideAttribData = GETITEMATB(pTransferItem, MaxMP);
	if(HideAttribHelper::GetAllAttribCount(uHideAttribData) != HideAttribHelper::GetActiveAttribCount(uHideAttribData))
	{
		return 5;
	}

	int nTransferAttribIndex[3] = {-1, -1, -1};
	for(int i = 0; i < nTransferAttribCount; ++i)
	{
		int nAttribIndex = rand() % HideAttribHelper::GetAllAttribCount(uHideAttribData);
		bool bExists = false;

		for(int j = 0; j < 3; ++j)
		{
			if(nTransferAttribIndex[j] == nAttribIndex)
			{
				bExists = true;
				break;
			}
		}

		if(bExists)
		{
			//	索引已存在
			--i;
			continue;
		}
		else
		{
			for(int j = 0; j < 3; ++j)
			{
				if(nTransferAttribIndex[j] == -1)
				{
					nTransferAttribIndex[j] = nAttribIndex;
					break;
				}
			}
		}
	}

	//	转移属性
	unsigned int uResultAttribData = 0;
	HideAttribHelper::SetAllAttribCount(uResultAttribData, nTransferAttribCount);
	HideAttribHelper::SetActiveAttribCount(uResultAttribData, 0);

	for(int i = 0; i < 3; ++i)
	{
		if(nTransferAttribIndex[i] != -1)
		{
			int nAttribType = HideAttribHelper::GetActiveAttribType(nTransferAttribIndex[i], uHideAttribData);
			int nAttribValue = HideAttribHelper::GetActiveAttribValue(nTransferAttribIndex[i], uHideAttribData);

			HideAttribHelper::SetActiveAttribType(uResultAttribData, i, nAttribType);
			HideAttribHelper::SetActiveAttribValue(uResultAttribData, i, nAttribValue);
		}
		else
		{
			break;
		}
	}

	SETITEMATB(pTransferItem, MaxMP, 0);
	SETITEMATB(pWaitTransferItem, MaxMP, uResultAttribData);

	if(CMainServer::GetInstance()->GetServerMode() != GM_LOGIN)
	{
		SetItemBind(pTransferItem, true);
		SetItemBind(pWaitTransferItem, true);
	}

	PkgPlayerUpdateItemNtf ppuin;
	ppuin.uTargetId = GetID();
	ppuin.stItem = *pTransferItem;
	ObjectValid::DecryptAttrib(&ppuin.stItem);
	SendPacket(ppuin);
	ppuin.uTargetId = GetID();
	ppuin.stItem = *pWaitTransferItem;
	ObjectValid::DecryptAttrib(&ppuin.stItem);
	SendPacket(ppuin);

	return 0;
}

int HeroObject::TransferIdentifyAttribFailed(int _nItemTag0, int _nItemTag1)
{
	ItemAttrib* pItem0 = GetItemByTag(_nItemTag0);
	ItemAttrib* pItem1 = GetItemByTag(_nItemTag1);

	if(NULL == pItem0 ||
		NULL == pItem1)
	{
		return 1;
	}

	if(!IsEquipItem(GETITEMATB(pItem0, Type)) ||
		!IsEquipItem(GETITEMATB(pItem1, Type)))
	{
		return 2;
	}

	int nHideCount0 = HideAttribHelper::GetAllAttribCount(GETITEMATB(pItem0, MaxMP));
	int nHideCount1 = HideAttribHelper::GetAllAttribCount(GETITEMATB(pItem1, MaxMP));

	if(0 == nHideCount0 &&
		0 == nHideCount1)
	{
		return 3;
	}

	if(0 != nHideCount0 &&
		0 != nHideCount1)
	{
		return 4;
	}

	//	开始转移属性 随机的
	ItemAttrib* pTransferItem = NULL;
	ItemAttrib* pWaitTransferItem = NULL;

	if(0 != nHideCount0)
	{
		pTransferItem = pItem0;
		pWaitTransferItem = pItem1;
	}
	else if(0 != nHideCount1)
	{
		pTransferItem = pItem1;
		pWaitTransferItem = pItem0;
	}
	else
	{
		return -1;
	}

	//	检查是否有未鉴定的
	unsigned int uHideAttribData = GETITEMATB(pTransferItem, MaxMP);
	if(HideAttribHelper::GetAllAttribCount(uHideAttribData) != HideAttribHelper::GetActiveAttribCount(uHideAttribData))
	{
		return 5;
	}

	SETITEMATB(pTransferItem, MaxMP, 0);
	SETITEMATB(pWaitTransferItem, MaxMP, 0);

	if(CMainServer::GetInstance()->GetServerMode() != GM_LOGIN)
	{
		SetItemBind(pTransferItem, true);
		SetItemBind(pWaitTransferItem, true);
	}

	PkgPlayerUpdateItemNtf ppuin;
	ppuin.uTargetId = GetID();
	ppuin.stItem = *pTransferItem;
	ObjectValid::DecryptAttrib(&ppuin.stItem);
	SendPacket(ppuin);
	ppuin.uTargetId = GetID();
	ppuin.stItem = *pWaitTransferItem;
	ObjectValid::DecryptAttrib(&ppuin.stItem);
	SendPacket(ppuin);

	return 0;
}

void HeroObject::SetItemBind(ItemAttrib* _pItem, bool _bBind)
{
	unsigned int uAtkPois = GETITEMATB(_pItem, AtkPois);
	if(_bBind)
	{
		SET_FLAG(uAtkPois, POIS_MASK_BIND);
	}
	else
	{
		RESET_FLAG(uAtkPois, POIS_MASK_BIND);
	}
	SETITEMATB(_pItem, AtkPois, uAtkPois);
}

void HeroObject::SetEffStatus(DWORD _effMask, DWORD _time, DWORD _param)
{
	int nEffTime = _time;

	if(_effMask == MMASK_LVDU ||
		_effMask == MMASK_STONE)
	{
		if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_ENERGYSHIELD))
		{
			const UserMagic* pMgc = GetUserMagic(MEFF_ENERGYSHIELD);
			if(pMgc != NULL)
			{
				if(pMgc->bLevel > 3)
				{
					int nLevel = pMgc->bLevel - 3;
					int nReduceTime = nLevel * 1000;
					nEffTime -= nReduceTime;
				}
			}
		}

		int nEffRecover = 0;
		if(_effMask == MMASK_LVDU)
		{
			nEffRecover = m_xStates.GetStateValue(EAID_POISRECOVER);
		}
		else if(_effMask == MMASK_STONE)
		{
			nEffRecover = m_xStates.GetStateValue(EAID_STONERECOVER);
		}
		if(0 != nEffTime)
		{
			nEffTime -= nEffRecover * 1000;
		}
	}

	if(nEffTime < 1500)
	{
		nEffTime = 1500;
	}

	__super::SetEffStatus(_effMask, nEffTime, _param);
}

void HeroObject::Lua_SetQuestStep(int _nQuestId, int _nStep)
{
	m_xQuest.SetQuestStep(_nQuestId, _nStep);
	SyncQuestData(_nQuestId);
}

int HeroObject::Lua_GetQuestStep(int _nQuestId)
{
	return m_xQuest.GetQuestStep(_nQuestId);
}

void HeroObject::Lua_SetQuestCounter(int _nQuestId, int _nCounter)
{
	m_xQuest.SetQuestCounter(_nQuestId, _nCounter);
	SyncQuestData(_nQuestId);
}

int HeroObject::Lua_GetQuestCounter(int _nQuestId)
{
	return m_xQuest.GetQuestCounter(_nQuestId);
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::OnPlayerLevelUp(int _nCurrentLevel)
{
	LoginSvr_UpdatePlayerRank();
}

void HeroObject::OnPlayerDressdItem(int _nTag)
{
	//LoginSvr_UpdatePlayerRank();
}

void HeroObject::OnPlayerLogin()
{
	//LoginSvr_UpdatePlayerRank();
}

bool HeroObject::CanPk()
{
	if(GetPkType() == kHeroPkType_All &&
		GetLocateScene()->GetMapPkType() == kMapPkType_All)
	{
		return true;
	}
	return false;
}

bool HeroObject::CanPkPlayer(HeroObject* _pHero)
{
	if(NULL == _pHero)
	{
		return false;
	}
	if(GetLocateScene()->GetMapPkType() != kMapPkType_All)
	{
		return false;
	}

	if(GetPkType() == kHeroPkType_All)
	{
		return true;
	}

	return false;
}

void HeroObject::ProcessDelayAction()
{
	DWORD dwTick = GetTickCount();
	DelayActionList::iterator it = m_xDelayActions.begin();

	for(it;
		it != m_xDelayActions.end();
		)
	{
		if(m_stData.eGameState == OS_DEAD)
		{
			break;
		}

		DelayActionBase* pAction = *it;

		if(pAction->dwActiveTime != 0 &&
			pAction->dwActiveTime > dwTick)
		{
			//	ignore
			++it;
			continue;
		}

		switch(pAction->uOp)
		{
		case kDelayAction_Struck:
			{
				PkgPlayerStruckAck ack;
				ack.uTargetId = GetID();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ack;
				SendPacket(ack);
			}break;
		case kDelayAction_Heal:
			{
				DelayActionHeal* pHealAction = (DelayActionHeal*)pAction;
				AddHealState(pHealAction->nHealCnt);
			}break;
		case kDelayAction_MakeSlave:
			{
				DelayActionMakeSlave* pMakeSlaveAction = (DelayActionMakeSlave*)pAction;
				int nSlaveID = pMakeSlaveAction->nSlaveId;
				int nSlaveMax = 1;
				const UserMagic* pUsrMgc = NULL;

				switch(nSlaveID)
				{
				case 29://	变异骷髅
					{
						pUsrMgc = GetUserMagic(MEFF_KULOU);
					}break;
				case 14://	神兽
					{
						pUsrMgc = GetUserMagic(MEFF_SUMMON);
					}break;
				case 62://	上古神兽
					{
						pUsrMgc = GetUserMagic(MEFF_SUPERSUMMON);
					}break;
				case 99://	白虎
					{
						pUsrMgc = GetUserMagic(MEFF_SUMMONTIGER);
					}break;
				case 144:
					{
						pUsrMgc = GetUserMagic(MEFF_SUMMONBOWMAN);
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

						if(GetSlaveCount() < nSlaveMax)
						{
							MakeSlave(nSlaveID);
						}
					}
					else if(nSlaveID == 99)
					{
						if(pUsrMgc->bLevel == 3 ||
							pUsrMgc->bLevel == 6)
						{
							nSlaveMax = 2;
						}

						if(GetSlaveCount() < nSlaveMax)
						{
							MakeSlave(nSlaveID);
						}
					}
					else if(nSlaveID == 144)
					{
						if(pUsrMgc->bLevel == 3 ||
							pUsrMgc->bLevel == 6)
						{
							nSlaveMax = 2;
						}

						if(GetSlaveCount() < nSlaveMax)
						{
							MakeSlave(nSlaveID);
						}
					}
				}
			}break;
		case kDelayAction_StaticMagic:
			{
				DelayActionStaticMagic* pStaticMagicAction = (DelayActionStaticMagic*)pAction;
				const UserMagic* pUserMgc = GetUserMagic(pStaticMagicAction->nMagicId);

				if(NULL == pUserMgc)
				{
					break;
				}
				if(0 == pUserMgc->bLevel)
				{
					break;
				}
				if(GetMapID() != pStaticMagicAction->nMapId)
				{
					break;
				}

				StaticMagic sm = {0};
				sm.dwFire = GetID();
				sm.sPosX = pStaticMagicAction->nPosX;
				sm.sPoxY = pStaticMagicAction->nPosY;
				sm.wMaxDC = pStaticMagicAction->wMaxDC;
				sm.wMinDC = pStaticMagicAction->wMinDC;
				sm.dwEnableTime = 0;
				sm.dwEffectActive = 2;
				sm.dwExpire = GetTickCount() + pStaticMagicAction->nContinueTime;
				sm.wMgcID = pStaticMagicAction->nMagicId;
				sm.wMgcLevel = pStaticMagicAction->nMagicLevel;
				sm.pFire = this;
				GetLocateScene()->PushStaticMagic(&sm);
			}break;
		}

		//	delete
		delete pAction;
		pAction = NULL;
		it = m_xDelayActions.erase(it);
	}

	if(m_stData.eGameState == OS_DEAD)
	{
		ClearDelayAction();
	}
}

void HeroObject::IdentifyLowLevelEquip(ItemAttrib* _pItem)
{
	UINT uHideAttribCode = GETITEMATB(_pItem, MaxMP);
	if(0 == uHideAttribCode)
	{
		return;
	}

	int nActiveAttribSum = HideAttribHelper::GetActiveAttribCount(uHideAttribCode);
	int nAllAttribSum = HideAttribHelper::GetAllAttribCount(uHideAttribCode);

	if(nActiveAttribSum >= nAllAttribSum)
	{
		PkgPlayerQuickMsgNtf ntf;
		ntf.uTargetId = GetID();
		ntf.nMsgID = QMSG_IDENTIFYFULL;
		SendPacket(ntf);
		return;
	}

	//	扣钱
	int nCostMoney = 0;
	nCostMoney = (1 + GetItemUpgrade(GETITEMATB(_pItem, Level))) * 1000;

	if(GetMoney() < nCostMoney)
	{
		PkgPlayerQuickMsgNtf ntf;
		ntf.uTargetId = GetID();
		ntf.nMsgID = QMSG_NOENOUGHMONEY;
		SendPacket(ntf);
		return;
	}

	MinusMoney(nCostMoney);

	nActiveAttribSum++;
	HideAttribHelper::SetActiveAttribCount(uHideAttribCode, nActiveAttribSum);
	SETITEMATB(_pItem, MaxMP, uHideAttribCode);

	int nValue = GETITEMATB(_pItem, AtkPois);
	SET_FLAG(nValue, POIS_MASK_BIND);
	SETITEMATB(_pItem, AtkPois, nValue);

	PkgPlayerQuickMsgNtf ntf;
	ntf.uTargetId = GetID();
	ntf.nMsgID = QMSG_IDENTIFYOK;
	SendPacket(ntf);

	SyncItemAttrib(_pItem->tag);
}

bool HeroObject::IsMagicAttackValid(int _nMagicID, int _nTargetX, int _nTargetY)
{
	int nSelfX = m_stData.wCoordX;
	int nSelfY = m_stData.wCoordY;

	// check target in attack range
// 	if (abs(nSelfX - _nTargetX) > VIEW_WIDTH / UNIT_WIDTH / 2 + 3 ||
// 		abs(nSelfY - _nTargetY) > VIEW_HEIGHT / UNIT_HEIGHT / 2 + 3)
	if (abs(nSelfX - _nTargetX) >= VIEW_WIDTH / UNIT_WIDTH / 2 + 3 ||
		abs(nSelfY - _nTargetY) >= VIEW_HEIGHT / UNIT_HEIGHT / 2 + 3)
	{
		m_nInvalidMagicAttackTimes++;

		return false;
	}

	return true;
}

void HeroObject::ForceDisconnectHero()
{
	CMainServer::GetInstance()->ForceCloseConnection(GetUserIndex());
}

void HeroObject::Lua_OpenChestBox(ItemAttrib* _pItem, int _nItemID, int _nItemLv)
{
	UINT uItemData = GETITEMATB(_pItem, MaxHP);
	WORD wItemID = 0;
	WORD wItemLv = 0;

	if (0 != uItemData)
	{
		wItemID = LOWORD(uItemData);
		wItemLv = HIWORD(uItemData);
	}
	else
	{
		// Random get item, just in battle net mode
		if (CMainServer::GetInstance()->GetServerMode() == GM_LOGIN &&
			0 != _nItemID)
		{
			wItemID = _nItemID;
			wItemLv = _nItemLv;
		}
	}

	int nTag = _pItem->tag;
	ZeroMemory(_pItem, sizeof(ItemAttrib));
	ObjectValid::EncryptAttrib(_pItem);

	PkgPlayerLostItemAck ack;
	ack.uTargetId = GetID();
	ack.dwTag = nTag;
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << ack;
	SendBuffer(GetUserIndex(), &g_xThreadBuffer);

	if (0 != wItemID &&
		wItemLv <= 8)
	{
		AddSuperItem(wItemID, wItemLv);
	}
}