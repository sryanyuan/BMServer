#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
FlameDCMonster::FlameDCMonster()
{
	m_dwLastSpellTime = 0;
	m_nAttackMode = -1;
}

void FlameDCMonster::DoAction(unsigned int _dwTick)
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
	case OS_SPELL:
		{
			if(m_dwCurrentTime - m_dwLastSpellTime >= 800)
			{
				m_dwLastSpellTime = m_dwCurrentTime;
				GetUserData()->eGameState = OS_STAND;
			}
		}break;
	}
}

bool FlameDCMonster::CanAttack()
{
	return __super::CanAttack();
}

bool FlameDCMonster::AttackTarget()
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

	int nRandom = rand() % 4;

	if(nDrt != -1)
	{
		if(nRandom == 0)
		{
			m_stData.eGameState = OS_SPELL;
			m_stData.nDrt = nDrt;
			m_dwLastSpellTime = GetTickCount();

			PkgObjectActionNot not;
			not.uAction = ACTION_SPELL;
			not.uTargetId = GetID();
			not.uParam0 = MAKE_POSITION_DWORD(this);
			not.uParam1 = MAKELONG(0, GetUserData()->nDrt);
			not.uUserId = m_pTarget->GetID();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

			//	decrease dc or sc or mc of the target object
			//m_pTarget->SetEffStatus(MMASK_DECDC, 5000, 0);
			m_pTarget->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * 2.0f);
		}
		else
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

			m_pTarget->ReceiveDamage(this, IsMagicAttackMode());
		}
	}
	return true;
}