#include "../CMainServer/CMainServer.h"
#include "GameWorld.h"
#include "MonsterTemplateObject.h"
#include "GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
static const int s_nMoveOft[] =
{
	0,-1,
	1,-1,
	1,0,
	1,1,
	0,1,
	-1,1,
	-1,0,
	-1,-1
};
//////////////////////////////////////////////////////////////////////////
MonsterTemplateObject::MonsterTemplateObject()
{
	m_nExecuteLogicIndex = -1;
	m_dwLastRecoverTime = 0;
	m_eAttackMode = ATTACK_NONE;
}

MonsterTemplateObject::~MonsterTemplateObject()
{

}

MonsLogicItem* MonsterTemplateObject::GetCurrentLogic()
{
	if(m_nExecuteLogicIndex < 0 ||
		m_nExecuteLogicIndex >= m_xMonsLogicList.size())
	{
		return NULL;
	}

	return &m_xMonsLogicList[m_nExecuteLogicIndex];
}

bool MonsterTemplateObject::CanAttack()
{
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
		return __super::CanAttack();
	}

	return false;
}

void MonsterTemplateObject::DoAction(unsigned int _dwTick)
{
	if(NULL == m_pTarget)
	{
		m_nExecuteLogicIndex = -1;
	}
	if(OS_DEAD == GetUserData()->eGameState)
	{
		m_nExecuteLogicIndex = -1;
	}

	if(GetUserData()->eGameState == OS_EXT1)
	{
		//
		MonsLogicItem* pLogic = GetCurrentLogic();

		if(NULL == pLogic)
		{
			GetUserData()->eGameState = OS_STAND;
		}
		else
		{
			if(m_dwCurrentTime - pLogic->m_dwLastExecuteTime > pLogic->m_dwRecoverTime)
			{
				GetUserData()->eGameState = OS_STAND;
				m_nExecuteLogicIndex = -1;
				m_eAttackMode = ATTACK_NONE;
				m_dwLastRecoverTime = GetTickCount();
			}
		}
	}
	else
	{
		int nPrevState = GetUserData()->eGameState;
		__super::DoAction(_dwTick);
		if(nPrevState == OS_ATTACK &&
			GetUserData()->eGameState == OS_STAND)
		{
			m_dwLastRecoverTime = GetTickCount();
		}
	}
}

void MonsterTemplateObject::SelectAttackMode()
{
	if(ATTACK_NONE == m_eAttackMode)
	{
		m_eAttackMode = ATTACK_NORMAL;

		int nRandom = rand() % 100;
		int nExecuteIndex = -1;
		int nExecuteCounter = 0;

		for(int i = 0; i < m_xMonsLogicList.size(); ++i)
		{
			int nProb = m_xMonsLogicList[i].m_nProbability;

			if(nProb != 0)
			{
				int nPreCounter = nExecuteCounter;
				nExecuteCounter += m_xMonsLogicList[i].m_nProbability;

				if(nRandom > nPreCounter &&
					nRandom <= nExecuteCounter)
				{
					nExecuteIndex = i;
					break;
				}
			}
		}

		if(nExecuteIndex != -1)
		{
			m_nExecuteLogicIndex = nExecuteIndex;
			m_eAttackMode = ATTACK_SPECIAL;
		}
	}
}

//	OS_STAND状态不断调用
void MonsterTemplateObject::AttackHero()
{
	if(m_pTarget == NULL)
	{
		return;
	}
	if(m_stData.eGameState != OS_STAND)
	{
		return;
	}
	if(m_pTarget->GetObject_HP() == 0 ||
		m_pTarget->GetUserData()->eGameState == OS_DEAD)
	{
		SetTarget(NULL);
		return;
	}

	//	Out of range
	if(IsOutOfView())
	{
		SetTarget(NULL);
		return;
	}

	//	free update
	if(!FreeUpdate())
	{
		return;
	}

	//	select attack mode
	if(ATTACK_NONE == m_eAttackMode)
	{
		SelectAttackMode();
	}

	if(!CanAttack())
	{
		//	Go to target
		if(m_dwCurrentTime - /*m_stData.*/m_dwLastWalkTime > GetWalkInterval())
		{
			WalkToTarget();
			//m_stData.dwLastWalkTime = m_dwCurrentTime;
		}
	}
	else
	{
		/*if(m_dwCurrentTime - / *m_stData.* /m_dwLastAttackTime > GetAttackInterval())
		{
			AttackTarget();
		}*/
		MonsLogicItem* pLogic = GetCurrentLogic();

		if(NULL == pLogic)
		{
			//	normal attack
			if(m_dwCurrentTime - m_dwLastAttackTime > GetAttackInterval())
			{
				AttackTarget();
			}
		}
		else
		{
			if(m_dwCurrentTime - m_dwLastRecoverTime > pLogic->m_dwExecuteInterval)
			{
				AttackTarget();
			}
		}
	}
}

