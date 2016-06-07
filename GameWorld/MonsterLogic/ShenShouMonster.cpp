#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
ShenShouMonster::ShenShouMonster()
{

}

ShenShouMonster::~ShenShouMonster()
{

}


bool ShenShouMonster::AttackTarget()
{
	if(NULL == m_pTarget)
	{
		return false;
	}
	if(m_stData.eGameState != OS_STAND)
	{
		return false;
	}
	if(m_pTarget->GetType() == SOT_HERO)
	{
		return false;
	}
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
		/*int nDC = GetRandomAbility(AT_DC);
		int nAC = m_pTarget->GetRandomAbility(AT_MAC);
		int nDamage = nDC - nAC;*/
		m_stData.eGameState = OS_ATTACK;
		m_stData.nDrt = nDrt;

		bool bIsNextTo = true;
		if(abs(nOftX) == 2 ||
			abs(nOftY) == 2)
		{
			bIsNextTo = false;
		}

		m_pTarget->ReceiveDamage(this, IsMagicAttackMode());

		//	Next position
		// bug [11/18/2013 yuanxj]
		//	ReceiveDamage后 可能导致m_pTarget死亡 然后整个场景消除目标 导致以下代码崩溃
		// end [11/18/2013 yuanxj]
		//int nTargetX = m_pTarget->GetUserData()->wCoordX;
		//int nTargetY = m_pTarget->GetUserData()->wCoordY;
		if(bIsNextTo)
		{
			nTargetX += g_nMoveOft[nDrt * 2];
			nTargetY += g_nMoveOft[nDrt * 2 + 1];
		}
		else
		{
			nTargetX = m_stData.wCoordX + g_nMoveOft[nDrt * 2];
			nTargetY = m_stData.wCoordY + g_nMoveOft[nDrt * 2 + 1];
		}
		MapCellInfo* pCell = GetLocateScene()->GetMapData(nTargetX, nTargetY);
		if(pCell != NULL)
		{
			if(pCell->pCellObjects != NULL)
			{
				CELLDATALIST::const_iterator begiter = pCell->pCellObjects->begin();
				for(begiter;
					begiter != pCell->pCellObjects->end();
					++begiter)
				{
					if((*begiter)->bType == CELL_MOVEOBJECT)
					{
						GameObject* pObj = (GameObject*)(*begiter)->pData;
						if(pObj->GetType() == SOT_MONSTER)
						{
							MonsterObject* pMons = static_cast<MonsterObject*>(pObj);
							if(pMons->GetMaster() == NULL)
							{
								/*if(nDC > 0)
								{
									info.uParam[0] = pMons->GetID();
									info.uParam[1] = nDamage;
									info.uParam[2] = GetID();
									info.uParam[3] = 1;
									info.uDelayTime = GetTickCount() + 500;
									pMons->GetLocateScene()->PushDelayBuf(info);
								}*/
								pMons->ReceiveDamage(this, IsMagicAttackMode());
							}
						}
					}
				}
			}
		}

		PkgObjectActionNot not;
		//not.uUserId = m_pTarget->GetID();
		not.uAction = ACTION_ATTACK;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = nDrt;
		//not.uParam2 = 2;
		//not.uParam3 = MAKE_POSITION_DWORD(m_pTarget);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		/*m_stData.*/m_dwLastAttackTime = m_dwCurrentTime;

		//	Erase this target
		/*if(bIsDead &&
			m_pTarget->GetType() == SOT_HERO)
		{
			m_pTarget->GetLocateScene()->EraseTarget(m_pTarget);
		}*/
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
int ShenShouMonster::GetRandomAbility(ABILITY_TYPE _type)
{
	int nValue = MonsterObject::GetRandomAbility(_type);

	if(_type == AT_DC)
	{
		if(m_pMaster)
		{
			int nSC = m_pMaster->GetRandomAbility(AT_SC);
			const UserMagic* pMgc = NULL;
			if(m_pMaster->GetType() == SOT_HERO)
			{
				HeroObject* pHero = static_cast<HeroObject*>(m_pMaster);
				pMgc = pHero->GetUserMagic(MEFF_SUMMON);
				if(pMgc)
				{
					if(pMgc->bLevel > 0)
					{
						float fInc = 0.2 + (float)pMgc->bLevel / 10;
						if(fInc >= 0.5f)
						{
							fInc = 0.5f;
						}

						nSC *= fInc;
						nValue += nSC;
					}
				}

				int nAddShenShou = pHero->GetStateController()->GetAddShenShouAddition();
				if(nAddShenShou > 0)
				{
					float fMulti = 1 + (float)nAddShenShou / 10;
					nValue *= fMulti;
				}
			}
		}
	}
	return nValue;
}