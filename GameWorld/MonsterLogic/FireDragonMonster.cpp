#include "../MonsterTemplateObject.h"
#include "../../GameWorld/GameSceneManager.h"
#include "../../GameWorld/GameDbBuffer.h"
#include "../../GameWorld/GameWorld.h"
//////////////////////////////////////////////////////////////////////////
FireDragonMonster::FireDragonMonster()
{
	AddLogicItem(0, ACTION_EXT1, 50, 600, 1200);
	//	三条完整的生命
	m_nDeadTimes = 0;

	m_dwHPRecoverInterval = 20 * 1000;
	m_bCanStruck = false;
	m_nLifeTimes = 3;
}

bool FireDragonMonster::LogicAttackTarget(const MonsLogicItem* _pLogic)
{
	if(_pLogic->m_nLogicID == 0)
	{
		return ExtAttack0();
	}

	return false;
}

int FireDragonMonster::GetRandomAbility(ABILITY_TYPE _type)
{
	int nAbility = __super::GetRandomAbility(_type);

	if(m_nDeadTimes >= 1)
	{
		if(_type == AT_AC)
		{
			nAbility *= 1.5f;
		}
	}
	if(m_nDeadTimes >= 2)
	{
		if(_type == AT_MAC)
		{
			nAbility *= 1.5f;
		}
	}
	if(m_nDeadTimes >= 3)
	{
		if(_type == AT_DC)
		{
			nAbility *= 1.5f;
		}
	}

	return nAbility;
}

bool FireDragonMonster::CanAttack()
{
	if(m_pTarget == NULL)
	{
		return false;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

	if(abs(nOftX) <= GetViewRange() &&
		abs(nOftY) <= GetViewRange())
	{
		return true;
	}

	return false;
}

bool FireDragonMonster::AttackTarget()
{
	if(ATTACK_NORMAL == m_eAttackMode)
	{
		bool bRet = NormalAttack();
		m_nExecuteLogicIndex = -1;
		m_eAttackMode = ATTACK_NONE;
	}
	else if(ATTACK_SPECIAL == m_eAttackMode)
	{
		MonsLogicItem* pLogic = GetCurrentLogic();
		if(NULL == pLogic)
		{
			m_eAttackMode = ATTACK_NONE;
			m_nExecuteLogicIndex = -1;
			return false;
		}
		else
		{
			if(LogicAttackTarget(pLogic))
			{
				pLogic->m_dwLastExecuteTime = m_dwCurrentTime;
				GetUserData()->eGameState = OS_EXT1;

				//	攻击成功 重置(OS_EXT1状态下不会继续调用)
				//m_eAttackMode = ATTACK_NONE;
				//m_nExecuteLogicIndex = -1;

				return true;
			}
			else
			{
				m_eAttackMode = ATTACK_NONE;
				m_nExecuteLogicIndex = -1;
				return false;
			}
		}
	}

	return true;
}

bool FireDragonMonster::NormalAttack()
{
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
		if(nSx == g_nMoveOft[i * 2] &&
			nSy == g_nMoveOft[i * 2 + 1])
		{
			nDrt = i;
			break;
		}
	}

	//bool bReqStuck = true;

	int nRandom = rand() % 10;

	if(nDrt == -1)
	{
		nDrt = 0;
	}

	if(nDrt != -1)
	{
		m_stData.eGameState = OS_ATTACK;
		m_stData.nDrt = nDrt;
		/*
		PkgObjectActionNot not;
				not.uUserId = m_pTarget->GetID();
				not.uAction = ACTION_ATTACK;
				not.uTargetId = GetID();
				not.uParam0 = MAKE_POSITION_DWORD(this);
				not.uParam1 = nDrt;*/
		PkgMonsterFireDragonAttack0Not not;
		not.uTargetId = GetID();
		not.uUserId = m_pTarget->GetID();
		not.dwPos = MAKE_POSITION_DWORD(this);
		not.dwExplosionPos = MAKE_POSITION_DWORD(m_pTarget);
		not.nDrt = nDrt;
		not.dwLastTime = 5000;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		m_dwLastAttackTime = m_dwCurrentTime;

		//m_pTarget->ReceiveDamage(this, IsMagicAttackMode());
		unsigned int dwTargetX = m_pTarget->GetCoordX();
		unsigned int dwTargetY = m_pTarget->GetCoordY();
		m_pTarget->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.3f);

		//	释放持续性魔法
		StaticMagic sm = {0};
		sm.dwFire = GetID();
		sm.sPosX = dwTargetX;
		sm.sPoxY = dwTargetY;
		sm.wMaxDC = GetObject_MaxDC() * 2.5f;
		sm.wMinDC = GetObject_DC() * 2.5f;
		sm.dwEffectActive = 1;
		sm.dwEnableTime = GetTickCount() + 800;
		sm.dwExpire = GetTickCount() + 800 + not.dwLastTime;
		sm.wMgcID = MEFF_FIREDRAGON_FIREWALL;
		sm.pFire = this;
		GetLocateScene()->PushStaticMagic(&sm);
	}
	return true;
}