void MonsterTemplateObject::AddLogicItem(int _nID, int _nActionCode, int _nProb, int _nExecuteInterval, int _nRecoverTime /* = 0 */)
{
	MonsLogicItem item;
	item.m_nLogicID = _nID;
	item.m_nActionCode = _nActionCode;
	item.m_nProbability = _nProb;
	item.m_dwExecuteInterval = _nExecuteInterval;
	if(_nRecoverTime != 0)
	{
		item.m_dwRecoverTime = _nRecoverTime;
	}
	m_xMonsLogicList.push_back(item);
}

MonsLogicItem* MonsterTemplateObject::GetLogicByID(int _nID)
{
	for(int i = 0; i < m_xMonsLogicList.size(); ++i)
	{
		if(m_xMonsLogicList[i].m_nLogicID == _nID)
		{
			return &m_xMonsLogicList[i];
		}
	}

	return NULL;
}

bool MonsterTemplateObject::AttackTarget()
{
	if(ATTACK_NORMAL == m_eAttackMode)
	{
		//	first call logic attack
		bool bRet = LogicNormalAttack();
		if (!bRet)
		{
			bRet = __super::AttackTarget();
		}
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

int MonsterTemplateObject::GetMoveOffsetXByDirection(int _nDrt)
{
	int nIndex = _nDrt * 2;
	if(nIndex < 0 ||
		nIndex >= sizeof(s_nMoveOft) / sizeof(s_nMoveOft[0]))
	{
		return 0;
	}

	return s_nMoveOft[nIndex];
}

int MonsterTemplateObject::GetMoveOffsetYByDirection(int _nDrt)
{
	int nIndex = _nDrt * 2 + 1;
	if(nIndex < 0 ||
		nIndex >= sizeof(s_nMoveOft) / sizeof(s_nMoveOft[0]))
	{
		return 0;
	}

	return s_nMoveOft[nIndex];
}

int MonsterTemplateObject::GetDirectionByMoveOffset(int _nOftX, int _nOftY)
{
	int nDrt = -1;

	for(int i = 0; i < 8; ++i)
	{
		if(_nOftX == GetMoveOffsetXByDirection(i) &&
			_nOftY == GetMoveOffsetYByDirection(i))
		{
			nDrt = i;
			break;
		}
	}

	return nDrt;
}

int MonsterTemplateObject::AttackTargetsSpecifiedPosition(int _nX, int _nY, int _nAction, float _fAttackFactor, int _nDelay /* = 500 */, const ReceiveDamageInfo* _pInfo /* = NULL */)
{
	return AttackTargetsRange(_nX, _nY, 0, 0, _nAction, _fAttackFactor, _nDelay, _pInfo);
}

int MonsterTemplateObject::AttackTargetsRange(int _nX, int _nY, int _nOftX, int _nOftY, int _nAction, float _fAttackFactor, int _nDelay /* = 500 */, const ReceiveDamageInfo* _pInfo /* = NULL */)
{
	GameObjectList objs;
	GetLocateScene()->GetMappedObjects(_nX, _nY, _nOftX, _nOftY, objs, MAPPEDOBJECT_STATUE_ALIVE | MAPPEDOBJECT_PLAYER | MAPPEDOBJECT_SLAVE);

	GameObjectList::iterator it = objs.begin();
	for (it; it != objs.end(); ++it)
	{
		GameObject* pObj = *it;

		pObj->ReceiveDamage(this, IsMagicAttackMode(), GetRandomAbility(AT_DC) * _fAttackFactor, _nDelay, _pInfo);
	}

	PkgObjectActionNot not;
	m_stData.eGameState = OS_ATTACK;
	m_dwLastAttackTime = GetTickCount();
	not.uAction = _nAction;
	not.uParam0 = MAKE_POSITION_DWORD(this);
	not.uTargetId = GetID();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

	return objs.size();
}