#ifndef _INC_GAMEINSTANCESCENE_
#define _INC_GAMEINSTANCESCENE_
//////////////////////////////////////////////////////////////////////////
#include "GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
class GameInstanceScene : public GameScene
{
public:
	GameInstanceScene();
	virtual ~GameInstanceScene();

public:
	virtual void Update(unsigned int _dwTick);
	//virtual bool Initialize(unsigned int _dwMapID);
	virtual void Release();

public:
	inline bool IsFree()				{return m_bFree;}
	inline void SetFree(bool _b)		{m_bFree = _b;m_dwLastFreeTime = GetTickCount();}

	void BeginInstance();
	void OnRound(int _nRound);
	void EndInstance();

protected:
	//	是否可以回收副本资源
	bool m_bFree;
	unsigned int m_dwLastFreeTime;
	unsigned int m_dwLastRoundTime;
	int m_nCurRound;
	//	一轮是否开始
	bool m_bRoundRunning;
	//	副本流程是否开始
	bool m_bInstanceRunning;
};
//////////////////////////////////////////////////////////////////////////
#endif