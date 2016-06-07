#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
//	ManWormMonster
ManWormMonster::ManWormMonster()
{
	m_dwLastSpellTime = 0;
	m_nAttackMode = -1;
}

ManWormMonster::~ManWormMonster()
{

}

void ManWormMonster::DoAction(unsigned int _dwTick)
{
	if(GetUserData()->eGameState == OS_EXT1)
	{
		if(m_dwCurrentTime - m_dwLastSpellTime >= 700)
		{
			m_dwLastSpellTime = m_dwCurrentTime;
			GetUserData()->eGameState = OS_STAND;
		}
	}
	else
	{
		__super::DoAction(_dwTick);
	}
}

bool ManWormMonster::CanAttack()
{
	if(m_nAttackMode == -1)
	{
		if(GetObject_HP() > 2500)
		{
			m_nAttackMode = rand() % 5;
		}
		else
		{
			m_nAttackMode = rand() % 2;
		}
	}

	return __super::CanAttack();
}

void ManWormMonster::AttackHero()
{
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
		if(m_nAttackMode == 0)
		{
			if(m_dwCurrentTime - /*m_stData.*/m_dwLastSpellTime > 800)
			{
				AttackTarget();
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
}

bool ManWormMonster::AttackTarget()
{
	bool bRet = false;

	if(m_nAttackMode == 0)
	{
		bRet = ExtAttack();	
	}
	else
	{
		bRet = __super::AttackTarget();
	}

	if(bRet)
	{
		m_nAttackMode = -1;
	}

	return bRet;
}

bool ManWormMonster::ExtAttack()
{
	//	super attack
	if(NULL == m_pTarget)
	{
		return false;
	}
	if(m_stData.eGameState != OS_STAND)
	{
		return false;
	}
	/*if(m_pTarget->GetType() == SOT_HERO)
	{
		return false;
	}*/
	if(m_pTarget->GetObject_HP() == 0 &&
		m_pTarget->GetUserData()->eGameState == OS_DEAD)
	{
		m_pTarget = NULL;
		return true;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;
	//int nTargetX = m_pTarget->GetUserData()->wCoordX;
	//int nTargetY = m_pTarget->GetUserData()->wCoordY;

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

	if(nDrt != -1)
	{
		m_stData.eGameState = OS_EXT1;
		m_stData.nDrt = nDrt;

		int nDamage = m_pTarget->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 2.0f);
		if(nDamage > 0 &&
			GetObject_ID() == 120)
		{
			int nPreHP = GetObject_HP();
			IncHP(nDamage);
			GetValidCheck()->IncHP(nDamage);
			if(nPreHP != GetObject_HP())
			{
				PkgPlayerUpdateAttribNtf ntf;
				ntf.bType = UPDATE_HP;
				ntf.dwParam = GetObject_HP();
				ntf.uTargetId = GetID();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ntf;
				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			}
		}

		PkgObjectActionNot not;
		//not.uUserId = m_pTarget->GetID();
		not.uAction = ACTION_EXT1;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = MAKELONG(0, nDrt);
		//not.uParam2 = 2;
		//not.uParam3 = MAKE_POSITION_DWORD(m_pTarget);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		/*m_stData.*/m_dwLastSpellTime = m_dwCurrentTime;

		//	Erase this target
		/*if(bIsDead &&
			m_pTarget->GetType() == SOT_HERO)
		{
			m_pTarget->GetLocateScene()->EraseTarget(m_pTarget);
		}*/
	}
	return true;
}