#include "FreeListManager.h"
//////////////////////////////////////////////////////////////////////////
FreeListManager::FreeListManager()
{

}

FreeListManager::~FreeListManager()
{

}


void FreeListManager::PushFreeSceneDelayMsg(SceneDelayMsg* _pMsg)
{
	m_xSceneDelayMsgStack.push(_pMsg);
}

SceneDelayMsg* FreeListManager::GetFreeSceneDelayMsg()
{
	if(m_xSceneDelayMsgStack.empty())
	{
		SceneDelayMsg* pMsg = new SceneDelayMsg;
		ZeroMemory(pMsg, sizeof(SceneDelayMsg));
		return pMsg;
	}
	else
	{
		SceneDelayMsg* pMsg = m_xSceneDelayMsgStack.top();
		m_xSceneDelayMsgStack.pop();
		ZeroMemory(pMsg, sizeof(SceneDelayMsg));
		return pMsg;
	}
}

void FreeListManager::PushFreeAttackMsg(AttackMsg* _pMsg)
{
	m_xAttackMsgStack.push(_pMsg);
}

AttackMsg* FreeListManager::GetFreeAttackMsg()
{
	if(m_xAttackMsgStack.empty())
	{
		AttackMsg* pMsg = new AttackMsg;
		ZeroMemory(pMsg, sizeof(AttackMsg));
		return pMsg;
	}
	else
	{
		AttackMsg* pMsg = m_xAttackMsgStack.top();
		m_xAttackMsgStack.pop();
		ZeroMemory(pMsg, sizeof(AttackMsg));
		return pMsg;
	}
}
//////////////////////////////////////////////////////////////////////////