bool FireDragonMonster::ExtAttack0()
{
	const MonsLogicItem* pLogic = GetLogicByID(0);

	if(m_pTarget == NULL)
	{
		return false;
	}
	if(m_stData.eGameState == OS_DEAD)
	{
		return false;
	}
	//	Range attack
	RECT rcAttackRage;
	rcAttackRage.left = (int)m_stData.wCoordX - 10;
	rcAttackRage.right = (int)m_stData.wCoordX + 10;
	rcAttackRage.top = (int)m_stData.wCoordY - 10;
	rcAttackRage.bottom = (int)m_stData.wCoordY + 10;

	if(rcAttackRage.left < 0)
	{
		rcAttackRage.left = 0;
	}
	if(rcAttackRage.right >= GetLocateScene()->GetMapInfo().nCol)
	{
		rcAttackRage.right = GetLocateScene()->GetMapInfo().nCol - 1;
	}
	if(rcAttackRage.top < 0)
	{
		rcAttackRage.top = 0;
	}
	if(rcAttackRage.bottom >= GetLocateScene()->GetMapInfo().nRow)
	{
		rcAttackRage.right = GetLocateScene()->GetMapInfo().nRow - 1;
	}

	MapCellInfo* pCell = NULL;
	CELLDATALIST* pList = NULL;
	HeroObject* pHero = NULL;
	GameObject* pObj = NULL;
	MonsterObject* pMonster = NULL;
	const UserMagic* pMgc = NULL;

	int nDamage = 0;
	int nDC = 0;
	int nAC = 0;
	bool bReqStruck = true;
	bool bMissed = false;

	PkgObjectActionNot not;
	PkgPlayerUpdateAttribNtf ntf;
	PkgPlayerPlayAniAck aniack;
	aniack.uUserId = 1;

	for(int x = rcAttackRage.left; x <= rcAttackRage.right; ++x)
	{
		for(int y = rcAttackRage.top; y <= rcAttackRage.bottom; ++y)
		{
			pCell = GetLocateScene()->GetMapData(x, y);
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
						bReqStruck = true;
						bMissed = false;

						if((*begiter)->bType == CELL_MOVEOBJECT)
						{
							pObj = (GameObject*)(*begiter)->pData;
							if(pObj->GetType() == SOT_HERO &&
								pObj->GetUserData()->eGameState != OS_DEAD)
							{
								pObj->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.5f);
								aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX - 1, pObj->GetUserData()->wCoordY - 1));
							}
							else if(pObj->GetType() == SOT_MONSTER)
							{
								MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
								if(pMonster != this &&
									pMonster->GetMaster() != NULL &&
									pMonster->GetUserData()->eGameState != OS_DEAD)
								{
									pMonster->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.5f);
									aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX - 1, pObj->GetUserData()->wCoordY - 1));
								}
							}
						}
					}
				}
			}
		}
	}
	m_stData.eGameState = OS_ATTACK;
	m_dwLastAttackTime = GetTickCount();
	not.uAction = ACTION_EXT1;
	not.uParam0 = MAKE_POSITION_DWORD(this);
	not.uTargetId = GetID();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

	aniack.uTargetId = GetID();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << aniack;
	GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	return true;
}

