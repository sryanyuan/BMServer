#include "../MonsterTemplateObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
MoonWarLordMonster::MoonWarLordMonster()
{
	m_dwLastDefenceTime = 0;
	m_dwLastFlyTime = 0;
	m_dwLastSummonTime = 0;
	m_dwLastAttackHeroTime = 0;

	AddLogicItem(0, ACTION_EXT1, 30, 600, 900);
	AddLogicItem(1, ACTION_EXT2, 20, 500);
	AddLogicItem(2, ACTION_EXT3, 5, 300);
	//AddLogicItem(3, ACTION_EXT4, 20, 300);
}

unsigned int MoonWarLordMonster::GetAttackCostTime()
{
	int nAtkFrame = 10;
	unsigned int dwCost = MAX_ATTACK_INTERVAL;
	dwCost = (MAX_ATTACK_INTERVAL - GetObject_AtkSpeed() * 5) * (nAtkFrame + 2) + 50;
	if(dwCost > MAX_ATTACK_INTERVAL)
	{
		//dwCost = MAX_ATTACK_INTERVAL;
	}
	return dwCost;
}

int MoonWarLordMonster::GetRandomAbility(ABILITY_TYPE _type)
{
	if(_type == AT_AC ||
		_type == AT_MAC)
	{
		if(m_dwLastDefenceTime != 0)
		{
			return __super::GetRandomAbility(_type) + 160;
		}
	}
	return __super::GetRandomAbility(_type);
}

void MoonWarLordMonster::DoAction(unsigned int _dwTick)
{
	__super::DoAction(_dwTick);

	if(m_dwCurrentTime > m_dwLastDefenceTime)
	{
		m_dwLastDefenceTime = 0;
	}

	if(GetObject_HP() < GetObject_MaxHP() / 2)
	{
		GetLogicByID(0)->m_nProbability = 50;
		GetLogicByID(1)->m_nProbability = 35;
		GetLogicByID(2)->m_nProbability = 12;
	}
}

void MoonWarLordMonster::SelectAttackMode()
{
	bool bIgnoreExt2 = false;
	if(m_dwLastDefenceTime != 0)
	{
		bIgnoreExt2 = true;
	}

	if(ATTACK_NONE == m_eAttackMode)
	{
		m_eAttackMode = ATTACK_NORMAL;

		int nRandom = rand() % 100;
		int nExecuteIndex = -1;
		int nExecuteCounter = 0;

		for(int i = 0; i < m_xMonsLogicList.size(); ++i)
		{
			if(bIgnoreExt2 &&
				i == 2)
			{
				continue;
			}

			int nProb = m_xMonsLogicList[i].m_nProbability;

			if(nProb != 0)
			{
				int nPreCounter = nExecuteCounter;
				nExecuteCounter += m_xMonsLogicList[i].m_nProbability;

				if(nRandom > nPreCounter &&
					nRandom <= nExecuteCounter)
				{
					nExecuteIndex = i;
					break;
				}
			}
		}

		if(nExecuteIndex != -1)
		{
			m_nExecuteLogicIndex = nExecuteIndex;
			m_eAttackMode = ATTACK_SPECIAL;
		}
	}
}

