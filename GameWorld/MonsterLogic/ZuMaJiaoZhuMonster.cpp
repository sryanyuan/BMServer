#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
#include "../../GameWorld/GameDbBuffer.h"
//////////////////////////////////////////////////////////////////////////
ZuMaJiaoZhuMonster::ZuMaJiaoZhuMonster()
{
	m_dwLastSummonTime = 0;
	m_dwLastFlyTime = 0;
	m_bUpdateAttrib = false;
}

ZuMaJiaoZhuMonster::~ZuMaJiaoZhuMonster()
{

}

void ZuMaJiaoZhuMonster::MonsterLogic()
{
	unsigned int dwCurTick = GetTickCount();

	bool bNeedFly = false;
	unsigned int dwInterval = 15000;
	if(GetObject_HP() < 4000)
	{
		dwInterval = 8000;
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

	if(GetObject_HP() < 2000)
	{
		if(!m_bUpdateAttrib)
		{
			SetObject_DC(GetObject_DC() * 2);
			SetObject_MaxDC(GetObject_MaxDC() * 2);
			//SetObject_MaxMC(GetObject_MaxMC() - 400);
			//SetObject_MC(GetObject_MC() - 200);
			SetObject_MaxMC(0);
			SetObject_MC(0);
			SetObject_MAC(GetObject_MAC() + 10);
			//SetObject_MoveSpeed(GetObject_MoveSpeed() + 3);
			PkgPlayerUpdateAttribNtf ntf;
			ntf.uTargetId = GetID();
			ntf.bType = UPDATE_MOVESPD;
			ntf.dwParam = GetObject_MoveSpeed();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ntf;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			m_bUpdateAttrib = true;
		}

		if(dwCurTick - m_dwLastSummonTime > 60 * 1000)
		{
			m_dwLastSummonTime = dwCurTick;
			unsigned int dwMonsNumber = GetLocateScene()->GetMonsterSum(47);

			if(dwMonsNumber < 10)
			{
				int nPosX = 0;
				int nPosY = 0;

				for(int i = 0; i < 8; ++i)
				{
					nPosX = m_stData.wCoordX + 2 * g_nMoveOft[2 * i];
					nPosY = m_stData.wCoordY + 2 * g_nMoveOft[2 * i + 1];

					if(GetLocateScene()->CanMove(nPosX, nPosY))
					{
						GetLocateScene()->CreateMonster(47, nPosX, nPosY);
						GetLocateScene()->CreateMonster(47, nPosX, nPosY);
						return;
					}
				}
			}
		}
		/*
		else if(dwCurTick - m_dwLastAttackTime > 10 * 1000 &&
					dwCurTick - m_dwLastFlyTime > 10 * 1000)
				{
					m_dwLastFlyTime = dwCurTick;
		
					if(m_pTarget)
					{
						int nPosX = m_pTarget->GetUserData()->wCoordX;
						int nPosY = m_pTarget->GetUserData()->wCoordY;
		
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
				}*/
		
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
			SetObject_MAC(GetObject_MAC() - 10);
			//SetObject_MoveSpeed(GetObject_MoveSpeed() - 3);
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