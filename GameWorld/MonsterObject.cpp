#include "../CMainServer/CMainServer.h"
#include "MonsterObject.h"
#include "GameSceneManager.h"
#include "DBThread.h"
#include "GameWorld.h"
#include "ObjectValid.h"
#include "ExceptionHandler.h"
#include <process.h>
#include "../../CommonModule/SettingLoader.h"
//////////////////////////////////////////////////////////////////////////
extern HWND g_hServerDlg;
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
MonsterObject::MonsterObject(/*DWORD _dwID*/)/* : GameObject(_dwID)*/: m_xAttackBuf(10)
{
	m_eType = SOT_MONSTER;
	m_pTarget = NULL;

	m_dwWalkCost = 0;
	m_dwAttackCost = 0;

	m_dwLastSearchTime = 0;

	m_pMaster = NULL;

	m_xMsgQueue.Resize(1024);

	m_bCanDropItems = true;
	m_bSlaveKilledByMaster = false;

	m_nLastRecvDamage = 0;
	m_nTotalRecvDamage = 0;

	m_dwLastWalkTime = GetTickCount();

	m_nStoneProb = 5;
	m_nStoneTime = 2000;

	m_nMasterIdBeforeDie = 0;

	m_nLifeTimes = 0;
	m_bCanStruck = true;
	m_bDeadAsSlave = false;
}

MonsterObject::~MonsterObject()
{
	ClearAttackProcess();
}

//////////////////////////////////////////////////////////////////////////
void MonsterObject::SetMaster(GameObject *_pMst)
{
	m_pMaster = _pMst;
	if(NULL != _pMst)
	{
		m_nMasterIdBeforeDie = _pMst->GetID();
	}
}
//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* virtual void DoWork(unsigned int _dwTick)
/************************************************************************/
void MonsterObject::DoWork(unsigned int _dwTick)
{
	RECORD_FUNCNAME_WORLD;

	m_dwCurrentTime = GetTickCount();

	if(!DoMsgQueue(_dwTick))
	{
		//	No delayed process exist
		DoAction(_dwTick);
	}

	ProcessAttackProcess();
	ProcessDelayAction();

	if(GetObject_HP() == 0 &&
		m_stData.eGameState != OS_DEAD)
	{
		//GameWorld::GetInstance().Stop(STOP_MONSNOTDEAD);
		m_stData.eGameState = OS_DEAD;
	}

	//	自动踢出堵门的宠物
	//if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
	{
		HeroObject* pMaster = (HeroObject*)GetMaster();

		if(NULL != pMaster)
		{
			if(_dwTick - m_dwLastWalkTime > 20 * 1000)
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
						PostMessage(g_hServerDlg, WM_CLOSECONNECTION, pMaster->GetUserIndex(), 0);
					}
				}
			}
		}
	}
}

void MonsterObject::UpdateStatus(DWORD _dwCurTick)
{
	RECORD_FUNCNAME_WORLD;

	if(m_pMaster &&
		m_dwDefectTime != 0)
	{
		if(_dwCurTick > m_dwDefectTime)
		{
			// 这里因为没有清除主人那儿的宠物指针，导致主人死亡时候服务器崩溃[11/14/2013 Administrator]
			HeroObject* pMaster = static_cast<HeroObject*>(m_pMaster);
			pMaster->ClearSlave(this);
			// End [11/14/2013 Administrator]
			SetMaster(NULL);
			SetMasterIdBeforeDie(0);
			m_dwDefectTime = 0;
		}
	}

	if(GetLocateScene()->IsAutoReset())
	{
		//	Full hp
		if(GetLocateScene()->GetPlayerCount() == 0 &&
			GetObject_HP() != 0 &&
			m_stData.eGameState != OS_DEAD &&
			GetMaster() == NULL &&
			GameWorld::GetInstance().GetAutoReset())
		{
			SetObject_HP(GetObject_MaxHP());
			m_pValid->SetHP(GetObject_HP());
		}
	}

	__super::UpdateStatus(_dwCurTick);
}

