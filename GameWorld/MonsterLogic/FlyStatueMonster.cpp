#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
FlyStatueMonster::FlyStatueMonster()
{
	m_dwLastSpellTime = 0;
	m_nAttackMode = -1;
	m_dwLastActualAttackTime = 0;
}

void FlyStatueMonster::DoAction(unsigned int _dwTick)
{
	PkgObjectActionNot stActNot;
	GameScene* pScene = GetLocateScene();
	if(!pScene)
	{
		LOG(ERROR) << "In no scene!";
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
	case OS_SPELL:
		{
			if(m_dwCurrentTime - m_dwLastSpellTime >= 1400)
			{
				m_dwLastSpellTime = m_dwCurrentTime;
				GetUserData()->eGameState = OS_STAND;
			}
		}break;
	}
}

bool FlyStatueMonster::CanAttack()
{
	if(m_pTarget == NULL)
	{
		return false;
	}

	if(m_nAttackMode == -1)
	{
		//	Not decided
		m_nAttackMode = rand() % 5;
	}

	/*if(GetTickCount() - m_dwLastActualAttackTime > 8000)
	{
		m_nAttackMode = 0;
	}*/

	//if(m_nAttackMode == 0)
	{
		int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
		int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

		if(0 == nOftX &&
			0 == nOftY)
		{
			return false;
		}

		if(abs(nOftX) <= GetViewRange() &&
			abs(nOftY) <= GetViewRange())
		{
			return true;
		}

		return false;
	}
	/*else
	{
		//	Normal attack
		return MonsterObject::CanAttack();
	}*/
}

bool FlyStatueMonster::AttackTarget()
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
	if(nDrt != -1)
	{
		if(m_nAttackMode == 0)
		{
			m_stData.eGameState = OS_SPELL;
			m_stData.nDrt = nDrt;
			m_dwLastSpellTime = GetTickCount();

			PkgObjectActionNot not;
			not.uAction = ACTION_SPELL;
			not.uTargetId = GetID();
			not.uParam0 = MAKE_POSITION_DWORD(this);
			not.uParam1 = MAKELONG(0, GetUserData()->nDrt);
			not.uUserId = m_pTarget->GetID();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

			//m_pTarget->ReceiveDamage(this, true);

			m_nAttackMode = -1;
			m_dwLastActualAttackTime = m_dwLastSpellTime;

			//	Attack around
			RECT rcAttackRage;
			rcAttackRage.left = (int)m_pTarget->GetUserData()->wCoordX - 2;
			rcAttackRage.right = (int)m_pTarget->GetUserData()->wCoordX + 2;
			rcAttackRage.top = (int)m_pTarget->GetUserData()->wCoordY - 2;
			rcAttackRage.bottom = (int)m_pTarget->GetUserData()->wCoordY + 2;

			int nTotalDamage = 0;

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

			//int nDamage = 0;
			//int nDC = 0;
			//int nAC = 0;
			//bool bReqStruck = true;
			//bool bMissed = false;

			//PkgObjectActionNot not;
			//PkgPlayerUpdateAttribNtf ntf;

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
								//bReqStruck = true;
								//bMissed = false;

								if((*begiter)->bType == CELL_MOVEOBJECT)
								{
									pObj = (GameObject*)(*begiter)->pData;
									if(pObj->GetType() == SOT_HERO)
									{
										nTotalDamage += pObj->ReceiveDamage(this, IsMagicAttackMode());
									}
									else if(pObj->GetType() == SOT_MONSTER)
									{
										MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
										if(pMonster != this &&
											pMonster->GetMaster() != NULL)
										{
											nTotalDamage += pMonster->ReceiveDamage(this, IsMagicAttackMode());
										}
									}
								}
							}
						}
					}
				}
			}

			if(nTotalDamage != 0 &&
				GetUserData()->eGameState != OS_DEAD)
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
		}
		else
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

			m_nAttackMode = -1;
			m_dwLastActualAttackTime = m_dwLastAttackTime;
		}
	}
	return true;
}