#include "TeammateControl.h"
#include "ObjectEngine.h"
//////////////////////////////////////////////////////////////////////////
int GameTeamManager::s_nTeamIDCounter = 0;
//////////////////////////////////////////////////////////////////////////
GameTeam::GameTeam()
{
	m_xTeammates.resize(MAX_TEAMMATES, NULL);
	m_nTeamID = 0;
	m_nTeamLeaderIndex = -1;
}

GameTeam::~GameTeam()
{

}

void GameTeam::Clear()
{
	m_xTeammates.assign(MAX_TEAMMATES, NULL);
}

bool GameTeam::Insert(HeroObject* _pHero)
{
	int nInsertIndex = -1;
	bool bInTeam = false;

	for(int i = 0; i < MAX_TEAMMATES; ++i)
	{
		if(m_xTeammates[i] == NULL &&
			nInsertIndex == -1)
		{
			nInsertIndex = i;
		}
		else if(_pHero == m_xTeammates[i])
		{
			nInsertIndex = -1;
			bInTeam = true;
			break;
		}
	}

	if(bInTeam)
	{
		return false;
	}

	if(-1 != nInsertIndex)
	{
		m_xTeammates[nInsertIndex] = _pHero;
		_pHero->SetTeamID(GetTeamID());

		//	默认第一个加入的玩家为队长
		if(GetCount() == 1)
		{
			m_nTeamLeaderIndex = nInsertIndex;
		}

		return true;
	}
	return false;
}

void GameTeam::Remove(HeroObject* _pHero)
{
	bool bTeamDismiss = false;
	bool bIsTeamLeader = false;

	if(GetTeamLeader() == _pHero)
	{
		if(GetCount() < 2)
		{
			bTeamDismiss = true;
		}

		bIsTeamLeader = true;
	}

	for(int i = 0; i < MAX_TEAMMATES; ++i)
	{
		if(m_xTeammates[i] == _pHero)
		{
			_pHero->SetTeamID(0);
			m_xTeammates[i] = NULL;
		}
	}

	if(bTeamDismiss)
	{
		//GameTeamManager::GetInstance()->RemoveTeam(GetTeamID());
	}
	else
	{
		/*for(int i = 0; i < MAX_TEAMMATES; ++i)
		{
			if(m_xTeammates[i] != NULL)
			{
				m_nTeamLeaderIndex = i;
			}
		}*/
		if(bIsTeamLeader)
		{
			for(int i = 0; i < MAX_TEAMMATES; ++i)
			{
				if(m_xTeammates[i] != NULL)
				{
					m_nTeamLeaderIndex = i;
					m_xTeammates[i]->SendSystemMessage("您成为了队长");
				}
			}
		}
	}
}

size_t GameTeam::GetCount()
{
	size_t nCounter = 0;

	for(int i = 0; i < MAX_TEAMMATES; ++i)
	{
		if(m_xTeammates[i] != NULL)
		{
			++nCounter;
		}
	}

	return nCounter;
}

bool GameTeam::IsFull()
{
	return MAX_TEAMMATES == GetCount();
}

bool GameTeam::IsInTeam(HeroObject* _pHero)
{
	for(int i = 0; i < MAX_TEAMMATES; ++i)
	{
		if(m_xTeammates[i] == _pHero)
		{
			return true;
		}
	}

	return false;
}

void GameTeam::SendTeamMessage(const char* _pszMsg)
{
	for(int i = 0; i < MAX_TEAMMATES; ++i)
	{
		if(m_xTeammates[i] != NULL)
		{
			m_xTeammates[i]->SendSystemMessage(_pszMsg);
		}
	}
}

void GameTeam::AddTeamExpr(HeroObject* _pMain, int _nExpr)
{
	int nTeamCounter = 0;
	bool bValid = false;

	for(int i = 0; i < MAX_TEAMMATES; ++i)
	{
		if(m_xTeammates[i] != NULL)
		{
			if(m_xTeammates[i]->GetMapID() == _pMain->GetMapID() &&
				abs(m_xTeammates[i]->GetUserData()->wCoordX - _pMain->GetUserData()->wCoordX < 12) &&
				abs(m_xTeammates[i]->GetUserData()->wCoordY - _pMain->GetUserData()->wCoordY < 12))
			{
				++nTeamCounter;
			}
		}
		if(m_xTeammates[i] == _pMain)
		{
			bValid = true;
		}
	}

	if(bValid)
	{
		if(nTeamCounter == 1)
		{
			_pMain->GainExp(_nExpr);
		}
		else if(nTeamCounter == 0)
		{

		}
		else
		{
			float fExpTotal = _nExpr;
			fExpTotal /= nTeamCounter;
			fExpTotal *= (1.0f + 0.05 * nTeamCounter);

			for(int i = 0; i < MAX_TEAMMATES; ++i)
			{
				if(m_xTeammates[i] != NULL)
				{
					if(m_xTeammates[i] == _pMain)
					{
						_pMain->GainExp((int)(fExpTotal * 1.1f));
					}
					else if(m_xTeammates[i]->GetMapID() == _pMain->GetMapID() &&
						abs(m_xTeammates[i]->GetUserData()->wCoordX - _pMain->GetUserData()->wCoordX < 12) &&
						abs(m_xTeammates[i]->GetUserData()->wCoordY - _pMain->GetUserData()->wCoordY < 12))
					{
						float fPercent = 1.0f;
						int nExpr = fExpTotal;

						if(m_xTeammates[i]->GetObject_Level() - _pMain->GetObject_Level() >= -5)
						{
							fPercent = 1.0f;
						}
						else
						{
							int nLevelGap = _pMain->GetObject_Level() - m_xTeammates[i]->GetObject_Level();
							fPercent -= (nLevelGap / 5 * 0.2f);
							if(fPercent < 0.0f)
							{
								fPercent = 0.0f;
							}

							nExpr = fExpTotal * fPercent;
							if(nExpr <= 0)
							{
								nExpr = 1;
							}
						}

						if(m_xTeammates[i]->GetUserData()->eGameState != OS_DEAD)
						{
							m_xTeammates[i]->GainExp(nExpr);
						}
					}
				}
			}
		}
	}
}

bool GameTeam::SetTeamLeader(HeroObject* _pHero)
{
	for(int i = 0; i < MAX_TEAMMATES; ++i)
	{
		if(m_xTeammates[i] == _pHero)
		{
			m_nTeamLeaderIndex = i;
			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
GameTeamManager::GameTeamManager()
{

}

GameTeamManager::~GameTeamManager()
{

}

GameTeam* GameTeamManager::CreateTeam()
{
	GameTeam* pNewTeam = new GameTeam;
	pNewTeam->m_nTeamID = GenerateTeamID();

	m_xTeamMap.insert(std::make_pair(pNewTeam->GetTeamID(), pNewTeam));

	return pNewTeam;
}

GameTeam* GameTeamManager::GetTeam(int _nTeamID)
{
	TeamMap::const_iterator fnditer = m_xTeamMap.find(_nTeamID);

	if(fnditer != m_xTeamMap.end())
	{
		return fnditer->second;
	}

	return NULL;
}

bool GameTeamManager::RemoveTeam(int _nTeamID)
{
	TeamMap::const_iterator fnditer = m_xTeamMap.find(_nTeamID);

	if(fnditer != m_xTeamMap.end())
	{
		delete fnditer->second;
		fnditer = m_xTeamMap.erase(fnditer);
		return true;
	}

	return false;
}