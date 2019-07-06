#include "../CMainServer/CMainServer.h"
#include "DBDropDownContext.h"
#include "GameSceneManager.h"
#include "../../CommonModule/ShareData.h"
#include "GameDbBuffer.h"
#include "GameWorld.h"
#include "../../CommonModule/SettingLoader.h"
//////////////////////////////////////////////////////////////////////////
DBDropDownContext::DBDropDownContext(DBOperationParam* _pParam)
{
	m_pParam = _pParam;
}

bool DBDropDownContext::IsDropValid()
{
	if(NULL == m_pParam)
	{
		return false;
	}

	int nCanDrop = LOWORD(m_pParam->dwParam[6]);
	if(1 != nCanDrop)
	{
		return false;
	}

	return true;
}

float DBDropDownContext::GetDropMultiple()
{
	if(NULL == m_pParam)
	{
		return 1;
	}

	float fProbMulti = 1.0f;
	unsigned char bMonsType = LOWORD(m_pParam->dwParam[3]);

	if(bMonsType == 1)
	{
		//	Elite
		fProbMulti = MONS_ELITE_DROP;
	}
	else if(bMonsType == 2)
	{
		//	Leader
		fProbMulti = MONS_LEADER_DROP;
	}

	//if(DBThread::GetInstance()->m_nWorldDropMulti == 1)
	{
		int nHumDropMulti = LOWORD(m_pParam->dwParam[4]);
		/*if(true || nHumDropMulti == 2)
		{
			//fProbMulti *= nHumDropMulti;
			fProbMulti *= 1.5f;
		}*/
		if (SettingLoader::GetInstance()->GetIntValue("FINALDROPMULTI") != 1) {
			if (CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
			{
				fProbMulti *= 1.5f;
			}
			else
			{
				//	普通模式 2倍
				fProbMulti *= 2.0f;
			}
		}

		//	礼花加成
		unsigned int dwBurstFireworkTime = GameWorld::GetInstance().GetBurstFireworkTime();
		if(0 != dwBurstFireworkTime)
		{
			fProbMulti += 0.5f;
		}

		//	世界加成
#ifdef _DEBUG
#else
		if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
#endif
		{
			int nWorldDropMulti = GameWorld::GetInstance().GetDropMultiple();
			if(0 != nWorldDropMulti)
			{
				fProbMulti += float(nWorldDropMulti) / 10;
			}
		}

		//	游戏难度
		if(CMainServer::GetInstance()->GetServerMode() == GM_NORMAL)
		{
			unsigned short wDifficulty = GameWorld::GetInstance().GetDifficultyLevel();

			if(kDifficultyEasy == wDifficulty)
			{
				fProbMulti -= 1.5f;
			}
			else if(kDifficultyNovice == wDifficulty)
			{
				fProbMulti -= 3.0f;
			}

			if(fProbMulti < 0.5f)
			{
				fProbMulti = 0.5f;
			}
		}
	}

	return fProbMulti;
}

int DBDropDownContext::GetMagicDropMultiple()
{
	if(NULL == m_pParam)
	{
		return 1;
	}

	unsigned char bMonsType = LOWORD(m_pParam->dwParam[3]);
	int nExtraProb = MAGIC_ITEM_BASE;

	if(bMonsType == 1)
	{
		nExtraProb -= MONS_ELITE_MITEM;
	}
	else if(bMonsType == 2)
	{
		nExtraProb -= MONS_LEADER_MITEM;
	}
	nExtraProb -= (HIWORD(m_pParam->dwParam[6]) * 1000);

	//	世界魔法加成
	if(GameWorld::GetInstance().GetMagicDropFireworkTime() != 0)
	{
		nExtraProb -= 8000;
#ifdef _DEBUG
		g_xConsole.CPrint("调整魔法装备加成至：%d", nExtraProb);
#endif
	}

	if(nExtraProb < MAGIC_ITEM_MIN)
	{
		nExtraProb = MAGIC_ITEM_MIN;
	}

	//	游戏难度
	if(CMainServer::GetInstance()->GetServerMode() == GM_NORMAL)
	{
		unsigned short wDifficulty = GameWorld::GetInstance().GetDifficultyLevel();

		if(kDifficultyEasy == wDifficulty)
		{
			nExtraProb += 5000;
		}
		else if(kDifficultyNovice == wDifficulty)
		{
			nExtraProb += 100000;
		}
	}

	return nExtraProb;
}

GameScene* DBDropDownContext::GetDropScene()
{
	if(NULL == m_pParam)
	{
		return NULL;
	}

	GameScene* pScene = GameSceneManager::GetInstance()->GetScene(HIWORD(m_pParam->dwParam[3]));
	return pScene;
}

int DBDropDownContext::GetOwnerID() {
	return int(m_pParam->dwParam[5]);
}

int DBDropDownContext::GetDropBasePosX()
{
	if(NULL == m_pParam)
	{
		return 0;
	}

	unsigned short wPosX = LOWORD(m_pParam->dwParam[1]);
	return wPosX;
}

int DBDropDownContext::GetDropBasePosY()
{
	if(NULL == m_pParam)
	{
		return 0;
	}

	unsigned short wPosY = HIWORD(m_pParam->dwParam[1]);
	return wPosY;
}

int DBDropDownContext::GetDropPosX(int _nItemIndex)
{
	if(_nItemIndex < 0 ||
		_nItemIndex >= m_xDropPosVector.size())
	{
		return 0;
	}

	return LOWORD(m_xDropPosVector[_nItemIndex]);
}

int DBDropDownContext::GetDropPosY(int _nItemIndex)
{
	if(_nItemIndex < 0 ||
		_nItemIndex >= m_xDropPosVector.size())
	{
		return 0;
	}

	return HIWORD(m_xDropPosVector[_nItemIndex]);
}

int DBDropDownContext::GetAdditionPoint()
{
	GameScene* pScene = GetDropScene();
	if (NULL == pScene)
	{
		return 0;
	}

	WeightCalc& refWorldCalc = GameWorld::GetInstance().GetAdditionPointCalc();
	WeightCalc& refSceneCalc = pScene->GetAdditionPointCalc();

	// first using the scene weight
	if (refSceneCalc.Calculable())
	{
		return refSceneCalc.Get();
	}
	// using world weight
	int nValue = refWorldCalc.Get();
#ifdef _DEBUG
	g_xConsole.CPrint("Drop addition item with value %d", nValue);
#endif
	return nValue;
}

int DBDropDownContext::InitDropPosition(int _nItems)
{
	//	get all dropdown position
	int nDropPosSum = sizeof(g_nSearchPoint) / sizeof(g_nSearchPoint[0]) / 2;
	bool bCanDrop = false;
	int nPosSize = 0;
	int nWhileCounter = 0;
	int nPosX = 0;
	int nPosY = 0;
	unsigned short wPosX = GetDropBasePosX();
	unsigned short wPosY = GetDropBasePosY();
	GameScene* pScene = GetDropScene();

	if(NULL == pScene ||
		0 == wPosX ||
		0 == wPosY)
	{
		return 0;
	}

	for(; ;)
	{
		if(m_xDropPosVector.size() >= _nItems)
		{
			break;
		}

		bCanDrop = true;
		nWhileCounter = 0;

		nPosX = wPosX;
		nPosY = wPosY;
		nPosX += g_nSearchPoint[nPosSize * 2];
		nPosY += g_nSearchPoint[nPosSize * 2 + 1];
		++nPosSize;
		if(nPosSize >= nDropPosSum)
		{
			nPosSize = 0;
		}

		nWhileCounter = 0;

		//	Thread-safe function
		while(!pScene->CanDrop(nPosX, nPosY))
		{
			nPosX = wPosX;
			nPosY = wPosY;
			nPosX += g_nSearchPoint[nPosSize * 2];
			nPosY += g_nSearchPoint[nPosSize * 2 + 1];
			++nPosSize;
			if(nPosSize >= sizeof(g_nSearchPoint) / 2)
			{
				nPosSize = 0;
			}
			++nWhileCounter;
			if(nWhileCounter >= 25)
			{
				bCanDrop = false;
				break;
			}
		}

		if(bCanDrop)
		{
			m_xDropPosVector.push_back(MAKELONG(nPosX, nPosY));
		}
	}

	return m_xDropPosVector.size();
}

GroundItem* DBDropDownContext::NewGroundItem(int _nItemID, int _nPosX, int _nPosY, int _nOwner)
{
	GroundItem* pItem = new GroundItem;
	memset(pItem, 0, sizeof(GroundItem));

	pItem->wPosX = _nPosX;
	pItem->wPosY = _nPosY;
	pItem->nOwner = _nOwner;

	if(GetRecordInItemTable(_nItemID, &pItem->stAttrib))
	{
		std::list<int>* pDropItems = (std::list<int>*)m_pParam->dwParam[2];
		pDropItems->push_back((int)pItem);
		return pItem;
	}
	else
	{
		delete pItem;
	}
	return NULL;
}