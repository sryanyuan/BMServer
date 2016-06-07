#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
MoLongStoneMonster::MoLongStoneMonster()
{
	m_pBoss = NULL;
	m_dwLastSearchBossTime = 0;
}

void MoLongStoneMonster::DoAction(unsigned int _dwTick)
{
	__super::DoAction(_dwTick);

	if(m_pBoss != NULL)
	{
		if(m_pBoss->GetObject_HP() == 0 &&
			m_pBoss->GetUserData()->eGameState == OS_DEAD)
		{
			m_pBoss = NULL;
		}
	}
}

bool MoLongStoneMonster::AttackTarget()
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
	rcAttackRage.left = (int)m_stData.wCoordX - 8;
	rcAttackRage.right = (int)m_stData.wCoordX + 8;
	rcAttackRage.top = (int)m_stData.wCoordY - 8;
	rcAttackRage.bottom = (int)m_stData.wCoordY + 8;

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

	if(NULL != m_pBoss)
	{
		if(m_pBoss->GetObject_HP() == 0 &&
			m_pBoss->GetUserData()->eGameState == OS_DEAD)
		{
			m_pBoss = NULL;
		}
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
	m_dwLastAttackTime = GetTickCount();
	not.uAction = ACTION_ATTACK;
	not.uParam0 = MAKE_POSITION_DWORD(this);
	not.uTargetId = GetID();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

	if(NULL != m_pBoss &&
		nTotalDamage != 0)
	{
		if(m_pBoss->GetObject_HP() != 0 &&
			m_pBoss->GetUserData()->eGameState != OS_DEAD)
		{
			int nPreHP = m_pBoss->GetObject_HP();
			m_pBoss->IncHP(nTotalDamage);
			m_pBoss->GetValidCheck()->IncHP(nTotalDamage);
			if(nPreHP != m_pBoss->GetObject_HP())
			{
				PkgPlayerUpdateAttribNtf ntf;
				ntf.bType = UPDATE_HP;
				ntf.dwParam = m_pBoss->GetObject_HP();
				ntf.uTargetId = m_pBoss->GetID();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ntf;
				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			}
		}
		
	}

	if(GetTickCount() - m_dwLastSearchBossTime > 5 * 1000)
	{
		if(NULL == m_pBoss)
		{
			m_pBoss = static_cast<MonsterObject*>(GetLocateScene()->GetMonster(86));
			if(m_pBoss != NULL)
			{
				if(m_pBoss->GetUserData()->eGameState == OS_DEAD &&
					m_pBoss->GetObject_HP() == 0)
				{
					m_pBoss = NULL;
				}
			}
			m_dwLastSearchBossTime = GetTickCount();
		}
	}

	return true;
}

bool MoLongStoneMonster::CanAttack()
{
	if(m_pTarget == NULL)
	{
		return false;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

	/*if(nOftX == 0 &&
		nOftY == 0)
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