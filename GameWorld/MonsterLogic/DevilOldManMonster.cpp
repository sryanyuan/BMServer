#include "../MonsterTemplateObject.h"
#include "../../GameWorld/GameSceneManager.h"
#include "../../GameWorld/GameWorld.h"
//////////////////////////////////////////////////////////////////////////
DevilOldManMonster::DevilOldManMonster()
{
	m_dwHPRecoverInterval = 180 * 1000;
	m_bCanPosion = false;
}

bool DevilOldManMonster::AttackTarget()
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

		int nDamage = m_pTarget->GetObject_MaxHP() / 6;

		ReceiveDamageInfo di = {0};
		di.bIgnoreAC = true;
		di.bIgnoreMagicShield = true;
		m_pTarget->ReceiveDamage(this, IsMagicAttackMode(), nDamage, 400, &di);
	}
	return true;
}

void DevilOldManMonster::ParseAttackMsg(AttackMsg* _pMsg)
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

	int nDamage = 1;
	m_nLastRecvDamage = nDamage;
	m_nTotalRecvDamage += nDamage;

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

		AddHumDam(pAttacker, _pMsg->wDamage);

		/*if(GetObject_HP() > info.wDamage)*/
		if(GetObject_HP() > nDamage)
		{
			/*DecHP(info.wDamage);*/
			DecHP(nDamage);
			/*m_pValid->DecHP(info.wDamage);*/
			m_pValid->DecHP(nDamage);

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
				m_stData.eGameState = OS_STRUCK;
				/*m_stData.*/m_dwLastStruckTime = m_dwCurrentTime;
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
									unsigned int dwLastsTime = 0;
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
		}
	}

	/*if(m_stData.eGameState == OS_DEAD)
	{
		break;
	}*/
}