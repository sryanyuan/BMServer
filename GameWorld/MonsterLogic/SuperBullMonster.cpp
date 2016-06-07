#include "../MonsterTemplateObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
SuperBullKing::SuperBullKing()
{
	AddLogicItem(0, ACTION_EXT1, 40, 300);
	m_dwLastFlyTime = 0;
}

bool SuperBullKing::LogicCanAttack(const MonsLogicItem *_pLogic)
{
	if(0 == _pLogic->m_nLogicID)
	{
		if(m_pTarget == NULL)
		{
			return false;
		}

		int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
		int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

		if(abs(nOftX) < 7 &&
			abs(nOftY) < 7)
		{
			return true;
		}

		return false;
	}

	return false;
}

bool SuperBullKing::LogicAttackTarget(const MonsLogicItem *_pLogic)
{
	if(0 == _pLogic->m_nLogicID)
	{
		return ExtAttack0();
	}

	return false;
}

bool SuperBullKing::ExtAttack0()
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

	nDrt = GetDirectionByMoveOffset(nSx, nSy);

	bool bReqStuck = true;

	if(nDrt != -1)
	{
		//m_stData.eGameState = OS_ATTACK;
		m_stData.nDrt = nDrt;
		PkgObjectActionNot not;
		not.uUserId = m_pTarget->GetID();
		not.uAction = ACTION_EXT1;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = MAKELONG(0, nDrt);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		m_dwLastAttackTime = m_dwCurrentTime;

		GameObjectList xObjs;
		GetLocateScene()->GetMapObjects(m_pTarget->GetUserData()->wCoordX, m_pTarget->GetUserData()->wCoordY, 3, xObjs);
		if(!xObjs.empty())
		{
			GameObjectList::iterator begIter = xObjs.begin();
			GameObjectList::iterator endIter = xObjs.end();

			for(begIter;
				begIter != endIter;
				++begIter)
			{
				GameObject* pObj = *begIter;

				if(pObj &&
					pObj != this)
				{
					bool bRecvDamage = false;
					if(pObj->GetType() == SOT_HERO)
					{
						bRecvDamage = true;
					}
					else if(pObj->GetType() == SOT_MONSTER)
					{
						MonsterObject* pMons = (MonsterObject*)pObj;
						if(pMons->GetMaster() != NULL)
						{
							bRecvDamage = true;
						}
					}

					if(bRecvDamage)
					{
						//pObj->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 1.8f);
						if(pObj->GetType() == SOT_HERO)
						{
							HeroObject* pHero = static_cast<HeroObject*>(pObj);
							int nMPDec = pObj->GetObject_MP();
							nMPDec /= 3;

							pHero->DecMP(nMPDec);
							pHero->GetValidCheck()->DecMP(nMPDec);
							pHero->SyncMP(pHero);

							//	dec hp
							int nMaxHP = pHero->GetObject_MaxHP();

							/*pHero->DecHP(nMPDec);
							pHero->GetValidCheck()->DecHP(nMPDec);
							pHero->SyncHP(pHero);*/
							pHero->ReceiveDamage(this, true, nMaxHP / 6 + GetRandomAbility(AT_DC));
						}
						else if(pObj->GetType() == SOT_MONSTER)
						{
							int nDamage = pObj->GetObject_MaxHP() / 15;
							pObj->ReceiveDamage(this, true, nDamage);
						}
					}
				}
			}
		}
	}
	return true;
}

bool SuperBullKing::FreeUpdate()
{
	if(GetObject_HP() > GetObject_MaxHP() / 2)
	{
		return true;
	}

	if(NULL == m_pTarget)
	{
		return true;
	}

	if(GetTickCount() - m_dwLastFlyTime > 12 * 1000)
	{
		int nTargetX = m_pTarget->GetCoordX();
		int nTargetY = m_pTarget->GetCoordY();
		int nFlyX, nFlyY = 0; 

		if(GetLocateScene()->GetAroundCanMove(nTargetX, nTargetY, &nFlyX, &nFlyY))
		{
			FlyTo(nFlyX, nFlyY);
			m_dwLastFlyTime = GetTickCount();
			return false;
		}
	}

	return true;
}

int SuperBullKing::GetRandomAbility(ABILITY_TYPE _type)
{
	if(GetObject_HP() > GetObject_MaxHP() / 2)
	{
		return __super::GetRandomAbility(_type);
	}

	int nValue = __super::GetRandomAbility(_type);

	if(_type == AT_AC)
	{
		nValue += 50;
	}
	else if(_type == AT_MAC)
	{
		nValue += 100;
	}
	else if(_type == AT_DC)
	{
		nValue *= 1.5f;
	}
	
	return nValue;
}

int SuperBullKing::ReceiveDamage(GameObject* _pAttacker, bool _bMgcAtk, int _oriDC /* = 0 */, int _nDelay /* = 350 */, const ReceiveDamageInfo* _pInfo /* = NULL */)
{
	/*int nDC = _pAttacker->GetRandomAbility(AT_DC);
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

	int nDamage = nDC - nAC;

	if(GetObject_HP() < GetObject_MaxHP() / 2)
	{
		nDamage *= 0.7f;
	}

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
		SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
		pMsg->uOp = DELAY_HEROATTACK;
		pMsg->uParam[0] = GetID();
		pMsg->uParam[1] = nDamage;
		pMsg->uParam[2] = _pAttacker->GetID();
		pMsg->uParam[3] = 0;
		pMsg->dwDelayTime = GetTickCount() + _nDelay;
		GetLocateScene()->PushDelayBuf(pMsg);
	}
	else if(_pAttacker->GetType() == SOT_HERO)
	{
		SceneDelayMsg* pMsg = FreeListManager::GetInstance()->GetFreeSceneDelayMsg();
		pMsg->uOp = DELAY_HEROATTACK;
		pMsg->uParam[0] = GetID();
		pMsg->uParam[1] = nDamage;
		pMsg->uParam[2] = _pAttacker->GetID();
		pMsg->uParam[3] = 0;
		pMsg->dwDelayTime = GetTickCount() + _nDelay;
		//	是否是暴击
		HeroObject* pHero = (HeroObject*)_pAttacker;
		if(pHero->IsLastAttackCritical())
		{
			SET_FLAG(pMsg->uParam[5], ATTACKMSG_MASK_CRITICAL);
		}

		//	是否是魔法攻击
		if(_bMgcAtk)
		{
			pMsg->uParam[3] = pHero->GetLastUseMagicID();
		}

		GetLocateScene()->PushDelayBuf(pMsg);
	}

	return nDamage;*/
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
	if(GetObject_HP() < GetObject_MaxHP() / 2)
	{
		nDamage *= 0.7f;
	}
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