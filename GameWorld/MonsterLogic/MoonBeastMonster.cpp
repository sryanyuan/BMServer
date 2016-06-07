#include "../MonsterTemplateObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
MoonBeastMonster::MoonBeastMonster()
{
	AddLogicItem(0, ACTION_EXT1, 10, 300);
	AddLogicItem(1, ACTION_EXT2, 10, 300);
	AddLogicItem(2, ACTION_EXT3, 10, 300);
	AddLogicItem(3, ACTION_EXT4, 20, 300);
}

bool MoonBeastMonster::LogicAttackTarget(const MonsLogicItem* _pLogic)
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

			m_pTarget->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.3f);

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
			//m_dwLastAttackTime = m_dwCurrentTime;

			m_pTarget->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.7f);

			//m_pTarget->SetEffStatus(MMASK_LVDU, 15000, 10);
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

			GameObjectList xObjs;
			GetLocateScene()->GetMapObjects(m_pTarget->GetUserData()->wCoordX, m_pTarget->GetUserData()->wCoordY, 3, xObjs);

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
						}
					}
				}
			}

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

bool MoonBeastMonster::LogicCanAttack(const MonsLogicItem* _pLogic)
{
	if(0 == _pLogic->m_nLogicID ||
		1 == _pLogic->m_nLogicID)
	{
		return MonsterObject::CanAttack();
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

		if(abs(nOftX) <= 3 &&
			abs(nOftY) <= 3)
		{
			return true;
		}

		return false;
	}
	else if(3 == _pLogic->m_nLogicID)
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

		if(abs(nOftX) <= 5 &&
			abs(nOftY) <= 5)
		{
			return true;
		}

		return false;
	}

	return false;
}