/*void FireDragonMonster::ParseAttackMsg(AttackMsg* _pMsg)
{
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
		if(GetObject_HP() > _pMsg->wDamage)
		{
			DecHP(_pMsg->wDamage);
			m_pValid->DecHP(_pMsg->wDamage);

			not.uParam2 = GetObject_HP();
			if(pAttacker != NULL)
			{
				not.uParam3 = 0;

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
							not.uParam3 = 1;
						}
					}
				}
			}

			if(m_stData.eGameState != OS_STRUCK)
			{
				//m_stData.eGameState = OS_STRUCK;
				//m_dwLastStruckTime = m_dwCurrentTime;
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
			//	看看死了几次了
			++m_nDeadTimes;
			if(m_nDeadTimes <= 3)
			{
				//	回血
				ItemAttrib atb;
				if(GetRecordInMonsterTable(GetObject_ID(), &atb))
				{
					SetObject_HP(GetObject_MaxHP());
					m_pValid->SetHP(GetObject_MaxHP());

					not.uParam2 = GetObject_HP();
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << not;
					pScene->BroadcastPacket(&g_xThreadBuffer);

					//	发送消息
					if(1 == m_nDeadTimes)
					{
						Say("人类只是小蝼蚁而已,想打败我?哈哈哈");
					}
					else if(2 == m_nDeadTimes)
					{
						Say("你们真的惹我生气了");
					}
					else if(3 == m_nDeadTimes)
					{
						Say("让你们看看我的真面目");
					}

					return;
				}
			}

			Say("我的灵魂是永生的,等着吧...");

			SetObject_HP(0);
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

			m_dwLastDeadTime = m_dwCurrentTime;
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
				if(m_pMaster->GetType() == SOT_HERO)
				{
					static_cast<HeroObject*>(m_pMaster)->ClearSlave(this);
				}
				SetMaster(NULL);
			}

			HeroObject* pHeroAttacker = NULL;
			bool bKilledBySlave = false;
			int nFinnalExprMulti = GameWorld::GetInstance().GetFinnalExprMulti();

			if(pAttacker != NULL &&
				GetObject_HP() == 0 &&
				m_stData.eGameState == OS_DEAD)
			{
				if(pAttacker->GetType() == SOT_HERO)
				{
					HeroObject* pHero = static_cast<HeroObject*>(pAttacker);
					pHeroAttacker = pHero;
				}
				else if(pAttacker->GetType() == SOT_MONSTER &&
					GetObject_HP() == 0 &&
					m_stData.eGameState == OS_DEAD)
				{
					MonsterObject* pSlave = static_cast<MonsterObject*>(pAttacker);
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
		}
	}
}*/
void FireDragonMonster::OnDeadRevive(int _nLeftLife)
{
	++m_nDeadTimes;

	if(0 == _nLeftLife)
	{
		Say("让你们看看我的真面目");
	}
	else if(1 == _nLeftLife)
	{
		Say("你们真的惹我生气了");
	}
	else if(2 == _nLeftLife)
	{
		Say("人类只是小蝼蚁而已,想打败我?哈哈哈");
	}
}

void FireDragonMonster::OnMonsterDead(HeroObject* _pAttacher, bool _bKilledBySlave)
{
	Say("我的灵魂是永生的,等着吧...");
	__super::OnMonsterDead(_pAttacher, _bKilledBySlave);
}