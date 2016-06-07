#include "../MonsterTemplateObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
YamaKingMonster::YamaKingMonster()
{
	AddLogicItem(0, ACTION_EXT1, 30, 600, 900);
	AddLogicItem(1, ACTION_EXT2, 20, 500, 900);
	AddLogicItem(2, ACTION_EXT3, 10, 500, 900);
	//AddLogicItem(3, ACTION_EXT4, 5, 500, 900);
	m_dwLastSummonTime = 0;
}

void YamaKingMonster::UpdateLogicProb()
{
	MonsLogicItem* pLogic = GetLogicByID(0);
	if(GetObject_HP() < 8000)
	{
		pLogic->m_nProbability = 10;
	}
	else
	{
		pLogic->m_nProbability = 30;
	}

	pLogic = GetLogicByID(1);
	if(GetObject_HP() < 8000)
	{
		pLogic->m_nProbability = 30;
	}
	else
	{
		pLogic->m_nProbability = 20;
	}

	pLogic = GetLogicByID(2);
	if(GetObject_HP() < 8000)
	{
		pLogic->m_nProbability = 18;
	}
	else
	{
		pLogic->m_nProbability = 10;
	}
}

bool YamaKingMonster::CanAttack()
{
	UpdateLogicProb();

	if(ATTACK_NONE == m_eAttackMode)
	{
		return false;
	}

	//	check valid
	if(ATTACK_SPECIAL == m_eAttackMode)
	{
		MonsLogicItem* pLogic = GetCurrentLogic();
		if(NULL == pLogic)
		{
			m_nExecuteLogicIndex = -1;
			m_eAttackMode = ATTACK_NONE;
			return false;
		}
	}

	//	special attack
	if(ATTACK_SPECIAL == m_eAttackMode)
	{
		return LogicCanAttack(&m_xMonsLogicList[m_nExecuteLogicIndex]);
	}
	else if(ATTACK_NORMAL == m_eAttackMode)
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

	return false;
}

bool YamaKingMonster::LogicCanAttack(const MonsLogicItem* _pLogic)
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

bool YamaKingMonster::AttackTarget()
{
	if(ATTACK_NORMAL == m_eAttackMode)
	{
		bool bRet = NormalAttack();
		m_nExecuteLogicIndex = -1;
		m_eAttackMode = ATTACK_NONE;
	}
	else if(ATTACK_SPECIAL == m_eAttackMode)
	{
		MonsLogicItem* pLogic = GetCurrentLogic();
		if(NULL == pLogic)
		{
			m_eAttackMode = ATTACK_NONE;
			m_nExecuteLogicIndex = -1;
			return false;
		}
		else
		{
			if(LogicAttackTarget(pLogic))
			{
				pLogic->m_dwLastExecuteTime = m_dwCurrentTime;
				GetUserData()->eGameState = OS_EXT1;

				//	攻击成功 重置(OS_EXT1状态下不会继续调用)
				//m_eAttackMode = ATTACK_NONE;
				//m_nExecuteLogicIndex = -1;

				return true;
			}
			else
			{
				m_eAttackMode = ATTACK_NONE;
				m_nExecuteLogicIndex = -1;
				return false;
			}
		}
	}

	return true;
}

