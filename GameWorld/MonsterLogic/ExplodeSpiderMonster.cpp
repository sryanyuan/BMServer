#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
void ExplodeSpiderMonster::DoAction(unsigned int _dwTick)
{
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
			}
		}break;
	case OS_DEAD:
		{
			//
			if(m_dwCurrentTime - m_dwLastDeadTime >= 10 * 1000)
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

bool ExplodeSpiderMonster::AttackTarget()
{
	if(NULL == m_pTarget)
	{
		return true;
	}
	if(m_pTarget->GetUserData()->eGameState == OS_DEAD ||
		m_pTarget->GetObject_HP() == 0)
	{
		m_pTarget = NULL;
		return true;
	}

	/*int nDamage = 0;
	int nDC = 0;
	int nAC = 0;*/
	PkgObjectActionNot not;
	PkgPlayerUpdateAttribNtf ntf;

	/*nDC = GetRandomAbility(AT_DC);
	nAC = m_pTarget->GetRandomAbility(AT_AC);
	nDamage = nDC - nAC;*/

	if(m_pTarget->GetType() == SOT_MONSTER ||
		m_pTarget->GetType() == SOT_HERO)
	{
		m_pTarget->ReceiveDamage(this, IsMagicAttackMode());
	}
	
/*
	if(m_pTarget->GetType() == SOT_HERO)
	{
		HeroObject* pHero = static_cast<HeroObject*>(m_pTarget);
		if(pHero->GetHeroJob() == 1)
		{
			if(pHero->IsEffectExist(MEFF_SHIELD))
			{
				int nDefence = 0;
				nDefence = pHero->GetSheildDefence(nDamage);
				pHero->ProcessSheild(nDefence);
				nDamage -= nDefence;
			}
		}

		if(nDamage > 0)
		{
			bool bReqStruck = true;

			bReqStruck = pHero->CanStruck(nDamage);

			if(!pHero->DecHP(nDamage))
			{
				//	Dead
				not.uTargetId = pHero->GetID();
				not.uAction = ACTION_DEAD;
				not.uParam0 = MAKE_POSITION_DWORD(pHero);
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << not;
				pHero->GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

				pHero->GetUserData()->eGameState = OS_DEAD;
				pHero->KillAllSlave();
				pHero->GetLocateScene()->EraseTarget(pHero);
				pHero->ResetSupply();
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

			if(bReqStruck)
			{
				DelaySendInfo info;
				info.uDelayTime = m_dwCurrentTime + 800;
				info.uOp = DELAY_HEROSTRUCK;
				info.uParam[0] = pHero->GetID();
				pHero->GetLocateScene()->PushDelayBuf(info);
			}
		}
	}*/

	not.uTargetId = GetID();
	not.uAction = ACTION_DEAD;
	not.uParam0 = MAKE_POSITION_DWORD(this);
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	m_stData.eGameState = OS_DEAD;
	SetObject_HP(0);
	m_pValid->SetHP(0);
	SetTarget(NULL);
	m_dwLastDeadTime = GetTickCount();

	GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	return true;
}