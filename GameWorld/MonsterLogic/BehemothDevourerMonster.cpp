#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
BehemothDevourerMonster::BehemothDevourerMonster()
{
	m_dwLastSpellTime = 0;
	m_nAttackMode = -1;
	m_dwLastAttackTime = 0;
	m_dwLastSpeAtk2 = 0;
	m_dwLastSpeAtk3 = 0;
}

void BehemothDevourerMonster::DoAction(unsigned int _dwTick)
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
		if(m_dwCurrentTime - m_dwLastSpeAtk2 >= 800)
		{
			m_dwLastSpeAtk2 = m_dwCurrentTime;
			GetUserData()->eGameState = OS_STAND;
		}
	}
	else if(GetUserData()->eGameState == OS_EXT3)
	{
		if(m_dwCurrentTime - m_dwLastSpeAtk3 >= 800)
		{
			m_dwLastSpeAtk3 = m_dwCurrentTime;
			GetUserData()->eGameState = OS_STAND;
		}
	}
	else
	{
		__super::DoAction(_dwTick);
	}
}

bool BehemothDevourerMonster::CanAttack()
{
	//	0:spe1 1:spe2 2:spe3 other:normal
	if(m_nAttackMode == -1)
	{
		if(GetObject_HP() > 15000)
		{
			m_nAttackMode = rand() % 10;
		}
		else
		{
			m_nAttackMode = rand() % 5;
		}
	}

	if(GetTickCount() - m_dwLastActualAttackTime > 8 * 1000)
	{
		//	long distance attack
		m_nAttackMode = 0;
	}

	if(0 == m_nAttackMode)
	{
		return CanAttackExt();
	}
	else if(2 == m_nAttackMode)
	{
		return CanAttackExt3();
	}

	return __super::CanAttack();
}

bool BehemothDevourerMonster::CanAttackExt()
{
	if(m_pTarget == NULL)
	{
		return false;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

	if(abs(nOftX) > 8 ||
		abs(nOftY) > 8)
	{
		return false;
	}

	/*if(nOftY == 0 &&
		nOftX == 0)
	{
		return false;
	}*/

	if(nOftX == 0 ||
		nOftY == 0)
	{
		return true;
	}

	return true;
}

bool BehemothDevourerMonster::CanAttackExt3()
{
	if(m_pTarget == NULL)
	{
		return false;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

	if(abs(nOftX) > 3 ||
		abs(nOftY) > 3)
	{
		return false;
	}

	/*if(nOftY == 0 &&
		nOftX == 0)
	{
		return false;
	}*/

	if(nOftX == 0 ||
		nOftY == 0)
	{
		return true;
	}

	return true;
}

void BehemothDevourerMonster::AttackHero()
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
		else if(m_nAttackMode == 1)
		{
			if(m_dwCurrentTime - m_dwLastSpeAtk2 > 1200)
			{
				AttackTarget();
			}
		}
		else if(m_nAttackMode == 2)
		{
			if(m_dwCurrentTime - m_dwLastSpeAtk3 > 1200)
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

bool BehemothDevourerMonster::AttackTarget()
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
	else if(m_nAttackMode == 2)
	{
		bRet = ExtAttack3();
	}
	else
	{
		bRet = __super::AttackTarget();
	}

	if(bRet)
	{
		m_nAttackMode = -1;
		m_dwLastActualAttackTime = GetTickCount();
	}

	return bRet;
}

bool BehemothDevourerMonster::ExtAttack()
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

		PkgObjectActionNot not;
		//not.uUserId = m_pTarget->GetID();
		not.uAction = ACTION_EXT1;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = MAKELONG(0, nDrt);
		not.uUserId = m_pTarget->GetID();
		//not.uParam2 = 2;
		//not.uParam3 = MAKE_POSITION_DWORD(m_pTarget);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		//int nDamage = m_pTarget->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 1.8f);

		GameObjectList xObjs;
		GetLocateScene()->GetMapObjects(m_pTarget->GetUserData()->wCoordX, m_pTarget->GetUserData()->wCoordY, 2, xObjs);
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
						pObj->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 1.8f);
					}
				}
			}
		}

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

bool BehemothDevourerMonster::ExtAttack2()
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
		m_stData.eGameState = OS_EXT2;
		m_stData.nDrt = nDrt;

		PkgObjectActionNot not;
		//not.uUserId = m_pTarget->GetID();
		not.uAction = ACTION_EXT2;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = MAKELONG(0, nDrt);
		not.uUserId = m_pTarget->GetID();
		//not.uParam2 = 2;
		//not.uParam3 = MAKE_POSITION_DWORD(m_pTarget);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		/*m_stData.*/m_dwLastSpeAtk2 = m_dwCurrentTime;

		int nDamage = m_pTarget->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 2.8f);

		int nPreHP = GetObject_HP();
		IncHP(nDamage / 2);
		GetValidCheck()->IncHP(nDamage / 2);
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

		//	Erase this target
		/*if(bIsDead &&
			m_pTarget->GetType() == SOT_HERO)
		{
			m_pTarget->GetLocateScene()->EraseTarget(m_pTarget);
		}*/
	}
	return true;
}

bool BehemothDevourerMonster::ExtAttack3()
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
		m_stData.eGameState = OS_EXT3;
		m_stData.nDrt = nDrt;

		PkgObjectActionNot not;
		//not.uUserId = m_pTarget->GetID();
		not.uAction = ACTION_EXT3;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = MAKELONG(0, nDrt);
		not.uUserId = m_pTarget->GetID();
		//not.uParam2 = 2;
		//not.uParam3 = MAKE_POSITION_DWORD(m_pTarget);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		/*m_stData.*/m_dwLastSpeAtk3 = m_dwCurrentTime;

		int nDamage = m_pTarget->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 3.5f);

		GameObjectList xObjs;
		GetLocateScene()->GetMapObjects(GetUserData()->wCoordX, GetUserData()->wCoordY, 2, xObjs);
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
					pObj != this &&
					pObj != m_pTarget)
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
						pObj->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 3.5f);
					}
				}
			}
		}

		int nPreHP = GetObject_HP();
		IncHP(nDamage * 3);
		GetValidCheck()->IncHP(nDamage * 3);
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

		//	Erase this target
		/*if(bIsDead &&
			m_pTarget->GetType() == SOT_HERO)
		{
			m_pTarget->GetLocateScene()->EraseTarget(m_pTarget);
		}*/
	}

	//	call monsters
	int nMovedNumber = GetLocateScene()->MoveSomeMonsterTo(123, 3, GetUserData()->wCoordX, GetUserData()->wCoordY);
	if(nMovedNumber < 3)
	{
		int nCallNumber = 3 - nMovedNumber;
		int nPosX = 0;
		int nPosY = 0;
		int nCallCounter = 0;

		for(int i = 0; i < 8; ++i)
		{
			nPosX = m_stData.wCoordX + 2 * g_nMoveOft[2 * i];
			nPosY = m_stData.wCoordY + 2 * g_nMoveOft[2 * i + 1];

			if(GetLocateScene()->CanThrough(nPosX, nPosY))
			{
				GetLocateScene()->CreateMonster(123, nPosX, nPosY);
				++nCallCounter;
			}

			if(nCallCounter >= nCallNumber)
			{
				break;
			}
		}
	}

	return true;
}