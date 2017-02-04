#include "ObjectEngine.h"
#include "../GameWorld/GameSceneManager.h"
#include "GameWorld.h"
//////////////////////////////////////////////////////////////////////////
NPCObject::NPCObject(/*DWORD _dwID*/) /*: GameObject(_dwID)*/
{
	m_eType = SOT_NPC;
	m_xMsgQueue.Resize(1024);
	m_bWorldNPC = false;
	ClearAllSellItem();
}

NPCObject::~NPCObject()
{

}
//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* virtual void DoWork(unsigned int _dwTick)
/************************************************************************/
void NPCObject::DoWork(unsigned int _dwTick)
{
	DoMsgQueue(_dwTick);
	//DoAction(_dwTick);
}

//////////////////////////////////////////////////////////////////////////
void NPCObject::ProcessPacket(PacketHeader* _pPkt)
{
	BEGIN_HANDLE_PACKET(_pPkt)
		HANDLE_PACKET(PKG_GAME_PLAYER_CLICKNPC_REQ,			PkgPlayerClickNPCReq)
	END_HANDLE_PACKET()
}

//////////////////////////////////////////////////////////////////////////
void NPCObject::DoPacket(const PkgPlayerClickNPCReq& req)
{
	GameScene* pScene = GetLocateScene();
	if(NULL == pScene)
	{
		return;
	}

	HeroObject* pHero = (HeroObject*)pScene->GetPlayerWithoutLock(req.uUserId);
	if(NULL == pHero)
	{
		return;
	}

	if(!m_bWorldNPC)
	{
		int nRet = 0;
		if(req.dwButtonID == 0)
		{
			lua_getglobal(pScene->GetLuaState(), "OnNPCClick");
			tolua_pushusertype(pScene->GetLuaState(), this, "NPCObject");
			tolua_pushusertype(pScene->GetLuaState(), pHero, "HeroObject");
			nRet = lua_pcall(pScene->GetLuaState(), 2, 0, 0);
		}
		else
		{
			lua_getglobal(pScene->GetLuaState(), "OnDlgButtonClick");
			tolua_pushnumber(pScene->GetLuaState(), req.dwButtonID);
			tolua_pushusertype(pScene->GetLuaState(), this, "NPCObject");
			tolua_pushusertype(pScene->GetLuaState(), pHero, "HeroObject");
			nRet = lua_pcall(pScene->GetLuaState(), 3, 0, 0);
		}

		if(nRet != 0)
		{
			LOG(ERROR) << "On map[" << pScene->GetMapID() <<"] execute [OnNPCClick] error:" << lua_tostring(pScene->GetLuaState(), -1);
			lua_pop(pScene->GetLuaState(), 1);
		}
	}
	else
	{
		//	call all world registered npc handler
		/*WorldEvent_NPCActive we;
		we.nButtonId = req.dwButtonID;
		we.pNPC = this;
		we.pHero = pHero;
		GameWorld::GetInstance().GetEventDispatcher()->DispatchEvent(kWorldEvent_NPCActive, &we);*/
	}
}
//////////////////////////////////////////////////////////////////////////
void NPCObject::AddSellItem(int _id)
{
	if(m_nCurItems >= 72)
	{
		return;
	}
	else
	{
		m_sSellItems[m_nCurItems++] = _id;
	}
}
//////////////////////////////////////////////////////////////////////////
bool NPCObject::IsItemExist(int _id)
{
	for(int i = 0; i < 72; ++i)
	{
		if(m_sSellItems[i] == _id)
		{
			return true;
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////