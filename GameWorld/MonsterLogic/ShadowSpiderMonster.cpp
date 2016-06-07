#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
ShadowSpiderMonster::ShadowSpiderMonster()
{
	m_dwLastSummonTime = 0;
}

bool ShadowSpiderMonster::CanAttack()
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

bool ShadowSpiderMonster::AttackTarget()
{
	if(NULL == m_pTarget)
	{
		return true;
	}
	if(m_pTarget->GetUserData()->eGameState == OS_DEAD ||
		m_pTarget->GetObject_HP() == 0)
	{
		m_pTarget = NULL;
		return true;
	}

	DWORD dwCurTick = GetTickCount();
	int nTargetX = 0;
	int nTargetY = 0;
	int nMonsterCounter = 0;
	nMonsterCounter = GetLocateScene()->GetMonsterSum(53);

	if(dwCurTick - m_dwLastSummonTime > 8 * 1000)
	{
		m_dwLastSummonTime = dwCurTick;

		if(nMonsterCounter < 12)
		{
			for(int i = 0; i < 8; i += 2)
			{
				nTargetX = m_stData.wCoordX + g_nMoveOft[i * 2];
				nTargetY = m_stData.wCoordY + g_nMoveOft[i * 2 + 1];

				if(GetLocateScene()->CanThrough(nTargetX, nTargetY))
				{
					GetLocateScene()->CreateMonster(53, nTargetX, nTargetY);
				}
			}
		}
	}

	PkgObjectActionNot not;
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