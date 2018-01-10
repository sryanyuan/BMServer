#ifndef _INC_TEAMMATECONTROL_
#define _INC_TEAMMATECONTROL_
//////////////////////////////////////////////////////////////////////////
#include <vector>
#include <map>
//////////////////////////////////////////////////////////////////////////
#define MAX_TEAMMATES	6
//////////////////////////////////////////////////////////////////////////
class HeroObject;
class GameTeam;
//////////////////////////////////////////////////////////////////////////
typedef std::vector<HeroObject*> TeammatesVector;
typedef std::map<int, GameTeam*> TeamMap;
//////////////////////////////////////////////////////////////////////////

class GameTeam
{
	friend class GameTeamManager;

protected:
	GameTeam();
public:
	~GameTeam();

public:
	void Clear();
	bool Insert(HeroObject* _pHero);
	void Remove(HeroObject* _pHero);
	size_t GetCount();
	bool IsFull();
	bool IsInTeam(HeroObject* _pHero);
	bool SetTeamLeader(HeroObject* _pHero);
	TeammatesVector& GetAllPlayer()		{return m_xTeammates;}

	inline int GetTeamID()
	{
		return m_nTeamID;
	}
	inline HeroObject* GetTeamLeader()
	{
		if(-1 == m_nTeamLeaderIndex)
		{
			return NULL;
		}
		return m_xTeammates[m_nTeamLeaderIndex];
	}

public:
	void SendTeamMessage(const char* _pszMsg);
	void AddTeamExpr(HeroObject* _pMain, int _nExpr);

protected:
	int m_nTeamID;
	TeammatesVector m_xTeammates;
	int m_nTeamLeaderIndex;
};

//////////////////////////////////////////////////////////////////////////
class GameTeamManager
{
public:
	static GameTeamManager* GetInstance(bool _bDestroy = false)
	{
		static GameTeamManager* s_pIns = NULL;
		if (_bDestroy) {
			delete s_pIns;
			s_pIns = NULL;
			return s_pIns;
		}
		if(NULL == s_pIns)
		{
			s_pIns = new GameTeamManager;
		}
		return s_pIns;
	}

	~GameTeamManager();

protected:
	GameTeamManager();

public:
	static int GenerateTeamID()
	{
		return ++s_nTeamIDCounter;
	}

public:
	GameTeam* CreateTeam();
	bool RemoveTeam(int _nTeamID);
	GameTeam* GetTeam(int _nTeamID);

protected:
	static int s_nTeamIDCounter;
	TeamMap m_xTeamMap;
};
//////////////////////////////////////////////////////////////////////////
#endif