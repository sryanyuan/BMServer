#include "../MonsterTemplateObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
MoonSpiderMonster::MoonSpiderMonster()
{
	AddLogicItem(0, ACTION_EXT1, 40, 300);
}

bool MoonSpiderMonster::LogicAttackTarget(const MonsLogicItem* _pLogic)
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

			m_pTarget->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 1.5f);

			//m_pTarget->SetEffStatus(MMASK_LVDU, 15000, 10);
		}
		return true;
	}

	return false;
}

bool MoonSpiderMonster::LogicCanAttack(const MonsLogicItem* _pLogic)
{
	if(0 == _pLogic->m_nLogicID)
	{
		if(m_pTarget == NULL)
		{
			return false;
		}

		int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
		int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

		if(abs(nOftX) > 2 ||
			abs(nOftY) > 2)
		{
			return false;
		}

		if(nOftY == 0 &&
			nOftX == 0)
		{
			return false;
		}

		if(nOftX == 0 ||
			nOftY == 0)
		{
			return true;
		}

		if(abs(nOftX) == abs(nOftY))
		{
			return true;
		}

		return false;
	}

	return false;
}