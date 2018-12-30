#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
#include "../../GameWorld/GameDbBuffer.h"
//////////////////////////////////////////////////////////////////////////
WoMaJiaoZhuMonster::WoMaJiaoZhuMonster()
{
	m_dwLastSummonTime = 0;
	m_dwLastFlyTime = 0;
	m_bUpdateAttrib = false;
}

WoMaJiaoZhuMonster::~WoMaJiaoZhuMonster()
{

}

bool WoMaJiaoZhuMonster::AttackTarget()
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
	int nDamage = GetRandomAbility(AT_DC);

	if(nDrt != -1)
	{
		ReceiveDamageInfo di = {0};
		if(GetObject_ID() == 148 ||
			GetObject_ID() == 158)
		{
			int nRand = rand() % 8;
			if(0 == nRand)
			{
				di.bIgnoreAC = true;
				di.bIgnoreMagicShield = true;

				//	limit max attack damage
				if(nDamage > m_pTarget->GetObject_MaxHP() / 4)
				{
					nDamage = m_pTarget->GetObject_MaxHP() / 4;
				}
			}
		}

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

		m_pTarget->ReceiveDamage(this, IsMagicAttackMode(), nDamage, 500, &di);
	}
	return true;
}

void WoMaJiaoZhuMonster::MonsterLogic()
{
	DWORD dwCurTick = GetTickCount();

	bool bNeedFly = false;
	DWORD dwInterval = 15000;
	if(GetObject_HP() < 1000)
	{
		dwInterval = 10000;
	}

	if(m_pTarget)
	{
		if(m_pTarget->GetType() == SOT_MONSTER)
		{
			bNeedFly = true;
		}
		int nOftX = abs((int)m_stData.wCoordX - (int)m_pTarget->GetUserData()->wCoordX);
		int nOftY = abs((int)m_stData.wCoordY - (int)m_pTarget->GetUserData()->wCoordY);
		if(nOftX > 1 ||
			nOftY > 1)
		{
			bNeedFly = true;
		}
	}

	if (GetObject_ID() == 158) {
		bNeedFly = false;
	}

	if(dwCurTick - m_dwLastAttackTime > 3 * 1000 &&
		dwCurTick - m_dwLastFlyTime > dwInterval &&
		bNeedFly)
	{
		m_dwLastFlyTime = dwCurTick;

		if(m_pTarget)
		{
			int nPosX = m_pTarget->GetUserData()->wCoordX;
			int nPosY = m_pTarget->GetUserData()->wCoordY;

			if(m_pTarget->GetType() == SOT_MONSTER)
			{
				//	Find the master
				MonsterObject* pMonster = static_cast<MonsterObject*>(m_pTarget);
				if(pMonster->GetMaster())
				{
					if(pMonster->GetMaster()->GetType() == SOT_HERO)
					{
						nPosX = pMonster->GetMaster()->GetUserData()->wCoordX;
						nPosY = pMonster->GetMaster()->GetUserData()->wCoordY;
						if(pMonster->GetMaster()->GetMapID() == GetMapID())
						{
							SetTarget(pMonster->GetMaster());
						}
					}
				}
			}

			int nDrt = -1;

			for(int i = 0; i < 8; ++i)
			{
				nPosX = m_pTarget->GetUserData()->wCoordX + g_nMoveOft[i * 2];
				nPosY = m_pTarget->GetUserData()->wCoordY + g_nMoveOft[i * 2 + 1];

				if(GetLocateScene()->CanMove(nPosX, nPosY))
				{
					FlyTo(nPosX, nPosY);
					return;
				}
			}
		}
	}

	if(GetObject_HP() < (GetObject_MaxHP() / 2))
	{
		if(!m_bUpdateAttrib)
		{
			SetObject_DC(GetObject_DC() * 2);
			SetObject_MaxDC(GetObject_MaxDC() * 2);
			//SetObject_MaxMC(GetObject_MaxMC() - 400);
			//SetObject_MC(GetObject_MC() - 200);
			SetObject_MaxMC(0);
			SetObject_MC(0);
			SetObject_MoveSpeed(GetObject_MoveSpeed() + 1);
			m_dwWalkCost = GetWalkCostTime();
			PkgPlayerUpdateAttribNtf ntf;
			ntf.uTargetId = GetID();
			ntf.bType = UPDATE_MOVESPD;
			ntf.dwParam = GetObject_MoveSpeed();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ntf;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			m_bUpdateAttrib = true;
		}

		if(dwCurTick - m_dwLastSummonTime > 80 * 1000)
		{
			m_dwLastSummonTime = dwCurTick;
			//DWORD dwMonsNumber = GetLocateScene()->GetMonsterSum(GetUserData()->stAttrib.tex);

			if(GetObject_Tex() != 0)
			{
				int nMovedNumber = GetLocateScene()->MoveSomeMonsterTo(GetObject_Tex(), 2, GetUserData()->wCoordX, GetUserData()->wCoordY);
				if(nMovedNumber < 2)
				{
					int nCallNumber = 2 - nMovedNumber;
					int nPosX = 0;
					int nPosY = 0;
					int nCallCounter = 0;

					for(int i = 0; i < 8; ++i)
					{
						nPosX = m_stData.wCoordX + 2 * g_nMoveOft[2 * i];
						nPosY = m_stData.wCoordY + 2 * g_nMoveOft[2 * i + 1];

						if(GetLocateScene()->CanThrough(nPosX, nPosY))
						{
							GetLocateScene()->CreateMonster(GetObject_Tex(), nPosX, nPosY);
							++nCallCounter;
						}

						if(nCallCounter >= nCallNumber)
						{
							break;
						}
					}
				}
			}
		}
		else
		{
			__super::MonsterLogic();
		}
	}
	else
	{
		if(m_bUpdateAttrib)
		{
			SetObject_DC(GetObject_DC() / 2);
			SetObject_MaxDC(GetObject_MaxDC() / 2);
			//SetObject_MaxMC(GetObject_MaxMC() + 400);
			//SetObject_MC(GetObject_MC() + 200);
			ItemAttrib item;
			if(GetRecordInMonsterTable(GetObject_ID(), &item))
			{
				if(IsElite())
				{
					SetObject_MaxMC(item.maxMC / MONS_ELITE_MULTI);
					SetObject_MC(item.MC / MONS_ELITE_MULTI);
				}
				else if(IsLeader())
				{
					SetObject_MaxMC(item.maxMC / MONS_LEADER_MULTI);
					SetObject_MC(item.MC / MONS_LEADER_MULTI);
				}
				else
				{
					SetObject_MaxMC(item.maxMC);
					SetObject_MC(item.MC);
				}
			}
			SetObject_MoveSpeed(GetObject_MoveSpeed() - 1);
			m_dwWalkCost = GetWalkCostTime();
			m_bUpdateAttrib = false;

			PkgPlayerUpdateAttribNtf ntf;
			ntf.uTargetId = GetID();
			ntf.bType = UPDATE_MOVESPD;
			ntf.dwParam = GetObject_MoveSpeed();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ntf;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
		}

		__super::MonsterLogic();
	}
}