bool YamaKingMonster::NormalAttack()
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
	rcAttackRage.left = (int)m_stData.wCoordX - 10;
	rcAttackRage.right = (int)m_stData.wCoordX + 10;
	rcAttackRage.top = (int)m_stData.wCoordY - 10;
	rcAttackRage.bottom = (int)m_stData.wCoordY + 10;

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
	aniack.uUserId = 0;

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
								pObj->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 0.9f);
								aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY));
							}
							else if(pObj->GetType() == SOT_MONSTER)
							{
								MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
								if(pMonster != this &&
									pMonster->GetMaster() != NULL &&
									pMonster->GetUserData()->eGameState != OS_DEAD)
								{
									pMonster->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 0.9f);
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

bool YamaKingMonster::ExtAttack0()
{
	const MonsLogicItem* pLogic = GetLogicByID(0);
	
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
	rcAttackRage.left = (int)m_stData.wCoordX - 10;
	rcAttackRage.right = (int)m_stData.wCoordX + 10;
	rcAttackRage.top = (int)m_stData.wCoordY - 10;
	rcAttackRage.bottom = (int)m_stData.wCoordY + 10;

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
	aniack.uUserId = 1;

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
								pObj->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.2f);
								aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX - 1, pObj->GetUserData()->wCoordY - 1));
							}
							else if(pObj->GetType() == SOT_MONSTER)
							{
								MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
								if(pMonster != this &&
									pMonster->GetMaster() != NULL &&
									pMonster->GetUserData()->eGameState != OS_DEAD)
								{
									pMonster->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.2f);
									aniack.xPos.push_back(MAKELONG(pObj->GetUserData()->wCoordX - 1, pObj->GetUserData()->wCoordY - 1));
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

bool YamaKingMonster::ExtAttack1()
{
	MonsLogicItem* pLogic = GetLogicByID(1);
//	range attack 9*9
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
	int nTargetID = m_pTarget->GetID();

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
		m_stData.eGameState = OS_EXT2;
		m_stData.nDrt = nDrt;

		for(int i = -3; i <= 3; ++i)
		{
			for(int j = -3; j <= 3; ++j)
			{
				/*int nAtkX = (int)GetUserData()->wCoordX + i;
				int nAtkY = (int)GetUserData()->wCoordY + j;*/
				int nAtkX = nTargetX + i;
				int nAtkY = nTargetY + j;

				MapCellInfo* pCellInfo = GetLocateScene()->GetMapData(nAtkX, nAtkY);
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
										pMons->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 1.2f);
									}
								}
								else if(pObj->GetType() == SOT_HERO)
								{
									pObj->ReceiveDamage(this, true, GetRandomAbility(AT_DC) * 1.2f);
								}
							}
						}
					}
				}
			}
		}

		PkgObjectActionNot not;
		not.uUserId = nTargetID;
		not.uAction = pLogic->m_nActionCode;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = MAKELONG(0, nDrt);
		//not.uParam2 = 2;
		//not.uParam3 = MAKE_POSITION_DWORD(m_pTarget);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	}
	return true;
}

bool YamaKingMonster::ExtAttack2()
{
	const MonsLogicItem* pLogic = GetLogicByID(2);

	//	summon
	SummonSlaves();

//	range attack 9*9
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
	int nTargetID = m_pTarget->GetID();

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

	int nTotalDamage = 0;

	if(nDrt != -1)
	{
		m_stData.eGameState = OS_EXT2;
		m_stData.nDrt = nDrt;

		for(int i = -3; i <= 3; ++i)
		{
			for(int j = -3; j <= 3; ++j)
			{
				/*int nAtkX = (int)GetUserData()->wCoordX + i;
				int nAtkY = (int)GetUserData()->wCoordY + j;*/
				int nAtkX = nTargetX + i;
				int nAtkY = nTargetY + j;

				MapCellInfo* pCellInfo = GetLocateScene()->GetMapData(nAtkX, nAtkY);
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
										nTotalDamage += pMons->ReceiveDamage(this, true, pMons->GetObject_HP() * 0.5);
									}
								}
								else if(pObj->GetType() == SOT_HERO)
								{
									nTotalDamage += pObj->ReceiveDamage(this, true, pObj->GetObject_HP() * 0.5);

									HeroObject* pHero = static_cast<HeroObject*>(pObj);
									int nMP = pHero->GetObject_MP();
									nMP /= 3;
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

		if(nTotalDamage != 0)
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
		not.uUserId = nTargetID;
		not.uAction = pLogic->m_nActionCode;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = MAKELONG(0, nDrt);
		//not.uParam2 = 2;
		//not.uParam3 = MAKE_POSITION_DWORD(m_pTarget);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	}
	return true;
}

bool YamaKingMonster::ExtAttack3()
{
	return false;
}

void YamaKingMonster::SummonSlaves()
{
	if(GetTickCount() - m_dwLastSummonTime < 90 * 1000)
	{
		return;
	}

	m_dwLastSummonTime = GetTickCount();

	int nSummonX = GetUserData()->wCoordX;
	int nSummonY = GetUserData()->wCoordY;
	int nCalled = 0;
	int nNeedCalled = 2;
	int nMovedX = 0;

	nCalled = GetLocateScene()->MoveSomeMonsterTo(139, nNeedCalled, nSummonX, nSummonY);
	if(nCalled < nNeedCalled)
	{
		for(int i = 0; i < (nNeedCalled - nCalled); ++i)
		{
			GetLocateScene()->CreateMonster(139, nSummonX, nSummonY);
		}
	}
}

bool YamaKingMonster::LogicAttackTarget(const MonsLogicItem* _pLogic)
{
	if(_pLogic->m_nLogicID == 0)
	{
		return ExtAttack0();
	}
	else if(_pLogic->m_nLogicID == 1)
	{
		return ExtAttack1();
	}
	else if(_pLogic->m_nLogicID == 2)
	{
		return ExtAttack2();
	}
	else if(_pLogic->m_nLogicID == 3)
	{
		return ExtAttack3();
	}

	return false;
}