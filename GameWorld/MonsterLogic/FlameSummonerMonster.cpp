#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
FlameSummonerMonster::FlameSummonerMonster()
{
	m_dwLastSpellTime1 = m_dwLastSpellTime2 = 0;
	m_dwLastActualAttackTime = 0;
	m_nAttackMode = -1;
}

void FlameSummonerMonster::DoAction(unsigned int _dwTick)
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
	case OS_EXT1:
		{
			if(m_dwCurrentTime - m_dwLastSpellTime1 >= 1400)
			{
				m_dwLastSpellTime1 = m_dwCurrentTime;
				GetUserData()->eGameState = OS_STAND;
			}
		}break;
	case OS_EXT2:
		{
			if(m_dwCurrentTime - m_dwLastSpellTime2 >= 1400)
			{
				m_dwLastSpellTime2 = m_dwCurrentTime;
				GetUserData()->eGameState = OS_STAND;
			}
		}break;
	}
}

bool FlameSummonerMonster::CanAttack()
{
	if(m_pTarget == NULL)
	{
		return false;
	}

	if(m_nAttackMode == -1)
	{
		m_nAttackMode = rand() % 6;
	}

	if(m_nAttackMode >= 2 &&
		m_nAttackMode <= 5)
	{
		if(GetTickCount() - m_dwLastActualAttackTime > 10000)
		{
			m_nAttackMode = 1;
		}
	}

	if(m_nAttackMode >= 2 &&
		m_nAttackMode <= 5)
	{
		//	Normal attack mode
		return __super::CanAttack();
	}
	/*else if(m_nAttackMode == 0)
	{
		//	Normal enhanced attack
		return __super::CanAttack();
	}*/
	else if(m_nAttackMode == 1 ||
		m_nAttackMode == 0)
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

	return false;
}

bool FlameSummonerMonster::AttackTarget()
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
			//	Ext1
			m_stData.eGameState = OS_EXT1;
			m_stData.nDrt = nDrt;
			PkgObjectActionNot not;
			not.uUserId = m_pTarget->GetID();
			not.uAction = ACTION_EXT1;
			not.uTargetId = GetID();
			not.uParam0 = MAKE_POSITION_DWORD(this);
			not.uParam1 = MAKELONG(0, nDrt);
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			//m_dwLastAttackTime = m_dwCurrentTime;
			m_dwLastSpellTime1 = m_dwCurrentTime;

			m_pTarget->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 2.5f);

			m_nAttackMode = -1;
			m_dwLastActualAttackTime = m_dwLastSpellTime1;
		}
		else if(m_nAttackMode == 1)
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
			//m_dwLastAttackTime = m_dwCurrentTime;
			m_dwLastSpellTime2 = m_dwCurrentTime;

			//m_pTarget->ReceiveDamage(this, IsMagicAttackMode());

			m_nAttackMode = -1;
			m_dwLastActualAttackTime = m_dwLastSpellTime2;


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

			/*int nDamage = 0;
			int nDC = 0;
			int nAC = 0;
			bool bReqStruck = true;
			bool bMissed = false;*/

			//PkgPlayerUpdateAttribNtf ntf;
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
												pHero->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.5f);
											}
											else if(pObj->GetType() == SOT_MONSTER &&
												pObj->GetUserData()->eGameState != OS_DEAD)
											{
												MonsterObject* pMons = static_cast<MonsterObject*>(pObj);

												if(pMons->GetMaster() != NULL &&
													pMons != this)
												{
													pMons->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.5f);
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
		else
		{
			m_stData.eGameState = OS_ATTACK;
			m_stData.nDrt = nDrt;
			m_dwLastAttackTime = GetTickCount();

			PkgObjectActionNot not;
			not.uAction = ACTION_ATTACK;
			not.uTargetId = GetID();
			not.uParam0 = MAKE_POSITION_DWORD(this);
			not.uParam1 = nDrt;
			not.uUserId = m_pTarget->GetID();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

			//m_pTarget->ReceiveDamage(this, true);

			m_nAttackMode = -1;
			m_dwLastActualAttackTime = m_dwLastAttackTime;

			//	Special attack
			int nAtkLong = GetObject_MaxSC();
			if(nAtkLong > 0)
			{
				MapCellInfo* pCell = NULL;
				CELLDATALIST* pList = NULL;
				HeroObject* pHero = NULL;
				GameObject* pObj = NULL;
				MonsterObject* pMonster = NULL;
				const UserMagic* pMgc = NULL;
				CellData* pCellData = NULL;

				int nAtkX = 0;
				int nAtkY = 0;

				for(int i = 1; i < nAtkLong + 1; ++i)
				{
					nAtkX = m_stData.wCoordX + g_nMoveOft[m_stData.nDrt * 2] * i;
					nAtkY = m_stData.wCoordY + g_nMoveOft[m_stData.nDrt * 2 + 1] * i;

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
											pMonster->ReceiveDamage(this, IsMagicAttackMode());
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
	return true;
}