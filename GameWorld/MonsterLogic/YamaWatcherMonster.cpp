#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
// Yama Watcher
YamaWatcherMonster::YamaWatcherMonster()
{
	m_nAttackMode = -1;
	m_dwLastFlyTime = 0;
}

YamaWatcherMonster::~YamaWatcherMonster()
{

}

void YamaWatcherMonster::MonsterLogic()
{
	if(m_pTarget)
	{
		unsigned int dwFlyInterval = 10000;
		if(GetObject_HP() < 6000)
		{
			dwFlyInterval = 6000;
		}

		if(GetTickCount() - m_dwLastAttackTime > 6000)
		{
			if(GetTickCount() - m_dwLastFlyTime > dwFlyInterval)
			{
				//	Fly to target
				int nTargetX = m_pTarget->GetUserData()->wCoordX;
				int nTargetY = m_pTarget->GetUserData()->wCoordY;

				for(int i = 0; i < 8; ++i)
				{
					if(GetLocateScene()->CanThrough(nTargetX + g_nMoveOft[i * 2],
						nTargetY + g_nMoveOft[i * 2 + 1]))
					{
						FlyTo(nTargetX + g_nMoveOft[i * 2], nTargetY + g_nMoveOft[i * 2 + 1]);
						m_dwLastFlyTime = GetTickCount();
						return;
					}
				}
			}
		}
	}

	__super::MonsterLogic();
}

void YamaWatcherMonster::DoAction(unsigned int _dwTick)
{
	if(GetUserData()->eGameState == OS_EXT1 ||
		GetUserData()->eGameState == OS_EXT2)
	{
		if(m_dwCurrentTime - m_dwLastAttackTime >= m_dwAttackCost)
		{
			m_dwLastAttackTime = m_dwCurrentTime;
			GetUserData()->eGameState = OS_STAND;
		}
	}
	else
	{
		__super::DoAction(_dwTick);
	}
}

bool YamaWatcherMonster::CanAttack()
{
	//	0:Absorb hp mp
	//	1-2:Super range attack
	//	other:normal attack
	if(NULL == m_pTarget)
	{
		return false;
	}

	if(m_nAttackMode == -1)
	{
		if(GetObject_HP() > 7000)
		{
			m_nAttackMode = rand() % 10;	
		}
		else
		{
			m_nAttackMode = rand() % 7;
		}
	}

	if(GetTickCount() - m_dwLastAttackTime > 7000)
	{
		m_nAttackMode = 1;
	}

	if(m_nAttackMode == 0)
	{
		//	Always
		int nOftX = abs((int)GetUserData()->wCoordX - (int)m_pTarget->GetUserData()->wCoordX);
		int nOftY = abs((int)GetUserData()->wCoordY - (int)m_pTarget->GetUserData()->wCoordY);

		if(nOftX == 0 &&
			nOftY == 0)
		{
			return false;
		}

		if(nOftX <= GetViewRange() &&
			nOftY <= GetViewRange())
		{
			return true;
		}
	}
	else if(m_nAttackMode == 1)
	{
		int nOftX = abs((int)GetUserData()->wCoordX - (int)m_pTarget->GetUserData()->wCoordX);
		int nOftY = abs((int)GetUserData()->wCoordY - (int)m_pTarget->GetUserData()->wCoordY);

		if(nOftX == 0 &&
			nOftY == 0)
		{
			return false;
		}

		if(nOftX <= GetViewRange() &&
			nOftY <= GetViewRange())
		{
			return true;
		}
	}
	else
	{
		return __super::CanAttack();
	}

	return false;
}

