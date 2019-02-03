#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
MoLongKingMonster::MoLongKingMonster()
{
	m_dwFlyTime = m_dwLastSummonTime = 0;
	m_bUpdateAttrib = false;
}

void MoLongKingMonster::MonsterLogic()
{
	unsigned int dwCurTick = GetTickCount();

	bool bNeedFly = false;
	unsigned int dwInterval = 15000;
	if(GetObject_HP() < 4000)
	{
		dwInterval = 8000;
	}

	if(m_pTarget)
	{
		if(m_pTarget->GetType() == SOT_MONSTER)
		{
			bNeedFly = true;
		}
		int nOftX = abs((int)m_stData.wCoordX - (int)m_pTarget->GetUserData()->wCoordX);
		int nOftY = abs((int)m_stData.wCoordY - (int)m_pTarget->GetUserData()->wCoordY);
		if(nOftX > 1 ||
			nOftY > 1)
		{
			bNeedFly = true;
		}
	}

	if(dwCurTick - m_dwLastAttackTime > 3 * 1000 &&
		dwCurTick - m_dwFlyTime > dwInterval &&
		bNeedFly)
	{
		m_dwFlyTime = dwCurTick;

		if(m_pTarget)
		{
			int nPosX = m_pTarget->GetUserData()->wCoordX;
			int nPosY = m_pTarget->GetUserData()->wCoordY;

			if(m_pTarget->GetType() == SOT_MONSTER)
			{
				//	Find the master
				MonsterObject* pMonster = static_cast<MonsterObject*>(m_pTarget);
				if(pMonster->GetMaster())
				{
					if(pMonster->GetMaster()->GetType() == SOT_HERO)
					{
						nPosX = pMonster->GetMaster()->GetUserData()->wCoordX;
						nPosY = pMonster->GetMaster()->GetUserData()->wCoordY;
						if(pMonster->GetMaster()->GetMapID() == GetMapID())
						{
							SetTarget(pMonster->GetMaster());
						}
					}
				}
			}

			int nDrt = -1;

			for(int i = 0; i < 8; ++i)
			{
				nPosX = m_pTarget->GetUserData()->wCoordX + g_nMoveOft[i * 2];
				nPosY = m_pTarget->GetUserData()->wCoordY + g_nMoveOft[i * 2 + 1];

				if(GetLocateScene()->CanMove(nPosX, nPosY))
				{
					FlyTo(nPosX, nPosY);
					return;
				}
			}
		}
	}

	int nSummonInterval = 30 * 1000;
	if(GetObject_HP() < 2000)
	{
		nSummonInterval = 15 * 1000;
	}
	if(dwCurTick - m_dwLastSummonTime > nSummonInterval &&
		m_pTarget)
	{
		PkgObjectActionNot not;
		not.uAction = ACTION_SPELL;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = MAKELONG(0, GetUserData()->nDrt);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

		m_dwLastSummonTime = dwCurTick;
		GetUserData()->eGameState = OS_SPELL;

		int nPosX = GetUserData()->wCoordX;
		int nPosY = GetUserData()->wCoordY;
		int nSummonX = nPosX;
		int nSummonY = nPosY;
		int nCall90 = 0;
		int nCall91 = 0;
		int nMovedX = 0;

		for(int i = 1; i <= 4; ++i)
		{
			for(int j = 0; j < 8; ++j)
			{
				nSummonX = nPosX + g_nMoveOft[j * 2] * i;
				nSummonY = nPosY + g_nMoveOft[j * 2 + 1] * i;

				if(GetLocateScene()->CanThrough(nSummonX, nSummonY))
				{
					if(nCall90 < 1)
					{
						if(1 == GetLocateScene()->MoveSomeMonsterTo(90, 1, nSummonX, nSummonY))
						{
							//
							++nCall90;
						}
						else
						{
							if(GetObject_ID() == 141)
							{
								GetLocateScene()->CreateLeaderMonster(90, nSummonX, nSummonY);
							}
							else
							{
								GetLocateScene()->CreateMonster(90, nSummonX, nSummonY);
							}
							
							++nCall90;
						}
					}
					else if(nCall91 < 1)
					{
						if(1 == GetLocateScene()->MoveSomeMonsterTo(91, 1, nSummonX, nSummonY))
						{
							//
							++nCall91;
						}
						else
						{
							if(GetObject_ID() == 141)
							{
								GetLocateScene()->CreateLeaderMonster(91, nSummonX, nSummonY);
							}
							else
							{
								GetLocateScene()->CreateMonster(91, nSummonX, nSummonY);
							}
							++nCall91;
						}
					}
					else
					{
						break;
					}
				}
			}

			if(nCall91 == 1 &&
				nCall90 == 1)
			{
				break;
			}
		}
		/*for(int i = -4; i <= 4; ++i)
		{
			for(int j = -4; j <= 4; ++j)
			{
				nSummonX = nPosX + j;
				nSummonY = nPosY + i;

				if(GetLocateScene()->CanThrough(nSummonX, nSummonY))
				{
					if(nCall90 < 1)
					{
						if(1 == GetLocateScene()->MoveSomeMonsterTo(90, 1, nSummonX, nSummonY))
						{
							//
							++nCall90;
						}
						else
						{
							GetLocateScene()->CreateMonster(90, nSummonX, nSummonY);
							++nCall90;
						}
					}
					else if(nCall91 < 1)
					{
						if(1 == GetLocateScene()->MoveSomeMonsterTo(91, 1, nSummonX, nSummonY))
						{
							//
							++nCall91;
						}
						else
						{
							GetLocateScene()->CreateMonster(91, nSummonX, nSummonY);
							++nCall91;
						}
					}
					else
					{
						break;
					}
				}
			}
		}*/

		int nPreHP = GetObject_HP();
		//IncHP(100);
		if(m_pTarget)
		{
			HeroObject* pDestHero = NULL;
			if(m_pTarget->GetType() == SOT_HERO)
			{
				pDestHero = static_cast<HeroObject*>(m_pTarget);
			}
			else if(m_pTarget->GetType() == SOT_MONSTER)
			{
				MonsterObject* pMonster = static_cast<MonsterObject*>(m_pTarget);
				if(pMonster->GetMaster() != NULL)
				{
					if(pMonster->GetMaster()->GetType() == SOT_HERO)
					{
						pDestHero = static_cast<HeroObject*>(pMonster->GetMaster());
					}
				}
			}

			if(pDestHero != NULL)
			{
				int nGetMP = pDestHero->GetObject_MP() / 2;
				if(nGetMP != 0)
				{
					pDestHero->DecMP(nGetMP);
					pDestHero->GetValidCheck()->DecMP(nGetMP);
					pDestHero->SyncMP(pDestHero);

					IncHP(nGetMP);
					m_pValid->IncHP(nGetMP);
				}
			}
		}
		
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
		return;
	}

	if(GetObject_HP() < 2000)
	{
		if(!m_bUpdateAttrib)
		{
			SetObject_DC(GetObject_DC() * 2);
			SetObject_MaxMC(GetObject_MaxMC() - 200);
			SetObject_MAC(GetObject_MAC() + 10);
			//SetObject_MoveSpeed(GetObject_MoveSpeed() + 3);
			PkgPlayerUpdateAttribNtf ntf;
			ntf.uTargetId = GetID();
			ntf.bType = UPDATE_MOVESPD;
			ntf.dwParam = GetObject_MoveSpeed();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ntf;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			m_bUpdateAttrib = true;
		}

		__super::MonsterLogic();
	}
	else
	{
		if(m_bUpdateAttrib)
		{
			SetObject_DC(GetObject_DC() / 2);
			SetObject_MaxMC(GetObject_MaxMC() + 200);
			SetObject_MAC(GetObject_MAC() - 10);
			//SetObject_MoveSpeed(GetObject_MoveSpeed() - 3);
			PkgPlayerUpdateAttribNtf ntf;
			ntf.uTargetId = GetID();
			ntf.bType = UPDATE_MOVESPD;
			ntf.dwParam = GetObject_MoveSpeed();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ntf;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			m_bUpdateAttrib = false;
		}

		__super::MonsterLogic();
	}
}