void MoonWarLordMonster::AttackHero()
{
	//	fly
	if(m_dwCurrentTime - m_dwLastFlyTime > 20 * 1000)
	{
		if(m_pTarget &&
			m_dwCurrentTime - m_dwLastAttackHeroTime > 12 * 1000)
		{
			int nPosX = m_pTarget->GetCoordX();
			int nPosY = m_pTarget->GetCoordY();

			if(m_pTarget->GetType() == SOT_MONSTER)
			{
				GameObject* pMaster = static_cast<MonsterObject*>(m_pTarget)->GetMaster();
				if(pMaster)
				{
					nPosX = pMaster->GetCoordX();
					nPosY = pMaster->GetCoordY();
					SetTarget(pMaster);
				}
			}

			for(int i = 0; i < 8; ++i)
			{
				int nFlyPosX = nPosX + g_nMoveOft[i * 2];
				int nFlyPosY = nPosY + g_nMoveOft[i * 2 + 1];

				if(GetLocateScene()->CanMove(nFlyPosX, nFlyPosY))
				{
					FlyTo(nFlyPosX, nFlyPosY);
					m_dwLastFlyTime = GetTickCount();
					break;
				}
			}
		}
	}

	//	change attack mode
	if(m_dwCurrentTime - m_dwLastRecoverTime > 10 * 1000)
	{
		//	attack mode 1
		m_eAttackMode = ATTACK_SPECIAL;
		m_nExecuteLogicIndex = 1;
	}

	//	summon
	//	call monsters
	if(m_dwCurrentTime - m_dwLastSummonTime > 60 * 1000)
	{
		int nMovedNumber = GetLocateScene()->MoveSomeMonsterTo(134, 3, GetUserData()->wCoordX, GetUserData()->wCoordY);
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
					GetLocateScene()->CreateMonster(134, nPosX, nPosY);
					++nCallCounter;
				}

				if(nCallCounter >= nCallNumber)
				{
					break;
				}
			}
		}

		m_dwLastSummonTime = m_dwCurrentTime;
	}
	

	__super::AttackHero();
}

