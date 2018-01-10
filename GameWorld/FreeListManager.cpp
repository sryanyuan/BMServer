#include "FreeListManager.h"
//////////////////////////////////////////////////////////////////////////
FreeListManager::FreeListManager()
{

}

FreeListManager::~FreeListManager()
{
	Clear();
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

void FreeListManager::Clear() {
	while (!m_xAttackMsgStack.empty()) {
		AttackMsg* pMsg = m_xAttackMsgStack.top();
		delete pMsg;
		m_xAttackMsgStack.pop();
	}
	while (!m_xSceneDelayMsgStack.empty()) {
		SceneDelayMsg* pMsg = m_xSceneDelayMsgStack.top();
		delete pMsg;
		m_xSceneDelayMsgStack.pop();
	}
}
//////////////////////////////////////////////////////////////////////////