#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
bool ChiYueEMoMonster::AttackTarget()
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
	PkgPlayerPlayAniAck aniack;

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
							if(pObj->GetType() == SOT_HERO &&
								pObj->GetUserData()->eGameState != OS_DEAD)
							{
								pObj->ReceiveDamage(this, IsMagicAttackMode());
								aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY));
							}
							else if(pObj->GetType() == SOT_MONSTER)
							{
								MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
								if(pMonster != this &&
									pMonster->GetMaster() != NULL &&
									pMonster->GetUserData()->eGameState != OS_DEAD)
								{
									pMonster->ReceiveDamage(this, IsMagicAttackMode());
									aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY));
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

	aniack.uTargetId = GetID();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << aniack;
	GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	return true;
}

bool ChiYueEMoMonster::CanAttack()
{
	if(m_pTarget == NULL)
	{
		return false;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

	if(abs(nOftX) <= GetViewRange() &&
		abs(nOftY) <= GetViewRange())
	{
		return true;
	}

	return false;
}