bool YamaWatcherMonster::AttackTarget()
{
	bool bRet = false;

	if(m_nAttackMode == 0)
	{
		bRet = ExtAttack();	
	}
	else if(m_nAttackMode == 1 ||
		m_nAttackMode == 2)
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

bool YamaWatcherMonster::ExtAttack()
{
	//	Absorb hp and mp
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

	int nDrt = -1;
	int nSx = 0;
	int nSy = 0;
	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

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

	if(nDrt == -1)
	{
		return false;
	}

	GetUserData()->eGameState = OS_EXT1;
	GetUserData()->nDrt = nDrt;
	int nTargetX = m_pTarget->GetUserData()->wCoordX;
	int nTargetY = m_pTarget->GetUserData()->wCoordY;

	int nTotalDamage = 0;

	for(int i = nTargetX - GetViewRange(); i <= nTargetX + GetViewRange(); ++i)
	{
		for(int j = nTargetY - GetViewRange(); j <= nTargetY + GetViewRange(); ++j)
		{
			MapCellInfo* pCellInfo = GetLocateScene()->GetMapData(i, j);
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
									int nDamage = pMons->GetObject_HP() * 0.5f;
									pMons->ReceiveDamage(this, true, nDamage);
								}
							}
							else if(pObj->GetType() == SOT_HERO)
							{
								int nDamage = pObj->GetObject_HP() * 0.5f;
								pObj->ReceiveDamage(this, true, nDamage);

								//	Steal mp
								if(GetObject_ID() == 114)
								{
									HeroObject* pHero = static_cast<HeroObject*>(pObj);
									int nMP = pHero->GetObject_MP();
									nMP /= 2;
									if(nMP > 0)
									{
										pHero->SetObject_MP(nMP);
										pHero->SyncMP(pHero);
										pHero->GetValidCheck()->SetMP(nMP);
										nTotalDamage += nMP;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if(nTotalDamage > 0 &&
		GetObject_ID() == 114)
	{
		int nPreHP = GetObject_HP();
		IncHP(nTotalDamage);
		GetValidCheck()->IncHP(nTotalDamage);
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
	not.uAction = ACTION_EXT1;
	not.uTargetId = GetID();
	not.uParam0 = MAKE_POSITION_DWORD(this);
	not.uParam1 = MAKELONG(0, nDrt);
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	m_dwLastAttackTime = m_dwCurrentTime;

	return true;
}

bool YamaWatcherMonster::ExtAttack2()
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

	if(nDrt != -1)
	{
		//	EXT2
		m_stData.eGameState = OS_EXT2;
		m_stData.nDrt = nDrt;
		PkgObjectActionNot not;
		not.uUserId = m_pTarget->GetID();
		not.uAction = ACTION_EXT2;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = MAKELONG(0, nDrt);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		m_dwLastAttackTime = m_dwCurrentTime;

		//////////////////////////////////////////////////////////////////////////
		RECT rcAttackRage;
		rcAttackRage.left = (int)m_stData.wCoordX - GetViewRange();
		rcAttackRage.right = (int)m_stData.wCoordX + GetViewRange();
		rcAttackRage.top = (int)m_stData.wCoordY - GetViewRange();
		rcAttackRage.bottom = (int)m_stData.wCoordY + GetViewRange();

		std::list<GameObject*> xTargets;

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
		CellData* pCellData = NULL;

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
							//bReqStruck = true;
							//bMissed = false;

							if((*begiter)->bType == CELL_MOVEOBJECT)
							{
								pObj = (GameObject*)(*begiter)->pData;
								if(pObj->GetType() == SOT_HERO &&
									pObj->GetUserData()->eGameState != OS_DEAD)
								{
									//pObj->ReceiveDamage(this, IsMagicAttackMode());
									xTargets.push_back(pObj);
									aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY));
								}
								else if(pObj->GetType() == SOT_MONSTER)
								{
									MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
									if(pMonster != this &&
										pMonster->GetMaster() != NULL &&
										pMonster->GetUserData()->eGameState != OS_DEAD)
									{
										//pMonster->ReceiveDamage(this, IsMagicAttackMode());
										xTargets.push_back(pMonster);
										aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY));
									}
								}
							}
						}
					}
				}
			}
		}

		if(!xTargets.empty())
		{
			int nAtkX = 0;
			int nAtkY = 0;

			std::list<GameObject*>::const_iterator begIter = xTargets.begin();
			std::list<GameObject*>::const_iterator endIter = xTargets.end();

			GameObject* pAtkedObj = NULL;

			for(begIter;
				begIter != endIter;
				++begIter)
			{
				pAtkedObj = *begIter;

				for(int i = -2; i <= 2; ++i)
				{
					for(int j = -2; j <= 2; ++j)
					{
						nAtkX = (int)pAtkedObj->GetUserData()->wCoordX + i;
						nAtkY = (int)pAtkedObj->GetUserData()->wCoordY + j;

						pCell = GetLocateScene()->GetMapData(nAtkX, nAtkY);

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
									pCellData = *begiter;

									if(pCellData->bType == CELL_MOVEOBJECT)
									{
										pObj = (GameObject*)pCellData->pData;

										if(pObj->GetType() == SOT_HERO &&
											pObj->GetUserData()->eGameState != OS_DEAD)
										{
											HeroObject* pHero = static_cast<HeroObject*>(pObj);
											pHero->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.0f);
										}
										else if(pObj->GetType() == SOT_MONSTER &&
											pObj->GetUserData()->eGameState != OS_DEAD)
										{
											MonsterObject* pMons = static_cast<MonsterObject*>(pObj);

											if(pMons->GetMaster() != NULL &&
												pMons != this)
											{
												pMons->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.0f);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		aniack.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << aniack;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	}
	return true;
}

int YamaWatcherMonster::GetRandomAbility(ABILITY_TYPE _type)
{
	if(GetObject_HP() < 7000)
	{
		if(_type == AT_DC)
		{
			int nValue = __super::GetRandomAbility(_type);
			nValue *= 1.2f;
			return nValue;
		}
		else
		{
			return __super::GetRandomAbility(_type);
		}
	}
	else
	{
		return __super::GetRandomAbility(_type);
	}
}