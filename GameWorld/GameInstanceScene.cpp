#include "GameInstanceScene.h"
#include "../common/glog.h"
//////////////////////////////////////////////////////////////////////////
GameInstanceScene::GameInstanceScene()
{
	m_bFree = false;
	m_dwLastFreeTime = 0;
	m_nCurRound = 0;
	m_bInstanceRunning = false;
	m_bRoundRunning = false;
	SetInstance(true);
}

GameInstanceScene::~GameInstanceScene()
{

}


void GameInstanceScene::Release()
{
	__super::Release();
}

bool GameInstanceScene::Initialize(DWORD _dwMapID)
{
	return __super::Initialize(_dwMapID);
}

void GameInstanceScene::Update(DWORD _dwTick)
{
	__super::Update(_dwTick);

	if(0 == m_nCurPlayers)
	{
		//	Free it
		if(0 == GetSlaveSum())
		{
			SetFree(true);
			DeleteAllMonster();
		}
	}
	else
	{
		if(m_bInstanceRunning)
		{
			if(!m_bRoundRunning)
			{
				if(GetTickCount() - m_dwLastRoundTime > 20000)
				{
					m_bRoundRunning = true;

					//	New round
					++m_nCurRound;
					lua_getglobal(GetLuaState(), "OnRound");
					tolua_pushusertype(GetLuaState(), this, "GameInstanceScene");
					lua_pushinteger(GetLuaState(), (int)m_nCurRound);
					int nRet = lua_pcall(GetLuaState(), 2, 0, 0);
					if(nRet != 0)
					{
#ifdef _DEBUG
						LOG(WARNING) << "Can't call OnRound : " << lua_tostring(GetLuaState(), -1);
#endif
						lua_pop(GetLuaState(), 1);
					}
				}
				else
				{
					//	Nothing
				}
			}
			else
			{
				if(m_nCurMonsters == 0)
				{
					m_bRoundRunning = false;
					m_dwLastRoundTime = GetTickCount();
				}
			}
		}
	}
}

void GameInstanceScene::BeginInstance()
{
	//SetFree(false);

	m_nCurRound = 0;
	m_bRoundRunning = false;
	m_bInstanceRunning = true;
	m_dwLastRoundTime = GetTickCount();
}

void GameInstanceScene::OnRound(int _nRound)
{

}

void GameInstanceScene::EndInstance()
{
	m_bInstanceRunning = false;
}