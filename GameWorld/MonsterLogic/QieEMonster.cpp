#include "../MonsterTemplateObject.h"
#include "../../GameWorld/GameSceneManager.h"
#include "../../GameWorld/GameWorld.h"
//////////////////////////////////////////////////////////////////////////
QieEMonster::QieEMonster()
{
	if(GetAttribID() == 128)
	{
		SetStoneProb(12);
	}
	else if(GetAttribID() == 153)
	{
		SetStoneProb(20);
	}
	SetStoneTime(2500);
}

bool QieEMonster::AttackTarget()
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
		m_stData.eGameState = OS_ATTACK;
		m_stData.nDrt = nDrt;
		PkgObjectActionNot not;
		not.uUserId = m_pTarget->GetID();
		not.uAction = ACTION_ATTACK;
		not.uTargetId = GetID();
		not.uParam0 = MAKE_POSITION_DWORD(this);
		not.uParam1 = nDrt;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		m_dwLastAttackTime = m_dwCurrentTime;

		ReceiveDamageInfo di = {0};
		di.bIgnoreAC = true;
		di.bIgnoreMagicShield = true;
		m_pTarget->ReceiveDamage(this, IsMagicAttackMode(), 0, 400, &di);
	}
	return true;
}