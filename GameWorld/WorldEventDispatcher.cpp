/*#include "WorldEventDispatcher.h"
#include <tolua++.h>
//////////////////////////////////////////////////////////////////////////
WorldEventDispatcher::WorldEventDispatcher()
{

}

WorldEventDispatcher::~WorldEventDispatcher()
{

}

//////////////////////////////////////////////////////////////////////////
int WorldEventDispatcher::OnDispatchEvent(const LuaDispatchEvent* _pEvent, LuaDispatchInfo* _pInfo)
{
	if(_pEvent->nEventId == kWorldEvent_NPCActive)
	{
		const WorldEvent_NPCActive* pEvent = (const WorldEvent_NPCActive*)_pEvent->pEvent;

		tolua_pushusertype(m_pLuaState, pEvent->pHero, "HeroObject");
		tolua_pushusertype(m_pLuaState, pEvent->pNPC, "NPCObject");
		PushInt(pEvent->nButtonId);
		_pInfo->nArgs = 3;
	}

	return 0;
}

void WorldEventDispatcher::OnDispatchEventResult(const LuaDispatchEvent* _pEvent, bool bResult)
{

}*/