bool MoonWarLordMonster::LogicAttackTarget(const MonsLogicItem* _pLogic)
{
	if(0 == _pLogic->m_nLogicID)
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
			//m_stData.eGameState = OS_ATTACK;
			m_stData.nDrt = nDrt;
			PkgObjectActionNot not;
			not.uUserId = m_pTarget->GetID();
			not.uAction = _pLogic->m_nActionCode;
			not.uTargetId = GetID();
			not.uParam0 = MAKE_POSITION_DWORD(this);
			not.uParam1 = MAKELONG(0, nDrt);
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			//m_dwLastAttackTime = m_dwCurrentTime;

			//m_pTarget->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.6f);
			int nPosX = GetCoordX() + 2 * g_nMoveOft[nDrt * 2];
			int nPosY = GetCoordY() + 2 * g_nMoveOft[nDrt * 2 + 1];

			GameObjectList xObjs;
			GetLocateScene()->GetMapObjects(nPosX, nPosY, 2, xObjs);

			int nTotalDamage = 0;

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
							m_dwLastAttackHeroTime = GetTickCount();
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
							int nDamage = pObj->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 1.6);
							nTotalDamage += nDamage;
						}
					}
				}
			}

			//m_pTarget->SetEffStatus(MMASK_LVDU, 15000, 10);
		}
		return true;
	}
	else if(1 == _pLogic->m_nLogicID)
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
			//m_stData.eGameState = OS_ATTACK;
			m_stData.nDrt = nDrt;
			PkgObjectActionNot not;
			not.uUserId = m_pTarget->GetID();
			not.uAction = _pLogic->m_nActionCode;
			not.uTargetId = GetID();
			not.uParam0 = MAKE_POSITION_DWORD(this);
			not.uParam1 = MAKELONG(0, nDrt);
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

			//	fly
			/*int nPosX = m_pTarget->GetCoordX();
			int nPosY = m_pTarget->GetCoordY();

			if(m_pTarget->GetType() == SOT_MONSTER)
			{
				GameObject* pMaster = static_cast<MonsterObject*>(m_pTarget)->GetMaster();
				if(pMaster)
				{
					nPosX = pMaster->GetCoordX();
					nPosY = pMaster->GetCoordY();
				}
			}

			for(int i = 0; i < 8; ++i)
			{
				int nFlyPosX = nPosX + g_nMoveOft[i * 2];
				int nFlyPosY = nPosY + g_nMoveOft[i * 2 + 1];

				if(GetLocateScene()->CanMove(nFlyPosX, nFlyPosY))
				{
					FlyTo(nFlyPosX, nFlyPosY);
					m_dwLastFlyTime = GetTickCount();
					break;
				}
			}*/
			
			//	attack
			GameObjectList xObjs;
			GetLocateScene()->GetMapObjects(GetCoordX(), GetCoordY(), 6, xObjs);

			int nTotalDamage = 0;
			PkgPlayerPlayAniAck aniack;
			aniack.uTargetId = GetID();

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
							m_dwLastAttackHeroTime = GetTickCount();
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
							int nDamage = pObj->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 1.3);
							nTotalDamage += nDamage;
							aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY));
						}
					}
				}

				g_xThreadBuffer.Reset();
				g_xThreadBuffer << aniack;
				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			}
		}
		return true;
	}
	else if(2 == _pLogic->m_nLogicID)
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
			//m_stData.eGameState = OS_ATTACK;
			m_stData.nDrt = nDrt;
			PkgObjectActionNot not;
			not.uUserId = m_pTarget->GetID();
			not.uAction = _pLogic->m_nActionCode;
			not.uTargetId = GetID();
			not.uParam0 = MAKE_POSITION_DWORD(this);
			not.uParam1 = MAKELONG(0, nDrt);
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			//m_dwLastAttackTime = m_dwCurrentTime;

			//GameObjectList xObjs;
			//GetLocateScene()->GetMapObjects(m_pTarget->GetUserData()->wCoordX, m_pTarget->GetUserData()->wCoordY, 3, xObjs);

			int nTotalDamage = GetObject_MaxHP() / 20;

			if(nTotalDamage > 0)
			{
				int nPreHP = GetObject_HP();
				IncHP(nTotalDamage);
				GetValidCheck()->IncHP(nTotalDamage);

				if(nPreHP != GetObject_HP())
				{
					PkgPlayerUpdateAttribNtf not;
					not.uTargetId = GetID();
					not.bType = UPDATE_HP;
					not.dwParam = GetObject_HP();

					g_xThreadBuffer.Reset();
					g_xThreadBuffer << not;
					GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
				}
			}

			unsigned int dwLastTime = 20 * 1000;
			m_dwLastDefenceTime = GetTickCount() + dwLastTime;
			PkgPlayerPlayAniAck aniack;
			aniack.uTargetId = GetID();
			aniack.wAniID = 1;
			aniack.xPos.push_back(dwLastTime);
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << aniack;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		}
		return true;
	}
	else if(3 == _pLogic->m_nLogicID)
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
			//m_stData.eGameState = OS_ATTACK;
			m_stData.nDrt = nDrt;
			PkgObjectActionNot not;
			not.uUserId = m_pTarget->GetID();
			not.uAction = _pLogic->m_nActionCode;
			not.uTargetId = GetID();
			not.uParam0 = MAKE_POSITION_DWORD(this);
			not.uParam1 = MAKELONG(0, nDrt);
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			//m_dwLastAttackTime = m_dwCurrentTime;

			PkgPlayerPlayAniAck aniack;
			aniack.uTargetId = GetID();

			GameObjectList xObjs;
			GetLocateScene()->GetMapObjects(m_pTarget->GetUserData()->wCoordX, m_pTarget->GetUserData()->wCoordY, 5, xObjs);
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
							pObj->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 1.4);
							aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY));
						}
					}
				}
			}

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << aniack;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		}
		return true;
	}

	return false;
}

bool MoonWarLordMonster::LogicCanAttack(const MonsLogicItem* _pLogic)
{
	if(0 == _pLogic->m_nLogicID)
	{
		return MonsterObject::CanAttack();
	}
	else if(1 == _pLogic->m_nLogicID)
	{
		if(m_pTarget == NULL)
		{
			return false;
		}

		int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
		int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

		if(0 == nOftX &&
			0 == nOftY)
		{
			return false;
		}

		if(abs(nOftX) <= 6 &&
			abs(nOftY) <= 6)
		{
			return true;
		}

		return false;
	}
	else if(2 == _pLogic->m_nLogicID)
	{
		if(m_pTarget == NULL)
		{
			return false;
		}

		int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
		int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

		if(0 == nOftX &&
			0 == nOftY)
		{
			return false;
		}

		if(abs(nOftX) <= 8 &&
			abs(nOftY) <= 8)
		{
			return true;
		}

		return false;
	}

	return false;
}