void MoLongKingMonster::DoAction(unsigned int _dwTick)
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
			if(m_dwCurrentTime - m_dwLastSummonTime >= 2500)
			{
				m_dwLastSummonTime = m_dwCurrentTime;
				GetUserData()->eGameState = OS_STAND;
			}
		}break;
	}
}

bool MoLongKingMonster::AttackTarget()
{
	if(m_pTarget == NULL)
	{
		return false;
	}
	if(m_stData.eGameState == OS_DEAD)
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
	int nSx = 0;
	int nSy = 0;

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
		//	Range attack
		RECT rcAttackRage;
		rcAttackRage.left = (int)m_stData.wCoordX - 2;
		rcAttackRage.right = (int)m_stData.wCoordX + 2;
		rcAttackRage.top = (int)m_stData.wCoordY - 2;
		rcAttackRage.bottom = (int)m_stData.wCoordY + 2;

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

		int nDamage = 0;
		int nDC = 0;
		int nAC = 0;
		bool bReqStruck = true;
		bool bMissed = false;

		PkgObjectActionNot not;
		PkgPlayerUpdateAttribNtf ntf;

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
		m_stData.eGameState = OS_ATTACK;
		m_stData.nDrt = nDrt;
		m_dwLastAttackTime = GetTickCount();
		not.uAction = ACTION_ATTACK;
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = m_stData.nDrt;
		not.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	}

	return true;
}