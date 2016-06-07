#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
//	Ice savage
IceSavageMonster::IceSavageMonster()
{
	m_dwLastSpellTime = m_dwLastSpellTime2 = 0;
	m_nAttackMode = -1;
}

IceSavageMonster::~IceSavageMonster()
{

}

void IceSavageMonster::DoAction(unsigned int _dwTick)
{
	if(GetUserData()->eGameState == OS_EXT1)
	{
		if(m_dwCurrentTime - m_dwLastSpellTime >= 700)
		{
			m_dwLastSpellTime = m_dwCurrentTime;
			GetUserData()->eGameState = OS_STAND;
		}
	}
	else if(GetUserData()->eGameState == OS_EXT2)
	{
		if(m_dwCurrentTime - m_dwLastSpellTime2 >= 700)
		{
			m_dwLastSpellTime2 = m_dwCurrentTime;
			GetUserData()->eGameState = OS_STAND;
		}
	}
	else
	{
		__super::DoAction(_dwTick);
	}
}

bool IceSavageMonster::CanAttack()
{
	if(m_nAttackMode == -1)
	{
		if(GetObject_HP() > 1500)
		{
			m_nAttackMode = rand() % 5;
		}
		else
		{
			m_nAttackMode = rand() % 3;
		}
	}

	return __super::CanAttack();
}

void IceSavageMonster::AttackHero()
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
		if(m_nAttackMode == 0 ||
			m_nAttackMode == 1)
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

bool IceSavageMonster::AttackTarget()
{
	bool bRet = false;

	if(m_nAttackMode == 0)
	{
		bRet = ExtAttack();	
	}
	else if(m_nAttackMode == 1)
	{
		bRet = ExtAttack2();
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

bool IceSavageMonster::ExtAttack()
{
	//	range attack 3*3
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

		m_pTarget->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 2.0f);

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

bool IceSavageMonster::ExtAttack2()
{
	//	range attack 3*3
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
	int nTargetX = m_pTarget->GetUserData()->wCoordX;
	int nTargetY = m_pTarget->GetUserData()->wCoordY;

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
		m_stData.eGameState = OS_EXT2;
		m_stData.nDrt = nDrt;

		for(int i = -1; i <= 1; ++i)
		{
			for(int j = -1; j <= 1; ++j)
			{
				int nAtkX = (int)GetUserData()->wCoordX + i;
				int nAtkY = (int)GetUserData()->wCoordY + j;

				MapCellInfo* pCellInfo = GetLocateScene()->GetMapData(nAtkX, nAtkY);
				if(pCellInfo)
				{
					if(pCellInfo->pCellObjects != NULL)
					{
						CELLDATALIST::const_iterator begiter = pCellInfo->pCellObjects->begin();
						for(begiter;
							begiter != pCellInfo->pCellObjects->end();
							++begiter)
						{
							if((*begiter)->bType == CELL_MOVEOBJECT)
							{
								GameObject* pObj = (GameObject*)(*begiter)->pData;
								if(pObj->GetType() == SOT_MONSTER)
								{
									MonsterObject* pMons = static_cast<MonsterObject*>(pObj);
									if(pMons->GetMaster() != NULL)
									{
										pMons->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 1.8f);
									}
								}
								else if(pObj->GetType() == SOT_HERO)
								{
									pObj->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 1.8f);
								}
							}
						}
					}
				}
			}
		}

		PkgObjectActionNot not;
		//not.uUserId = m_pTarget->GetID();
		not.uAction = ACTION_EXT2;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = MAKELONG(0, nDrt);
		//not.uParam2 = 2;
		//not.uParam3 = MAKE_POSITION_DWORD(m_pTarget);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		/*m_stData.*/m_dwLastSpellTime2 = m_dwCurrentTime;

		//	Erase this target
		/*if(bIsDead &&
			m_pTarget->GetType() == SOT_HERO)
		{
			m_pTarget->GetLocateScene()->EraseTarget(m_pTarget);
		}*/
	}
	return true;
}