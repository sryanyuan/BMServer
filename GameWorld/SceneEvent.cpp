#include "SceneEvent.h"
#include "GameSceneManager.h"
#include "ObjectEngine.h"
#include "../common/glog.h"
//////////////////////////////////////////////////////////////////////////
SceneEventExecutor::SceneEventExecutor()
{
	ZeroMemory(m_bExistCache, sizeof(m_bExistCache));
}

void SceneEventExecutor::AddSceneEvent(SceneEvent _eType, const char* _pszFuncName, DWORD _dwInterval)
{
	if(NULL == _pszFuncName)
	{
		return;
	}
	if(0 == strlen(_pszFuncName))
	{
		return;
	}

	SceneEventItem* pItem = new SceneEventItem;
	pItem->eEventType = _eType;
	pItem->dwLastCallbackTime = 0;
	pItem->dwCallbackInterval = _dwInterval;
	pItem->xFuncName = _pszFuncName;

	m_xSceneEventList.push_back(pItem);
	m_bExistCache[_eType] = true;
}

void SceneEventExecutor::RemoveSceneEvent(SceneEvent _eType)
{
	SceneEventList::iterator begIter = m_xSceneEventList.begin();

	for(begIter;
		begIter != m_xSceneEventList.end();
		)
	{
		SceneEventItem* pItem = *begIter;

		if(pItem->eEventType == _eType)
		{
			SAFE_DELETE(pItem);
			begIter = m_xSceneEventList.erase(begIter);
		}
		else
		{
			++begIter;
		}
	}

	m_bExistCache[_eType] = false;
}

void SceneEventExecutor::RemoveAllSceneEvent()
{
	SceneEventList::iterator begIter = m_xSceneEventList.begin();
	SceneEventList::iterator endIter = m_xSceneEventList.end();

	for(begIter;
		begIter != m_xSceneEventList.end();
		++begIter)
	{
		SceneEventItem* pItem = *begIter;

		SAFE_DELETE(pItem);
		//begIter = m_xSceneEventList.erase(begIter);
	}

	m_xSceneEventList.clear();
	ZeroMemory(m_bExistCache, sizeof(m_bExistCache));
}

bool SceneEventExecutor::IsEventExist(SceneEvent _eEvent)
{
	return m_bExistCache[_eEvent];
}

int SceneEventExecutor::Call_SceneUpdate(GameScene* _pScene)
{
	if(!IsEventExist(SceneEvent_Update))
	{
		return -1;
	}

	SceneEventList::iterator begIter = m_xSceneEventList.begin();
	SceneEventList::iterator endIter = m_xSceneEventList.end();

	int nCalled = 0;

	for(begIter;
		begIter != m_xSceneEventList.end();
		++begIter)
	{
		SceneEventItem* pItem = *begIter;

		if(pItem->eEventType == SceneEvent_Update)
		{
			if((pItem->dwCallbackInterval == 0) ||
				(pItem->dwCallbackInterval != 0 && GetTickCount() - pItem->dwLastCallbackTime > pItem->dwCallbackInterval))
			{
				lua_State* pState = _pScene->GetLuaState();

				lua_getglobal(pState, pItem->xFuncName.c_str());
				tolua_pushusertype(pState, _pScene, "GameScene");

				int nRet = lua_pcall(pState, 1, 0, 0);
				if(nRet != 0)
				{
#ifdef _DEBUG
					LOG(WARNING) << "Can't call onSceneUpdate : " << lua_tostring(pState, -1);
#endif
					lua_pop(pState, 1);
				}
				else
				{
					++nCalled;
				}

				pItem->dwLastCallbackTime = GetTickCount();
			}
		}
	}

	return nCalled;
}

int SceneEventExecutor::Call_ScenePlayerEnter(GameScene* _pScene, HeroObject* _pHero)
{
	if(!IsEventExist(SceneEvent_PlayerEnter))
	{
		return -1;
	}

	SceneEventList::iterator begIter = m_xSceneEventList.begin();
	SceneEventList::iterator endIter = m_xSceneEventList.end();

	int nCalled = 0;

	for(begIter;
		begIter != m_xSceneEventList.end();
		++begIter)
	{
		SceneEventItem* pItem = *begIter;

		if(pItem->eEventType == SceneEvent_PlayerEnter)
		{
			if((pItem->dwCallbackInterval == 0) ||
				(pItem->dwCallbackInterval != 0 && GetTickCount() - pItem->dwLastCallbackTime > pItem->dwCallbackInterval))
			{
				lua_State* pState = _pScene->GetLuaState();

				lua_getglobal(pState, pItem->xFuncName.c_str());
				tolua_pushusertype(pState, _pScene, "GameScene");
				tolua_pushusertype(pState, _pHero, "HeroObject");

				int nRet = lua_pcall(pState, 2, 0, 0);
				if(nRet != 0)
				{
#ifdef _DEBUG
					LOG(WARNING) << "Can't call Call_ScenePlayerEnter : " << lua_tostring(pState, -1);
#endif
					lua_pop(pState, 1);
				}
				else
				{
					++nCalled;
				}

				pItem->dwLastCallbackTime = GetTickCount();
			}
		}
	}

	return nCalled;
}

int SceneEventExecutor::Call_ScenePlayerLeave(GameScene* _pPrevScene, GameScene* _pScene, HeroObject* _pHero)
{
	if(!IsEventExist(SceneEvent_PlayerLeave))
	{
		return -1;
	}

	SceneEventList::iterator begIter = m_xSceneEventList.begin();
	SceneEventList::iterator endIter = m_xSceneEventList.end();

	int nCalled = 0;

	for(begIter;
		begIter != m_xSceneEventList.end();
		++begIter)
	{
		SceneEventItem* pItem = *begIter;

		if(pItem->eEventType == SceneEvent_PlayerLeave)
		{
			if((pItem->dwCallbackInterval == 0) ||
				(pItem->dwCallbackInterval != 0 && GetTickCount() - pItem->dwLastCallbackTime > pItem->dwCallbackInterval))
			{
				lua_State* pState = _pPrevScene->GetLuaState();

				lua_getglobal(pState, pItem->xFuncName.c_str());
				tolua_pushusertype(pState, _pHero, "HeroObject");
				tolua_pushusertype(pState, _pPrevScene, "GameScene");
				tolua_pushusertype(pState, _pScene, "GameScene");

				int nRet = lua_pcall(pState, 3, 0, 0);
				if(nRet != 0)
				{
#ifdef _DEBUG
					LOG(WARNING) << "Can't call Call_ScenePlayerLeave : " << lua_tostring(pState, -1);
#endif
					lua_pop(pState, 1);
				}
				else
				{
					++nCalled;
				}

				pItem->dwLastCallbackTime = GetTickCount();
			}
		}
	}

	return nCalled;
}