void MonsterObject::DoAction(unsigned int _dwTick)
{
	RECORD_FUNCNAME_WORLD;

	PkgObjectActionNot stActNot;
	GameScene* pScene = GetLocateScene();
	if(!pScene)
	{
#ifdef _DEBUG
		LOG(ERROR) << "In no scene!";
#endif
		return;
	}

	UpdateStatus(_dwTick);

	if(m_stData.eGameState == OS_STOP)
	{
		if(GetMaster() == NULL)
		{
			m_stData.eGameState = OS_STAND;
		}
	}

	switch(m_stData.eGameState)
	{
	case OS_ATTACK:
		{
			if(m_dwCurrentTime - m_dwLastAttackTime >= m_dwAttackCost)
			{
				m_stData.eGameState = OS_STAND;
				m_dwLastAttackTime = _dwTick;
			}
		}break;
	case OS_WALK:
		{
			if(m_dwCurrentTime - m_dwLastWalkTime >= m_dwWalkCost)
			{
				m_stData.eGameState = OS_STAND;
				m_dwLastWalkTime = _dwTick;
			}
		}break;
	case OS_STRUCK:
		{
			//	
			if(m_dwCurrentTime - m_dwLastStruckTime >= INTERVAL_STRUCK)
			{
				m_stData.eGameState = OS_STAND;
				m_dwLastStruckTime = _dwTick;

				//	Force to attack
				if(m_pMaster == NULL)
				{
					MonsterLogic();
				}
				else
				{
					SlaveLogic();
				}
			}
		}break;
	case OS_DEAD:
		{
			//
			if(m_dwCurrentTime - m_dwLastDeadTime >= INTERVAL_DEAD / 2)
			{
				//	Clean the dead body
				//GetLocateScene()->AddWaitRemove(this);
				GetLocateScene()->AddWaitDelete(this);
			}
		}break;
	case OS_STAND:
		{
			if(m_pMaster == NULL)
			{
				MonsterLogic();
			}
			else
			{
				SlaveLogic();
			}
		}break;
	}
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::MonsterLogic()
{
	RECORD_FUNCNAME_WORLD;

	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_STONE))
	{
		return;
	}
	if(GetViewRange() > 0)
	{
		//	Auto search target
		if(m_pTarget == NULL)
		{
			OnLoseTarget();
			if(m_dwCurrentTime - /*m_stData.*/m_dwLastSearchTime >= INTERVAL_SEARCH / 10)
			{
				m_pTarget = SearchViewRange();
				m_dwLastSearchTime = m_dwCurrentTime;
			}
			if(m_pTarget == NULL)
			{
				Wonder();
			}
			else
			{
				OnFindTarget();
			}
		}
		else
		{
			if(m_pTarget->GetMapID() != GetMapID())
			{
				SetTarget(NULL);
				return;
			}
			AttackHero();
		}
	}
	else
	{
		if(m_pTarget == NULL)
		{
			//	Auto walk
			Wonder();
		}
		else
		{
			//	Try to attack hero
			AttackHero();
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::SlaveLogic()
{
	RECORD_FUNCNAME_WORLD;

	bool bInSameMap = true;
	GameScene* pMasterScene = m_pMaster->GetLocateScene();

	if(m_pTarget == NULL)
	{
		//	If other slaves have target
		if(m_pMaster)
		{
			if(m_pMaster->GetType() == SOT_HERO)
			{
				HeroObject* pHero = static_cast<HeroObject*>(m_pMaster);
				for(int i = 0; i < MAX_SLAVE_SUM; ++i)
				{
					if(pHero->GetSlave(i) != this &&
						pHero->GetSlave(i) != NULL)
					{
						if(pHero->GetSlave(i)->GetType() == SOT_MONSTER)
						{
							MonsterObject* pMonster = static_cast<MonsterObject*>(pHero->GetSlave(i));
							if(pMonster->GetTarget() != NULL)
							{
								if(pMonster->GetTarget()->GetMapID() == GetMapID())
								{
									SetTarget(pMonster->GetTarget());
								}
								return;
							}
						}
					}
				}
			}
		}
		//	search view range to find a target
		if(m_dwCurrentTime - /*m_stData.*/m_dwLastSearchTime >= INTERVAL_SEARCH / 10)
		{
			m_pTarget = SearchViewRange();
			m_dwLastSearchTime = m_dwCurrentTime;
		}
		if(NULL != m_pTarget)
		{
			return;
		}
		//	Walk to master
		if(m_stData.wMapID != m_pMaster->GetUserData()->wMapID)
		{
			//	Fly to master
			bInSameMap = false;
			int nFlyX = 0;
			int nFlyY = 0;
			for(int i = 0; i < 8; ++i)
			{
				nFlyX = m_pMaster->GetUserData()->wCoordX + g_nMoveOft[i * 2];
				nFlyY = m_pMaster->GetUserData()->wCoordY + g_nMoveOft[i * 2 + 1];
				if(pMasterScene->CanThrough(nFlyX, nFlyY))
				{
					FlyToMap(nFlyX, nFlyY, m_pMaster->GetUserData()->wMapID);
					bInSameMap = true;
					break;
				}
			}
		}
		else
		{
			//	walk to master
			if(abs(m_stData.wCoordX - m_pMaster->GetUserData()->wCoordX) > 15 ||
				abs(m_stData.wCoordY - m_pMaster->GetUserData()->wCoordY) > 15)
			{
				int nFlyX = 0;
				int nFlyY = 0;
				for(int i = 0; i < 8; ++i)
				{
					nFlyX = m_pMaster->GetUserData()->wCoordX + g_nMoveOft[i * 2];
					nFlyY = m_pMaster->GetUserData()->wCoordY + g_nMoveOft[i * 2 + 1];
					if(GetLocateScene()->CanThrough(nFlyX, nFlyY))
					{
						FlyTo(nFlyX, nFlyY);
						break;
					}
				}
			}
			else
			{
				WalkToMaster();
			}
		}
	}
	else
	{
		if(m_stData.wMapID != m_pMaster->GetUserData()->wMapID)
		{
			//	Fly to master
			bInSameMap = false;
			int nFlyX = 0;
			int nFlyY = 0;
			for(int i = 0; i < 8; ++i)
			{
				nFlyX = m_pMaster->GetUserData()->wCoordX + g_nMoveOft[i * 2];
				nFlyY = m_pMaster->GetUserData()->wCoordY + g_nMoveOft[i * 2 + 1];
				if(pMasterScene->CanThrough(nFlyX, nFlyY))
				{
					FlyToMap(nFlyX, nFlyY, m_pMaster->GetUserData()->wMapID);
					bInSameMap = true;
					break;
				}
			}
			SetTarget(NULL);
			return;
		}

		//	Attack target
		AttackHero();
	}
}
//////////////////////////////////////////////////////////////////////////
int MonsterObject::GetRandomAbility(ABILITY_TYPE _type)
{
	RECORD_FUNCNAME_WORLD;

	if(_type == AT_AC)
	{
		int nAC = GetObject_AC();
		if(TEST_FLAG(m_dwHumEffFlag, MMASK_LVDU))
		{
			float fDefenceMinus = float(m_dwPoison * 2) / 100;
			if(fDefenceMinus > 0.5f)
			{
				fDefenceMinus = 0.5f;
			}
			nAC *= (1.0f - fDefenceMinus);
		}
		return nAC;
	}
	else if(_type == AT_MAC)
	{
		int nMAC = GetObject_MAC();
		if(TEST_FLAG(m_dwHumEffFlag, MMASK_LVDU))
		{
			float fDefenceMinus = float(m_dwPoison * 2) / 100;
			if(fDefenceMinus > 0.5f)
			{
				fDefenceMinus = 0.5f;
			}
			nMAC *= (1.0f - fDefenceMinus);
		}
		return nMAC;
	}
	else
	{
		return __super::GetRandomAbility(_type);
	}
}
//////////////////////////////////////////////////////////////////////////
int MonsterObject::GetDefPalsyProb()
{
	if(GetObject_ID() == 140 ||
		GetObject_ID() == 143 ||
		GetObject_ID() == 159)
	{
		//	破天魔龙 70%几率防麻痹
		return 70;
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::Say(const char* _pszSaying)
{
	PkgChatNot not;
	not.xMsg += _pszSaying;
	not.uTargetId = GetID();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer, GetID());
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::AddAttackProcess(ByteBuffer& _xBuf)
{
	RECORD_FUNCNAME_WORLD;

	if(0 == m_xAttackBuf.Write(_xBuf.GetHead(), _xBuf.GetLength()))
	{
		LOG(FATAL) << "Fatal error occurred while writing to the attack buffer";
	}
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::AddAttackProcess(AttackMsg* _pMsg)
{
	RECORD_FUNCNAME_WORLD;

	m_xAttackMsgList.push_back(_pMsg);
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::ParseAttackMsg(AttackMsg* _pMsg)
{
	RECORD_FUNCNAME_WORLD;

	GameScene* pScene = GetLocateScene();
	if(NULL == pScene)
	{
#ifdef _DEBUG
		LOG(ERROR) << "In no scene!";
		return;
#endif
	}

	if(m_stData.eGameState == OS_DEAD)
	{
		return;
	}

	PkgObjectActionNot not;
	not.uTargetId = GetID();
	not.uParam0 = MAKELONG(m_stData.wCoordX, m_stData.wCoordY);
	not.uAction = ACTION_STRUCK;
	not.uParam1 = m_stData.nDrt;

	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_STONE) &&
		CanStoneRestore())
	{
		ResetEffStatus(MMASK_STONE);

		PkgPlayerSetEffectAck ack;
		ack.bShow = false;
		ack.dwMgcID = MMASK_STONE;
		ack.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	}

	m_nLastRecvDamage = _pMsg->wDamage;
	m_nTotalRecvDamage += _pMsg->wDamage;

	//if(req.uAction == ACTION_ATTACK)
	{
		GameObject* pAttacker = NULL;
		if(_pMsg->dwAttacker != 0)
		{
			pAttacker = pScene->GetPlayerWithoutLock(_pMsg->dwAttacker);
			if(NULL == pAttacker)
			{
				pAttacker = pScene->GetNPCByHandleID(_pMsg->dwAttacker);
			}
		}

		if(pAttacker != NULL)
		{
			//	是否是宠物的攻击
			if(GetMaster() != NULL)
			{
				if(pAttacker->GetType() == SOT_MONSTER)
				{
					if(static_cast<MonsterObject*>(pAttacker)->GetMaster() != NULL)
					{
						return;
					}
				}
			}
		}
		/*if(GetObject_HP() > info.wDamage)*/
		if(GetObject_HP() > _pMsg->wDamage)
		{
			/*DecHP(info.wDamage);*/
			DecHP(_pMsg->wDamage);
			/*m_pValid->DecHP(info.wDamage);*/
			m_pValid->DecHP(_pMsg->wDamage);

			not.uParam2 = MAKELONG(GetObject_HP(), GetObject_MaxHP());

			//	set param3 mask
			not.uParam3 = 0;

			if(pAttacker != NULL)
			{
				if(_pMsg->bType != 0)
				{
					//	魔法攻击
					not.uParam3 = 0;
				}
				else
				{
					//	物理攻击
					if(pAttacker->GetType() == SOT_HERO)
					{
						HeroObject* pHero = static_cast<HeroObject*>(pAttacker);
						ItemAttrib* pWeapon = pHero->GetEquip(PLAYER_ITEM_WEAPON);
						if(GETITEMATB(pWeapon, Type) != ITEM_NO)
						{
							SET_FLAG(not.uParam3, STRUCK_MASK_WEAPON);
						}
					}
				}
			}

			//	critical flag
			if(TEST_FLAG_BOOL(_pMsg->dwMasks, ATTACKMSG_MASK_CRITICAL))
			{
				SET_FLAG(not.uParam3, STRUCK_MASK_CRITICAL);
			}

			if(m_stData.eGameState != OS_STRUCK)
			{
				if(m_bCanStruck)
				{
					m_stData.eGameState = OS_STRUCK;
					/*m_stData.*/m_dwLastStruckTime = m_dwCurrentTime;
				}
			}

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			pScene->BroadcastPacket(&g_xThreadBuffer);

			//	Set the target
			if(GetUserData()->eGameState != OS_STOP &&
				pAttacker != NULL)
			{
				if(pAttacker->GetMapID() == GetMapID())
				{
					SetTarget(pAttacker);
				}
				else
				{
					SetTarget(NULL);
				}
			}

			/*if(info.bType == MEFF_ICEPALM)*/
			if(_pMsg->bType == MEFF_ICEPALM ||
				_pMsg->bType == MEFF_ICETHRUST)
			{
				if(!IsFrozen() &&
					pAttacker != NULL &&
					!CanDefIce())
				{
					if(pAttacker->GetType() == SOT_HERO)
					{
						HeroObject* pHero = static_cast<HeroObject*>(pAttacker);
						const UserMagic* pMgc = NULL;

						if(_pMsg->bType == MEFF_ICEPALM)
						{
							pMgc = pHero->GetUserMagic(MEFF_ICEPALM);
						}
						else if(_pMsg->bType == MEFF_ICETHRUST)
						{
							pMgc = pHero->GetUserMagic(MEFF_ICETHRUST);
						}

						if(NULL != pMgc)
						{
							if(pMgc->bLevel > 0)
							{
								int nCanFrozen = 0;
								nCanFrozen = pMgc->bLevel * 20;
								if(nCanFrozen > rand() % 100)
								{
									DWORD dwLastsTime = 0;
									dwLastsTime = pMgc->bLevel * 1000;
									dwLastsTime += (pHero->GetRandomAbility(AT_MC) * 200);
#ifdef _DEBUG
									dwLastsTime = 30000;
#endif

									SetEffStatus(MMASK_ICE, dwLastsTime, 0);

									PkgPlayerSetEffectAck ack;
									ack.uTargetId = GetID();
									ack.bShow = true;
									ack.dwMgcID = MMASK_ICE;
									ack.dwTime = dwLastsTime;
									g_xThreadBuffer.Reset();
									g_xThreadBuffer << ack;
									GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
								}
							}
						}
					}
				}
			}
		}
		else
		{
			//	Dead
			//m_pValid->SetHP(0);
			//PROTECT_START_VM
			if(m_nLifeTimes > 0)
			{
				--m_nLifeTimes;

				//	回血
				ItemAttrib atb;
				if(GetRecordInMonsterTable(GetObject_ID(), &atb))
				{
					SetObject_HP(GetObject_MaxHP());
					m_pValid->SetHP(GetObject_MaxHP());

					not.uParam2 = MAKELONG(GetObject_HP(), GetObject_MaxHP());
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << not;
					pScene->BroadcastPacket(&g_xThreadBuffer);

					OnDeadRevive(m_nLifeTimes);

					return;
				}
			}

			SetObject_HP(0);
			/*m_pValid->DecHP(info.wDamage);*/
			m_pValid->DecHP(_pMsg->wDamage);

			bool bCanDropItem = true;

			if(!m_pValid->TestValid())
			{
				DEBUG_BREAK;
				//LOG(INFO) << "CHEATER";
				//PostQuitMessage(0);
				//_endthreadex(10);
				GameWorld::GetInstance().Stop(10);
				//FlyToPrison();
				bCanDropItem = false;
			}

			/*m_stData.*/m_dwLastDeadTime = m_dwCurrentTime;
			not.uParam2 = MAKELONG(GetObject_HP(), GetObject_MaxHP());
			//PROTECT_END_VM

			m_stData.eGameState = OS_DEAD;
			ResetSupply();
			not.uAction = ACTION_DEAD;
			not.uParam0 = MAKE_POSITION_DWORD(this);

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			pScene->BroadcastPacket(&g_xThreadBuffer);

			if(GetMaster() != NULL)
			{
				bCanDropItem = false;
				m_bDeadAsSlave = true;
				if(m_pMaster->GetType() == SOT_HERO)
				{
					HeroObject* pMaster = static_cast<HeroObject*>(m_pMaster);
					pMaster->ClearSlave(this);
					m_nMasterIdBeforeDie = pMaster->GetID();
				}
				SetMaster(NULL);
			}

			/*GameObject* pAttacker = NULL;
			if(_pMsg->dwAttacker != 0)
			{
				pAttacker = pScene->GetPlayerWithoutLock(_pMsg->dwAttacker);
				if(NULL == pAttacker)
				{
					pAttacker = pScene->GetNPCByHandleID(_pMsg->dwAttacker);
				}
			}*/

			HeroObject* pHeroAttacker = NULL;
			bool bKilledBySlave = false;
			int nFinnalExprMulti = GameWorld::GetInstance().GetFinnalExprMulti();
			int nMonsterId = GetObject_ID();

			if(pAttacker != NULL &&
				GetObject_HP() == 0 &&
				m_stData.eGameState == OS_DEAD)
			{
				if(pAttacker->GetType() == SOT_HERO)
				{
					HeroObject* pHero = static_cast<HeroObject*>(pAttacker);
					pHeroAttacker = pHero;
				}
				else if(pAttacker->GetType() == SOT_MONSTER)
				{
					MonsterObject* pSlave = static_cast<MonsterObject*>(pAttacker);
					if(pSlave->GetUserData()->eGameState == OS_DEAD)
					{
						//	可能宠物已经死了
						int nMasterId = pSlave->GetMasterIdBeforeDie();
						if(0 != nMasterId)
						{
							pHeroAttacker = (HeroObject*)pScene->GetPlayerWithoutLock(nMasterId);
							bKilledBySlave = true;
						}
					}
					else
					{
						if(pSlave->GetMaster())
						{
							if(pSlave->GetMaster()->GetType() == SOT_HERO)
							{
								pHeroAttacker = static_cast<HeroObject*>(pSlave->GetMaster());
								bKilledBySlave = true;

								if(pSlave->AddSlaveExpr(GetObject_Expr()))
								{
									PkgPlayerUpdateAttribNtf uantf;
									uantf.bType = UPDATE_SLAVELV;
									uantf.dwParam = pSlave->GetObject_MaxAC();
									uantf.uTargetId = pSlave->GetID();
									g_xThreadBuffer.Reset();
									g_xThreadBuffer << uantf;
									pSlave->GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
								}
							}
						}
					}
				}

				if(NULL != pHeroAttacker)
				{
					OnMonsterDead(pHeroAttacker, bKilledBySlave);
				}

				if(bCanDropItem)
				{
					//PROTECT_START
						if(GetObject_HP() == 0 &&
							m_stData.eGameState == OS_DEAD)
						{
							//DropMonsterItems();
							DropMonsterItems(pHeroAttacker);
						}
						//PROTECT_END
				}

				SetTarget(NULL);
			}

			//	清空所有目标为this的对象的目标
			//GetLocateScene()->EraseTarget(this);
		}
	}

	/*if(m_stData.eGameState == OS_DEAD)
	{
		break;
	}*/
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::KilledByMaster()
{
	if(GetMaster() != NULL)
	{
		m_bSlaveKilledByMaster = true;
	}
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::ProcessAttackProcess()
{
	RECORD_FUNCNAME_WORLD;

	//////////////////////////////////////////////////////////////////////////
	//	New version
	GameScene* pScene = GetLocateScene();
	if(NULL == pScene)
	{
#ifdef _DEBUG
		LOG(ERROR) << "In no scene!";
#endif
		return;
	}
	if(m_stData.eGameState == OS_DEAD ||
		m_stData.eGameState == OS_APPEAR ||
		m_stData.eGameState == OS_SHOW)
	{
		//m_xAttackBuf.Reset();
		//	Return all msg
		//	所有攻击无效
		AttackMsgList::const_iterator begIter = m_xAttackMsgList.begin();
		AttackMsgList::const_iterator endIter = m_xAttackMsgList.end();
		AttackMsg* pMsg = NULL;

		for(begIter;
			begIter != endIter;
			++begIter)
		{
			pMsg = *begIter;
			FreeListManager::GetInstance()->PushFreeAttackMsg(pMsg);
		}

		m_xAttackMsgList.clear();
		return;
	}

	if(m_bSlaveKilledByMaster)
	{
		//	被主人杀死了 则添加必死攻击
		m_bSlaveKilledByMaster = false;
		AttackMsg* pMsg = FreeListManager::GetInstance()->GetFreeAttackMsg();
		pMsg->wDamage = /*pSlave->GetObject_MaxHP()*/0xFFFF;
		pMsg->dwInfo = 0;
		pMsg->dwAttacker = 0;
		AddAttackProcess(pMsg);
	}

	if(m_stData.eGameState == OS_STAND ||
		m_stData.eGameState == OS_STOP)
	{
		DWORD dwTick = GetTickCount();
		//	Now can struck
		PkgObjectActionNot not;
		not.uTargetId = GetID();
		not.uParam0 = MAKELONG(m_stData.wCoordX, m_stData.wCoordY);
		not.uAction = ACTION_STRUCK;
		not.uParam1 = m_stData.nDrt;
		AttackInfo info;

		if(!m_xAttackMsgList.empty())
		{
			AttackMsgList::const_iterator begIter = m_xAttackMsgList.begin();
			AttackMsgList::const_iterator endIter = m_xAttackMsgList.end();
			AttackMsg* pMsg = NULL;

			/*for(begIter;
				begIter != endIter;
				)
			{
				pMsg = *begIter;

				if(m_stData.eGameState != OS_DEAD)
				{
					ParseAttackMsg(pMsg);
				}
					
				//	Remove this one 
				FreeListManager::GetInstance()->PushFreeAttackMsg(pMsg);
				begIter = m_xAttackMsgList.erase(begIter);
			}*/
			for(begIter;
				begIter != m_xAttackMsgList.end();
				)
			{
				pMsg = *begIter;

				if(m_stData.eGameState == OS_DEAD)
				{
					break;
				}

				if(pMsg->dwActiveTime != 0 &&
					pMsg->dwActiveTime > dwTick)
				{
					++begIter;
					continue;
				}

				ParseAttackMsg(pMsg);

				//	remove the msg
				FreeListManager::GetInstance()->PushFreeAttackMsg(pMsg);
				begIter = m_xAttackMsgList.erase(begIter);
			}

			//	if dead, we need clear all attack msg
			if(m_stData.eGameState == OS_DEAD)
			{
				ClearAttackProcess();
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::ClearAttackProcess()
{
	AttackMsgList::iterator it = m_xAttackMsgList.begin();
	for(it;
		it != m_xAttackMsgList.end();
		++it)
	{
		FreeListManager::GetInstance()->PushFreeAttackMsg(*it);
	}
	m_xAttackMsgList.clear();
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::DropMonsterItems()
{
	RECORD_FUNCNAME_WORLD;

	if(m_stData.eGameState != OS_DEAD)
	{
		return;
	}
	if(!m_pValid->TestValid())
	{
		return;
	}
	if(!m_bCanDropItems)
	{
		return;
	}

	DBOperationParam* pParam = new DBOperationParam;
	pParam->dwOperation = DO_QUERY_DROPITEM;
	pParam->dwParam[0] = GetObject_ID();
	pParam->dwParam[1] = MAKE_POSITION_DWORD(this);
	std::list<int>* pDropItems = new std::list<int>;
	pParam->dwParam[2] = (DWORD)pDropItems;
	pParam->dwParam[3] = m_stData.wMapID;

	DBThread::GetInstance()->AsynExecute(pParam);
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::DropMonsterItems(HeroObject* _pHero)
{
	RECORD_FUNCNAME_WORLD;

	if(_pHero == NULL)
	{
		return;
	}

	if(m_stData.eGameState != OS_DEAD)
	{
		return;
	}
	if(!m_pValid->TestValid())
	{
		//return;
	}
	if(!m_bCanDropItems)
	{
		return;
	}

	DBOperationParam* pParam = new DBOperationParam;
	pParam->dwOperation = DO_QUERY_DROPITEMEX;
	pParam->dwParam[0] = GetObject_ID();
	pParam->dwParam[1] = MAKE_POSITION_DWORD(this);
	std::list<int>* pDropItems = new std::list<int>;
	pParam->dwParam[2] = (DWORD)pDropItems;
	
	//pParam->dwParam[3] = m_stData.wMapID;
	if(IsElite())
	{
		pParam->dwParam[3] = MAKELONG(1, m_stData.wMapID);
	}
	else if(IsLeader())
	{
		pParam->dwParam[3] = MAKELONG(2, m_stData.wMapID);
	}
	else
	{
		pParam->dwParam[3] = MAKELONG(0, m_stData.wMapID);
	}
	//pParam->dwParam[4] = _pHero->GetID();
	//pParam->dwParam[5] = GetID();
	WORD wDropMuiti = 0;
	if(_pHero->CanActiveDoubleDrop())
	{
		wDropMuiti = 2;
	}
	WORD wDifficultyLevel = 0;
	pParam->dwParam[4] = MAKELONG(wDropMuiti, wDifficultyLevel);

	pParam->dwParam[6] = MAKELONG(m_pValid->TestValid() ? 1 : 0, _pHero->GetStateController()->GetMagicItemAddition());

	//DBThread::GetInstance()->AsynExecute(pParam);
	lua_State* L = GameWorld::GetInstance().GetLuaState();
	lua_getglobal(L, "DropDownMonsItems");
	tolua_pushusertype(L, pParam, "DBOperationParam");
	tolua_pushusertype(L, _pHero, "HeroObject");
	tolua_pushusertype(L, this, "MonsterObject");
	int nRet = lua_pcall(L, 3, 0, 0);

	if(0 == nRet)
	{

	}
	else
	{
#ifdef _DEBUG
		//LOG(WARNING) << "Execute script(OnMapLoaded) On[" << m_xScript.GetMapName() << "]" << lua_tostring(m_xScript.GetScript(), -1);
		LOG(WARNING) << lua_tostring(L, -1);
#endif
		lua_pop(L, 1);
	}
}
//////////////////////////////////////////////////////////////////////////
GameObject* MonsterObject::SearchViewRange()
{
	RECORD_FUNCNAME_WORLD;

	if(GetObject_SC() == 0)
	{
		//	Not a initiative attacking object
		return NULL;
	}

	GameScene* pScene = GetLocateScene();
	if(NULL == pScene)
	{
#ifdef _DEBUG
		LOG(ERROR) << "In no scene!";
#endif
		return NULL;
	}
	if(GetViewRange() == 0)
	{
		return NULL;
	}

	RECT rcSearch;
	int nValue = 0;
	nValue = m_stData.wCoordX - GetObject_SC();
	if(nValue < 0)	nValue = 0;
	rcSearch.left = nValue;
	rcSearch.right = m_stData.wCoordX + GetObject_SC();
	nValue = m_stData.wCoordY - GetObject_SC();
	if(nValue < 0) nValue = 0;
	rcSearch.top = nValue;
	rcSearch.bottom = m_stData.wCoordY + GetObject_SC();

	//return pScene->GetOnePlayerInRange(rcSearch, CanSeeHide());
	if(GetMaster() == NULL)
	{
		return pScene->MonsterGetOneTargetInRange(rcSearch, CanSeeHide());
	}
	else
	{
		return pScene->SlaveGetOneTargetInRange(rcSearch, CanSeeHide());
	}
}
//////////////////////////////////////////////////////////////////////////
DWORD MonsterObject::GetAttackCostTime()
{
	RECORD_FUNCNAME_WORLD;

	int nAtkFrame = 6;
	if(GetObject_ID() == 80)
	{
		nAtkFrame = 8;
	}
	else if(GetObject_ID() == 144)
	{
		nAtkFrame = 10;
	}
	DWORD dwCost = MAX_ATTACK_INTERVAL;
	dwCost = (MAX_ATTACK_INTERVAL - GetObject_AtkSpeed() * 5) * (nAtkFrame + 2) + 50;
	if(dwCost > MAX_ATTACK_INTERVAL)
	{
		//dwCost = MAX_ATTACK_INTERVAL;
	}
	return dwCost;
}

DWORD MonsterObject::GetWalkCostTime()
{
	RECORD_FUNCNAME_WORLD;

	int nMoveSpeed = GetObject_MoveSpeed();
	if(nMoveSpeed > 10)
	{
		nMoveSpeed = 10;
	}
	DWORD dwMoveCost = NORMAL_WALK_COST_TIME - nMoveSpeed * 50;
	dwMoveCost *= 1.2f;
	return dwMoveCost;


	WORD dwCost = MAX_MOVE_INTERVAL;
	dwCost = (MAX_MOVE_INTERVAL + GetObject_MoveSpeed() * 10) / 2;
	/*if(dwCost > 220)
	{
		dwCost = 220;
	}*/

	float fMoveTime = (60.0f) / (float)dwCost;
	dwCost = fMoveTime * 1000;
	return dwCost;
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::Wonder()
{
	RECORD_FUNCNAME_WORLD;

	if(m_stData.eGameState != OS_STAND)
	{
		return;
	}
	if(IsUnmovableMonster())
	{
		return;
	}

	if(m_dwCurrentTime - /*m_stData.*/m_dwLastWalkTime > GetWalkInterval())
	{
		int nValue = rand() % 20;
		if(nValue <= 3)
		{
			// Turn the direction
			nValue = rand() % 8;

			if(nValue != m_stData.nDrt)
			{
//  				m_stData.nDrt = nValue;
//  
//  				PkgObjectActionNot not;
//  				not.uAction = ACTION_TURN;
//  				not.uParam0 = MAKE_POSITION_DWORD(this);
//  				not.uParam1 = m_stData.nDrt;
//  				not.uTargetId = GetID();
//  				g_xThreadBuffer.Reset();
//  				g_xThreadBuffer << not;
//  				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
//  				/*m_stData.*/m_dwLastWalkTime = m_dwCurrentTime;
				TurnTo(nValue);
			}
		}
		else if(nValue >= 12)
		{
			//	Walk
// 			nValue = rand() % 8;
// 			PkgObjectActionNot not;
// 			not.uAction = ACTION_WALK;
// 			not.uParam0 = MAKE_POSITION_DWORD(this);
// 			not.uTargetId = GetID();

			//if(WalkOneStep(nValue))

			if(WalkOneStep(m_stData.nDrt))
			{
// 				not.uParam1 = MAKE_POSITION_DWORD(this);
// 				/*m_stData.*/m_dwLastWalkTime = m_dwCurrentTime;
// 				g_xThreadBuffer.Reset();
// 				g_xThreadBuffer << not;
// 				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
// 				/*m_stData.*/m_dwLastWalkTime = m_dwCurrentTime;
			}
			else
			{
				for(int i = 0; i < 7; ++i)
				{
					nValue = rand() % 8;
					if(WalkOneStep(nValue))
					{
// 						not.uParam1 = MAKE_POSITION_DWORD(this);
// 						/*m_stData.*/m_dwLastWalkTime = m_dwCurrentTime;
// 						g_xThreadBuffer.Reset();
// 						g_xThreadBuffer << not;
// 						GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
// 						/*m_stData.*/m_dwLastWalkTime = m_dwCurrentTime;
						break;
					}
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
/*
bool MonsterObject::WalkOneStep(int _nDrt)
{
	if(_nDrt < 0 &&
		_nDrt > 7)
	{
		return false;
	}

	PkgObjectActionNot not;
	not.uAction = ACTION_WALK;
	not.uTargetId = GetID();
	not.uParam0 = MAKE_POSITION_DWORD(this);

	int nTargetX = m_stData.wCoordX + s_nMoveOft[_nDrt * 2];
	int nTargetY = m_stData.wCoordY + s_nMoveOft[_nDrt * 2 + 1];
	if(GetLocateScene()->CanMove(nTargetX, nTargetY))
	{
		m_stData.wCoordX = nTargetX;
		m_stData.wCoordY = nTargetY;
		m_stData.nDrt = _nDrt;
		m_stData.eGameState = OS_WALK;
		/ *m_stData.* /m_dwLastWalkTime = m_dwCurrentTime;

		not.uParam1 = MAKE_POSITION_DWORD(this);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		return true;
	}
	return false;
}*/

//////////////////////////////////////////////////////////////////////////
void MonsterObject::AttackHero()
{
	RECORD_FUNCNAME_WORLD;

	if(m_pTarget == NULL)
	{
		return;
	}
	if(m_stData.eGameState != OS_STAND)
	{
		return;
	}
	if(m_pTarget->GetObject_HP() == 0 ||
		m_pTarget->GetUserData()->eGameState == OS_DEAD)
	{
		SetTarget(NULL);
		return;
	}

	//	Out of range
	/*
	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
		int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;
		if(abs(nOftX) > 10 ||
			abs(nOftY) > 10)*/
	if(IsOutOfView())
	{
		SetTarget(NULL);
		return;
	}

	if(!CanAttack())
	{
		//	Go to target
		if(m_dwCurrentTime - /*m_stData.*/m_dwLastWalkTime > GetWalkInterval())
		{
			WalkToTarget();
			//m_stData.dwLastWalkTime = m_dwCurrentTime;
		}
	}
	else
	{
		if(m_dwCurrentTime - /*m_stData.*/m_dwLastAttackTime > GetAttackInterval())
		{
			AttackTarget();
		}
	}
}
//////////////////////////////////////////////////////////////////////////
bool MonsterObject::IsOutOfView()
{
	RECORD_FUNCNAME_WORLD;

	if(m_pTarget == NULL)
	{
		return true;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;
	if(abs(nOftX) > 10 ||
		abs(nOftY) > 10)

	{
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool MonsterObject::CanAttack()
{
	RECORD_FUNCNAME_WORLD;

	if(m_pTarget == NULL)
	{
		return false;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

	if(abs(nOftX) > GetObject_MaxSC() ||
		abs(nOftY) > GetObject_MaxSC())
	{
		return false;
	}

	if(nOftY == 0 &&
		nOftX == 0)
	{
		return false;
	}

	if(nOftX == 0 ||
		nOftY == 0)
	{
		return true;
	}

	if(abs(nOftX) == abs(nOftY))
	{
		return true;
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::WalkToTarget()
{
	RECORD_FUNCNAME_WORLD;

	if(m_pTarget == NULL)
	{
		return;
	}

	if(IsUnmovableMonster())
	{
		return;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

	if(nOftX == 0 &&
		nOftY == 0)
	{
		for(int i = 0; i < 8 ; ++i)
		{
			if(WalkOneStep(i))
			{
				break;
			}
		}
		return;
	}

	int nWalkX = -2;
	int nWalkY = -2;
	int nDrt = -1;

	if(nOftX == 0)
	{
		nWalkX = 0;
	}
	else if(nOftX > 0)
	{
		nWalkX = 1;
	}
	else if(nOftX < 0)
	{
		nWalkX = -1;
	}
	if(nOftY == 0)
	{
		nWalkY = 0;
	}
	else if(nOftY > 0)
	{
		nWalkY = 1;
	}
	else if(nOftY < 0)
	{
		nWalkY = -1;
	}

	for(int i = 0; i < 8; ++i)
	{
		if(nWalkX == s_nMoveOft[i * 2] &&
			nWalkY == s_nMoveOft[i * 2 + 1])
		{
			nDrt = i;
			break;
		}
	}
	if(nDrt != -1)
	{
		if(!WalkOneStep(nDrt))
		{
			for(int i = 0; i < 7; ++i)
			{
				nDrt = rand() % 8;
				if(WalkOneStep(nDrt))
				{
					break;
				}
			}
		}
		else
		{
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::WalkToMaster()
{
	RECORD_FUNCNAME_WORLD;

	if(m_pMaster == NULL)
	{
		return;
	}
	if(IsUnmovableMonster())
	{
		return;
	}
	if(m_stData.wMapID != m_pMaster->GetMapID())
	{
		return;
	}
	DWORD dwCurTick = GetTickCount();
	if(dwCurTick - m_dwLastWalkTime > GetWalkInterval())
	{
		int nOftX = m_pMaster->GetUserData()->wCoordX - m_stData.wCoordX;
		int nOftY = m_pMaster->GetUserData()->wCoordY - m_stData.wCoordY;

		if(nOftX == 0 &&
			nOftY == 0)
		{
			return;
		}

		int nWalkX = -2;
		int nWalkY = -2;
		int nDrt = -1;

		if(nOftX == 0)
		{
			nWalkX = 0;
		}
		else if(nOftX > 0)
		{
			nWalkX = 1;
		}
		else if(nOftX < 0)
		{
			nWalkX = -1;
		}
		if(nOftY == 0)
		{
			nWalkY = 0;
		}
		else if(nOftY > 0)
		{
			nWalkY = 1;
		}
		else if(nOftY < 0)
		{
			nWalkY = -1;
		}

		for(int i = 0; i < 8; ++i)
		{
			if(nWalkX == s_nMoveOft[i * 2] &&
				nWalkY == s_nMoveOft[i * 2 + 1])
			{
				nDrt = i;
				break;
			}
		}
		if(nDrt != -1)
		{
			if(!WalkOneStep(nDrt))
			{
				for(int i = 0; i < 7; ++i)
				{
					nDrt = rand() % 8;
					if(WalkOneStep(nDrt))
					{
						break;
					}
				}
			}
			else
			{
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void MonsterObject::ReceiveHeroDamage(GameObject* _pAttacker, int _nDamage, int _delay)
{
	RECORD_FUNCNAME_WORLD;

	if(_nDamage > 0 &&
		_delay > 0)
	{
		if(_pAttacker->GetType() == SOT_HERO)
		{
			MonsterObject* pMonster = static_cast<MonsterObject*>(_pAttacker);
			/*DelaySendInfo info;
			info.uOp = DELAY_HEROATTACK;
			info.uParam[0] = GetID();
			info.uParam[1] = _nDamage;
			info.uParam[2] = _pAttacker->GetID();
			info.uParam[3] = 0;
			info.uDelayTime = GetTickCount() + _delay;
			GetLocateScene()->PushDelayBuf(info);*/
			SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
			pMsg->uOp = DELAY_HEROATTACK;
			pMsg->uParam[0] = GetID();
			pMsg->uParam[1] = _nDamage;
			pMsg->uParam[2] = _pAttacker->GetID();
			pMsg->uParam[3] = 0;
			pMsg->dwDelayTime = GetTickCount() + _delay;
			GetLocateScene()->PushDelayBuf(pMsg);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
int MonsterObject::ReceiveDamage(GameObject* _pAttacker, bool _bMgcAtk, int _oriDC /* = 0 */, int _nDelay /* = 350 */, const ReceiveDamageInfo* _pInfo /* = NULL */)
{
	RECORD_FUNCNAME_WORLD;

	//	是否能被攻击
	if(_pAttacker->GetMapID() != GetMapID())
	{
		return 0;
	}
	if(_pAttacker == this)
	{
		return 0;
	}

	if(GetMaster() == NULL)
	{
		//	普通怪物
		if(_pAttacker->GetType() == SOT_HERO)
		{
			//	被玩家攻击了
		}
		else if(_pAttacker->GetType() == SOT_MONSTER)
		{
			//	被怪物攻击了
			MonsterObject* pMonster = (MonsterObject*)_pAttacker;

			if(pMonster->GetMaster() == NULL)
			{
				//	被普通怪物攻击了
				return 0;
			}
			else
			{
				//	被宝宝攻击了
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		HeroObject* pMaster = (HeroObject*)GetMaster();

		if(pMaster == _pAttacker)
		{
			return 0;
		}

		//	宝宝
		if(_pAttacker->GetType() == SOT_HERO)
		{
			HeroObject* pHero = (HeroObject*)_pAttacker;

			if(GetLocateScene()->GetMapPkType() == kMapPkType_None)
			{
				//	安全地图
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
					//	可以被攻击
				}
				else if(pHero->GetPkType() == kHeroPkType_Team)
				{
					if(pHero->GetTeamID() == pMaster->GetTeamID() &&
						pHero->GetTeamID() != 0)
					{
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
			MonsterObject* pMonster = (MonsterObject*)_pAttacker;

			if(pMonster->GetMaster() == NULL)
			{
				//	被普通怪物攻击
			}
			else
			{
				//	被宝宝攻击
				HeroObject* pAttackerMaster = (HeroObject*)pMonster->GetMaster();
				
				if(GetLocateScene()->GetMapPkType() == kMapPkType_None)
				{
					return 0;
				}
				else
				{
					if(pAttackerMaster->GetPkType() == kHeroPkType_None)
					{
						return 0;
					}
					else if(pAttackerMaster->GetPkType() == kHeroPkType_All)
					{
						if(pAttackerMaster->GetTeamID() == pMaster->GetTeamID() &&
							pAttackerMaster->GetTeamID() != 0)
						{
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
		else
		{
			return 0;
		}
	}

	int nDC = _pAttacker->GetRandomAbility(AT_DC);
	int nAC = 0;

	if(_oriDC != 0)
	{
		nDC = _oriDC;
	}

	if(m_stData.eGameState == OS_DEAD)
	{
		return 0;
	}

	if(_bMgcAtk)
	{
		nAC = GetRandomAbility(AT_MAC);
	}
	else
	{
		nAC = GetRandomAbility(AT_AC);
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
	else if(_pAttacker->GetType() == SOT_HERO)
	{
		HeroObject* pAttacker = (HeroObject*)_pAttacker;
		int nIgnoreAC = pAttacker->GetStateController()->GetIgnoreACAddition();
		if(nIgnoreAC)
		{
			float fPercent = 1.0f - (float)nIgnoreAC / 100;
			nAC *= fPercent;
		}
	}

	if(NULL != _pInfo &&
		_pInfo->bIgnoreAC)
	{
		nAC = 0;
	}

	int nDamage = nDC - nAC;
	if(nDamage <= 0)
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

	if(_pAttacker->GetType() == SOT_MONSTER)
	{
		MonsterObject* pMonster = static_cast<MonsterObject*>(_pAttacker);
		/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
		pMsg->uOp = DELAY_HEROATTACK;
		pMsg->uParam[0] = GetID();
		pMsg->uParam[1] = nDamage;
		pMsg->uParam[2] = _pAttacker->GetID();
		pMsg->uParam[3] = 0;
		pMsg->dwDelayTime = GetTickCount() + _nDelay;
		GetLocateScene()->PushDelayBuf(pMsg);*/

		/*AttackMsg* pMsg = FreeListManager::GetInstance()->GetFreeAttackMsg();
		pMsg->bType = 0;
		pMsg->dwAttacker = _pAttacker->GetID();
		pMsg->wDamage = nDamage;
		pMsg->dwInfo = 0;
		pMsg->dwActiveTime = GetTickCount() + _nDelay;
		AddAttackProcess(pMsg);*/

		DelayActionAttacked* pAction = new DelayActionAttacked;
		pAction->dwActiveTime = GetTickCount() + _nDelay;
		pAction->dwAttackerId = _pAttacker->GetID();
		pAction->nDamage = nDamage;
		AddDelayAction(pAction);
	}
	else if(_pAttacker->GetType() == SOT_HERO)
	{
		HeroObject* pHero = (HeroObject*)_pAttacker;
		if(_bMgcAtk)
		{
			if(pHero->GetHeroJob() == 1)
			{
				nDamage += pHero->GetStateController()->GetStateValue(EAID_MCDAMAGE);
			}
			else if(pHero->GetHeroJob() == 2)
			{
				nDamage += pHero->GetStateController()->GetStateValue(EAID_SCDAMAGE);
			}
		}
		else
		{
			if(pHero->GetHeroJob() == 0)
			{
				nDamage += pHero->GetStateController()->GetStateValue(EAID_DCDAMAGE);
			}
		}

		/*SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
		pMsg->uOp = DELAY_HEROATTACK;
		pMsg->uParam[0] = GetID();
		pMsg->uParam[1] = nDamage;
		pMsg->uParam[2] = _pAttacker->GetID();
		pMsg->uParam[3] = 0;
		pMsg->dwDelayTime = GetTickCount() + _nDelay;

		//	是否是暴击
		if(pHero->IsLastAttackCritical())
		{
			SET_FLAG(pMsg->uParam[5], ATTACKMSG_MASK_CRITICAL);
		}

		//	是否是魔法攻击
		if(_bMgcAtk)
		{
			pMsg->uParam[3] = pHero->GetLastUseMagicID();
		}

		GetLocateScene()->PushDelayBuf(pMsg);*/

		/*AttackMsg* pMsg = FreeListManager::GetInstance()->GetFreeAttackMsg();
		pMsg->bType = 0;
		pMsg->dwAttacker = _pAttacker->GetID();
		pMsg->wDamage = nDamage;
		pMsg->dwInfo = 0;
		pMsg->dwActiveTime = GetTickCount() + _nDelay;
		if(_bMgcAtk)
		{
			pMsg->bType = pHero->GetLastUseMagicID();
		}
		if(pHero->IsLastAttackCritical())
		{
			SET_FLAG(pMsg->dwMasks, ATTACKMSG_MASK_CRITICAL);
		}
		AddAttackProcess(pMsg);*/

		DelayActionAttacked* pAction = new DelayActionAttacked;
		pAction->dwActiveTime = GetTickCount() + _nDelay;
		pAction->dwAttackerId = _pAttacker->GetID();
		pAction->nDamage = nDamage;
		if(_bMgcAtk)
		{
			pAction->nMgcId = pHero->GetLastUseMagicID();
		}
		if(pHero->IsLastAttackCritical())
		{
			SET_FLAG(pAction->dwAttackMask, ATTACKMSG_MASK_CRITICAL);
		}
		AddDelayAction(pAction);
	}

	//	是否可以麻痹
	if(!_bMgcAtk)
	{
		if(_pAttacker->GetType() == SOT_HERO)
		{
			HeroObject* pHero = (HeroObject*)_pAttacker;

			bool bCanStone = false;
			int nStoneProb = pHero->GetStateController()->GetNearPalsyAddition();
			if(NULL != _pInfo &&
				_pInfo->bFarPalsy)
			{
				nStoneProb = pHero->GetStateController()->GetFarPalsyAddtion();
			}
			if(nStoneProb != 0)
			{
				if(rand() % 100 < nStoneProb)
				{
					bCanStone = true;
				}
			}

			if(bCanStone &&
				0 != nDamage)
			{
				//int nRandom = 2;
				//nRandom = rand() % STONE_PROPERTY;
				bool bCanPalsy = true;
				int nDefPalsyProb = GetDefPalsyProb();
				if(0 != nDefPalsyProb)
				{
					if(rand() % 100 < nDefPalsyProb)
					{
						bCanPalsy = false;
					}
				}

				if(bCanPalsy)
				{
					int nStoneTime = STONE_TIME;
					if(NULL != _pInfo &&
						0 != _pInfo->nStoneTime)
					{
						nStoneTime = _pInfo->nStoneTime;
					}

					SetEffStatus(MMASK_STONE, nStoneTime, 0);
					SetStoneRestore(false);

					PkgPlayerSetEffectAck ack;
					ack.bShow = true;
					ack.dwMgcID = MMASK_STONE;
					ack.uTargetId = GetID();
					ack.dwTime = nStoneTime;
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ack;
					GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
				}
			}

			int nGetHpPercent = pHero->GetStateController()->GetStateValue(EAID_STEALHP);
			if(nGetHpPercent > 0)
			{
				nGetHpPercent = nDamage * nGetHpPercent / 100;
				if(pHero->GetObject_HP() != pHero->GetObject_MaxHP())
				{
					pHero->IncHP(nGetHpPercent);
					pHero->GetValidCheck()->IncHP(nGetHpPercent);
					pHero->SyncHP(pHero);
				}
			}
		}
	}

	return nDamage;
}
//////////////////////////////////////////////////////////////////////////
bool MonsterObject::AttackTarget()
{
	RECORD_FUNCNAME_WORLD;

	if(NULL == m_pTarget)
	{
		return false;
	}
	if(m_pTarget->GetObject_HP() == 0 &&
		m_pTarget->GetUserData()->eGameState == OS_DEAD)
	{
		m_pTarget = NULL;
		return true;
	}
	if(m_stData.eGameState != OS_STAND)
	{
		return false;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

	int nDrt = -1;
	int nTimes = 0;
	int nSx = 0;
	int nSy = 0;
	bool bIsDead = false;

	if(nOftX > 0)
	{
		nSx = 1;
	}
	else if(nOftX < 0)
	{
		nSx = -1;
	}
	if(nOftY > 0)
	{
		nSy = 1;
	}
	else if(nOftY < 0)
	{
		nSy = -1;
	}

	for(int i = 0; i < 8; ++i)
	{
		if(nSx == s_nMoveOft[i * 2] &&
			nSy == s_nMoveOft[i * 2 + 1])
		{
			nDrt = i;
			break;
		}
	}

	bool bReqStuck = true;

	if(nDrt != -1)
	{
		m_stData.eGameState = OS_ATTACK;
		m_stData.nDrt = nDrt;
		PkgObjectActionNot not;
		not.uUserId = m_pTarget->GetID();
		not.uAction = ACTION_ATTACK;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = nDrt;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		m_dwLastAttackTime = m_dwCurrentTime;

		m_pTarget->ReceiveDamage(this, IsMagicAttackMode());
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool MonsterObject::AddSlaveExpr(int _exp)
{
	RECORD_FUNCNAME_WORLD;

	int nInitLevel = GetInitLevel();
	int nMaxLevel = 0;

	if(nInitLevel == 1 ||
		nInitLevel == 0)
	{
		nMaxLevel = 3;
	}
	else if(nInitLevel == 2)
	{
		nMaxLevel = 5;
	}
	else if(nInitLevel == 3)
	{
		nMaxLevel = 7;
	}

	int nNowLevel = GetUpgradeLevel();
	if(nNowLevel >= nMaxLevel)
	{
		return false;
	}
	int nNowExpr = GetSlaveExpr();
	nNowExpr += _exp;

	bool bLevelUp = false;
#ifdef _DEBUG
	if(nNowExpr >= nNowLevel * 100)
#else
	if(nNowExpr >= nNowLevel * 1000)
#endif
	{
		bLevelUp = true;
		SetUpgradeLevel(nNowLevel + 1);
		SetSlaveExpr(0);
	}
	else
	{
		SetSlaveExpr(nNowExpr);
	}

	if(bLevelUp)
	{
		SlaveLevelUp();
	}
	return bLevelUp;
}

void MonsterObject::SlaveLevelUp()
{
	RECORD_FUNCNAME_WORLD;

	SetObject_DC(GetObject_DC() * 1.05f);
	SetObject_MaxDC(GetObject_MaxDC() * 1.15f);

	int nInterval = GetObject_MC();
	nInterval -= 50;
	if(nInterval < 0)
	{
		nInterval = 0;
	}
	SetObject_MC(nInterval);

	nInterval = GetObject_MaxMC();
	nInterval -= 50;
	if(nInterval < 200)
	{
		nInterval = 200;
	}
	SetObject_MaxMC(nInterval);
}

void MonsterObject::OnMonsterDead(HeroObject *_pAttacher, bool _bKilledBySlave)
{
	if(NULL == _pAttacher)
	{
		return;
	}

	HeroObject* pHero = _pAttacher;
	int nFinnalExprMulti = GameWorld::GetInstance().GetFinnalExprMulti();
	if(CMainServer::GetInstance()->GetServerMode() != GM_LOGIN)
	{
		nFinnalExprMulti = 0;
	}

	int nGainExp = 0;
	if(GetLocateScene()->IsTreasureMap())
	{
		if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
		{
			nGainExp = GetObject_Expr() * EXPR_MULTIPLE_2;
		}
		else
		{
			nGainExp = GetObject_Expr() * EXPR_MULTIPLE_6;
		}
	}
	else
	{
		if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
		{
			nGainExp = GetObject_Expr() * EXPR_MULTIPLE_2;
		}
		else
		{
			nGainExp = GetObject_Expr() * EXPR_MULTIPLE_3;
		}
	}

	if(0 != nFinnalExprMulti)
	{
		nGainExp = GetObject_Expr() * nFinnalExprMulti;
	}

	if(pHero->GetObject_Level() > 70)
	{
		nGainExp = GetObject_Expr();
	}

	//	礼花经验加成
	DWORD dwExpFirework = GameWorld::GetInstance().GetExpFireworkTime();
	if(0 != dwExpFirework)
	{
		nGainExp += GetObject_Expr() * 0.5f;
	}

	//	世界经验加成
#ifdef _DEBUG
#else
	if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
#endif
	{
		int nWorldExprMulti = GameWorld::GetInstance().GetExprMultiple();
		if(0 != nWorldExprMulti)
		{
			float fWorldMulti = float(nWorldExprMulti) / 10;
			nGainExp += GetObject_Expr() * fWorldMulti;
		}
	}

	if(!m_bDeadAsSlave)
	{
		pHero->GainExpEx(nGainExp);
		if(!_bKilledBySlave)
		{
			pHero->IncWeaponGrowWithExp(nGainExp);
		}
	}

	lua_getglobal(GetLocateScene()->GetLuaState(), "OnMonsterDead");
	tolua_pushusertype(GetLocateScene()->GetLuaState(), this, "MonsterObject");
	tolua_pushusertype(GetLocateScene()->GetLuaState(), pHero, "HeroObject");

	int nRet = lua_pcall(GetLocateScene()->GetLuaState(), 2, 0, 0);
	if(nRet != 0)
	{
#ifdef _DEBUG
		LOG(ERROR) << "ERROR ON EXECUTE OnMonsterDead : " << lua_tostring(GetLocateScene()->GetLuaState(), -1);
#endif
		lua_pop(GetLocateScene()->GetLuaState(), 1);
	}

	if(IsBoss())
	{
		/*int nBossLevel = GetObject_Level();

		int nRandomSeed = 5;
		if(IsElite())
		{
			nRandomSeed = 3;
		}
		else if(IsLeader())
		{
			nRandomSeed = 2;
		}
		int nRandom = rand() % nRandomSeed;

		if(0 == nRandom)
		{
			if(nBossLevel > 60)
			{
				pHero->AddItem(265);
			}
			else if(nBossLevel > 50)
			{
				pHero->AddItem(264);
			}
			else if(nBossLevel > 40)
			{
				pHero->AddItem(263);
			}
			else
			{
				pHero->AddItem(262);
			}
		}*/
	}

	//
	/*if(IsElite())
	{
		int nRandom = rand() % 7;
		if(nRandom == 0 &&
			!pHeroAttacker->CanActiveDoubleDrop())
		{
			{
				pHeroAttacker->SendQuickMessage(QMSG_DOUBLEDROPINFO, 15);
				pHeroAttacker->ActiveDoubleDrop(60 * 15 * 1000);
				pHeroAttacker->SendStatusInfo();
			}
		}
	}
	else if(IsLeader())
	{
		int nRandom = rand() % 3;
		if(nRandom == 0 &&
			!pHeroAttacker->CanActiveDoubleDrop())
		{
			//pHeroAttacker->SendSystemMessage("获得祝福，双倍爆率20分钟");
			pHeroAttacker->SendQuickMessage(QMSG_DOUBLEDROPINFO, 20);
			pHeroAttacker->ActiveDoubleDrop(60 * 1000 * 20);
			pHeroAttacker->SendStatusInfo();
		}
	}*/
}

void MonsterObject::ProcessDelayAction()
{
	if(m_stData.eGameState == OS_DEAD ||
		m_stData.eGameState == OS_APPEAR ||
		m_stData.eGameState == OS_SHOW)
	{
		//	所有攻击无效
		DelayActionList::iterator it = m_xDelayActions.begin();

		for(it;
			it != m_xDelayActions.end();
			)
		{
			DelayActionBase* pAction = *it;

			if(pAction->uOp == kDelayAction_Attacked)
			{
				//	delete
				delete pAction;
				pAction = NULL;
				it = m_xDelayActions.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	if(m_bSlaveKilledByMaster)
	{
		//	被主人杀死了 则添加必死攻击
		m_bSlaveKilledByMaster = false;
		DelayActionAttacked* pAction = new DelayActionAttacked;
		pAction->nDamage = 0xffff;
		AddDelayAction(pAction);
	}

	DWORD dwTick = GetTickCount();
	DelayActionList::iterator it = m_xDelayActions.begin();

	for(it;
		it != m_xDelayActions.end();
		)
	{
		if(GetUserData()->eGameState == OS_DEAD)
		{
			//	already dead
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

		//	process
		bool bProcessed = true;

		switch(pAction->uOp)
		{
		case kDelayAction_Attacked:
			{
				if(m_stData.eGameState == OS_STOP ||
					m_stData.eGameState == OS_STAND ||
					m_stData.eGameState == OS_STRUCK)
				{
					DelayActionAttacked* pAttackedAction = (DelayActionAttacked*)pAction;

					AttackMsg msg = {0};
					msg.bType = pAttackedAction->nMgcId;
					msg.wDamage = pAttackedAction->nDamage;
					msg.dwAttacker = pAttackedAction->dwAttackerId;
					msg.dwMasks = pAttackedAction->dwAttackMask;
					ParseAttackMsg(&msg);
				}
				else
				{
					bProcessed = false;
				}
			}break;
		case kDelayAction_Heal:
			{
				DelayActionHeal* pHealAction = (DelayActionHeal*)pAction;
				AddHealState(pHealAction->nHealCnt);
			}break;
		case kDelayAction_StaticMagic:
			{
				DelayActionStaticMagic* pStaticMagicAction = (DelayActionStaticMagic*)pAction;
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
		if(bProcessed)
		{
			delete pAction;
			pAction = NULL;
			it = m_xDelayActions.erase(it);
		}
		else
		{
			++it;
		}
	}

	if(GetUserData()->eGameState == OS_DEAD)
	{
		ClearDelayAction();
	}
}