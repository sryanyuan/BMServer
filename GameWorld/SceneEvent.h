#ifndef _INC_SCENEEVENT_
#define _INC_SCENEEVENT_
//////////////////////////////////////////////////////////////////////////
#include <list>
#include <windows.h>
#include <string>
//////////////////////////////////////////////////////////////////////////
class GameScene;
class HeroObject;
//////////////////////////////////////////////////////////////////////////
enum SceneEvent
{
	SceneEvent_None,
	SceneEvent_Update,
	SceneEvent_PlayerEnter,
	SceneEvent_PlayerLeave,
	SceneEvent_Total
};

struct SceneEventItem
{
	SceneEvent eEventType;
	std::string xFuncName;
	DWORD dwLastCallbackTime;
	DWORD dwCallbackInterval;

	SceneEventItem()
	{
		dwLastCallbackTime = dwCallbackInterval = 0;
		eEventType = SceneEvent_None;
	}
};

typedef std::list<SceneEventItem*> SceneEventList;

class SceneEventExecutor
{
public:
	SceneEventExecutor();
	~SceneEventExecutor()
	{
		RemoveAllSceneEvent();
	}

public:
	void AddSceneEvent(SceneEvent _eType, const char* _pszFuncName, DWORD _dwInterval);
	void RemoveSceneEvent(SceneEvent _eType);
	void RemoveAllSceneEvent();

	bool IsEventExist(SceneEvent _eEvent);
	int Call_SceneUpdate(GameScene* _pScene);
	int Call_ScenePlayerEnter(GameScene* _pScene, HeroObject* _pHero);
	int Call_ScenePlayerLeave(GameScene* _pPrevScene, GameScene* _pScene, HeroObject* _pHero);

protected:
	bool m_bExistCache[SceneEvent_Total];
	SceneEventList m_xSceneEventList;
};
//////////////////////////////////////////////////////////////////////////
#endif