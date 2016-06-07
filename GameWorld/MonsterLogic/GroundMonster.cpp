#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* The ground monster
/************************************************************************/
GroundMonster::GroundMonster()
{
	m_bInGround = true;
	m_stData.eGameState = OS_GROUND;
}

GroundMonster::~GroundMonster()
{

}
//////////////////////////////////////////////////////////////////////////
void GroundMonster::OnFindTarget()
{
	if(m_stData.eGameState == OS_GROUND)
	{
		PkgObjectActionNot not;
		not.uAction = ACTION_APPEAR;
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		m_stData.eGameState = OS_APPEAR;
		m_dwLastAppearTime = m_dwCurrentTime;
	}
}
//////////////////////////////////////////////////////////////////////////
void GroundMonster::OnLoseTarget()
{
	if(m_stData.eGameState == OS_STAND)
	//	Up the ground
	{
		PkgObjectActionNot not;
		not.uAction = ACTION_GROUND;
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		m_stData.eGameState = OS_GROUND;
		m_dwLastGroundTime = m_dwCurrentTime;
	}
}
//////////////////////////////////////////////////////////////////////////
bool GroundMonster::CanAttack()
{
	if(m_pTarget == NULL)
	{
		return false;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

	/*if(0 == nOftX &&
		0 == nOftY)
	{
		return false;
	}*/

	if(abs(nOftX) <= GetViewRange() &&
		abs(nOftY) <= GetViewRange())
	{
		return true;
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GroundMonster::IsOutOfView()
{
	if(m_pTarget == NULL)
	{
		return true;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;
	if(abs(nOftX) > GetViewRange() ||
		abs(nOftY) > GetViewRange())

	{
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
void GroundMonster::DoAction(unsigned int _dwTick)
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
			if(m_dwCurrentTime - /*m_stData.*/m_dwLastAttackTime >= INTERVAL_ATTACK / 2)
			{
				m_stData.eGameState = OS_STAND;
			}
		}break;
	case OS_WALK:
		{
			if(m_dwCurrentTime - /*m_stData.*/m_dwLastWalkTime >= INTERVAL_WALK / 2)
			{
				m_stData.eGameState = OS_STAND;
			}
		}break;
	case OS_STRUCK:
		{
			//	
			if(m_dwCurrentTime - /*m_stData.*/m_dwLastStruckTime >= INTERVAL_STRUCK)
			{
				m_stData.eGameState = OS_STAND;

				stActNot.uTargetId = GetID();
				stActNot.uAction = ACTION_STAND;
				stActNot.uParam0 = MAKE_POSITION_DWORD(this);

				g_xThreadBuffer.Reset();
				g_xThreadBuffer << stActNot;
				pScene->BroadcastPacket(&g_xThreadBuffer);

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
			if(m_dwCurrentTime - /*m_stData.*/m_dwLastDeadTime >= INTERVAL_DEAD / 2)
			{
				//	Clean the dead body
				GetLocateScene()->AddWaitDelete(this);
			}
		}break;
	case OS_APPEAR:
		{
			if(m_dwCurrentTime - m_dwLastAppearTime >= INTERVAL_APPEAR)
			{
				m_stData.eGameState = OS_STAND;
			}
		}break;
	case OS_GROUND:
		{
			if(m_dwCurrentTime - m_dwLastGroundTime >= INTERVAL_GROUND)
			{
				//m_stData.eGameState = OS_
				if(m_pTarget == NULL)
				{
					if(m_dwCurrentTime - /*m_stData.*/m_dwLastSearchTime >= INTERVAL_SEARCH)
					{
						m_pTarget = SearchViewRange();
					}
					if(m_pTarget)
					{
						OnFindTarget();
					}
				}
				else
				{
					//	Have target??
					OnFindTarget();
				}
			}
		}break;
	case OS_STAND:
		{
			if(GetObject_SC() > 0)
			{
				//	Auto search target
				if(m_pTarget == NULL)
				{
					OnLoseTarget();
					if(m_dwCurrentTime - /*m_stData.*/m_dwLastSearchTime >= INTERVAL_SEARCH)
					{
						m_pTarget = SearchViewRange();
					}
					if(m_pTarget == NULL)
					{
						Wonder();
					}
					else
					{
						//OnFindTarget();
					}
				}
				else
				{
					AttackHero();
				}
			}
			else
			{
				if(m_pTarget == NULL)
				{
					//	Auto walk
					Wonder();
				}
				else
				{
					//	Try to attack hero
					AttackHero();
				}
			}
		}break;
	}
}
//////////////////////////////////////////////////////////////////////////
bool GroundMonster::AttackTarget()
{
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
	rcAttackRage.left = (int)m_stData.wCoordX - 7;
	rcAttackRage.right = (int)m_stData.wCoordX + 7;
	rcAttackRage.top = (int)m_stData.wCoordY - 6;
	rcAttackRage.bottom = (int)m_stData.wCoordY + 6;

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
								pObj->ReceiveDamage(this, IsMagicAttackMode());
							}
							else if(pObj->GetType() == SOT_MONSTER)
							{
								MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
								if(pMonster != this &&
									pMonster->GetMaster() != NULL)
								{
									pObj->ReceiveDamage(this, IsMagicAttackMode());
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
	not.uAction = ACTION_ATTACK;
	not.uParam0 = MAKE_POSITION_DWORD(this);
	not.uTargetId = GetID();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	return true;
}