#include "ObjectStatus.h"
#include "GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
DrugDelayFreeList g_xDrugDelayFreeList;
std::stack<ObjectStatusItem*> g_xObjectStatusFreeList;
//////////////////////////////////////////////////////////////////////////
DrugDelayManager::DrugDelayManager()
{

}

DrugDelayManager::~DrugDelayManager()
{

}


bool DrugDelayManager::CanUse(int _nDrugItemAttribID)
{
	DrugUseHistoryList::iterator fnditer = m_xDrugUseHistoryList.find(_nDrugItemAttribID);
	DrugDelayItem* pDelayItem = NULL;

	if(fnditer != m_xDrugUseHistoryList.end())
	{
		pDelayItem = fnditer->second;
		//	Check if status expires
		if(GetTickCount() > pDelayItem->dwExpireTime)
		{
			//	can use
			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool DrugDelayManager::PushDrugStatus(int _nAttribID, int _nDelayTime)
{
	DrugUseHistoryList::iterator fnditer = m_xDrugUseHistoryList.find(_nAttribID);
	DrugDelayItem* pDelayItem = NULL;

	if(fnditer != m_xDrugUseHistoryList.end())
	{
		pDelayItem = fnditer->second;
		//	Check if status expires
		if(GetTickCount() > pDelayItem->dwExpireTime)
		{
			//	can use
			pDelayItem->dwExpireTime = GetTickCount() + _nDelayTime;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		//	Create a new item
		if(g_xDrugDelayFreeList.empty())
		{
			pDelayItem = new DrugDelayItem;
			ZeroMemory(pDelayItem, sizeof(DrugDelayItem));

			pDelayItem->nDrugAttribID = _nAttribID;
			pDelayItem->dwExpireTime = GetTickCount() + _nDelayTime;
			m_xDrugUseHistoryList.insert(std::make_pair(_nAttribID, pDelayItem));
		}
		else
		{
			pDelayItem = g_xDrugDelayFreeList.top();
			g_xDrugDelayFreeList.pop();

			pDelayItem->nDrugAttribID = _nAttribID;
			pDelayItem->dwExpireTime = GetTickCount() + _nDelayTime;
			m_xDrugUseHistoryList.insert(std::make_pair(_nAttribID, pDelayItem));
		}

		return true;
	}
}

void DrugDelayManager::Clear()
{
	DrugUseHistoryList::iterator begiter = m_xDrugUseHistoryList.begin();
	DrugDelayItem* pDelayItem = NULL;

	for(begiter;
		begiter != m_xDrugUseHistoryList.end();
		++begiter)
	{
		pDelayItem = begiter->second;

		ZeroMemory(pDelayItem, sizeof(DrugDelayItem));
		g_xDrugDelayFreeList.push(pDelayItem);
	}

	m_xDrugUseHistoryList.clear();
}









//////////////////////////////////////////////////////////////////////////
ObjectStatusControl::ObjectStatusControl(GameObject* _pAttach)
{
	m_pAttach = NULL;
	m_pAttach = _pAttach;

	if(NULL == _pAttach)
	{
		assert("Invalid attached object" == NULL);
	}
}

ObjectStatusControl::~ObjectStatusControl()
{

}


void ObjectStatusControl::Clear()
{
	ObjectStatusMap::iterator begiter = m_xObjectStatus.begin();
	ObjectStatusItem* pStatusItem = NULL;

	for(begiter;
		begiter != m_xObjectStatus.end();
		++begiter)
	{
		pStatusItem = begiter->second;

		ZeroMemory(pStatusItem, sizeof(ObjectStatusItem));
		g_xObjectStatusFreeList.push(pStatusItem);
	}

	m_xObjectStatus.clear();
}

void ObjectStatusControl::Update()
{
	//	
	ObjectStatusMap::iterator begiter = m_xObjectStatus.begin();
	ObjectStatusItem* pStatusItem = NULL;

	if(m_pAttach->GetUserData()->eGameState == OS_DEAD)
	{
		Clear();
		return;
	}

	for(begiter;
		begiter != m_xObjectStatus.end();
		)
	{
		pStatusItem = begiter->second;

		if(GetTickCount() > (DWORD)pStatusItem->nExpireTime)
		{
			ZeroMemory(pStatusItem, sizeof(ObjectStatusItem));
			g_xObjectStatusFreeList.push(pStatusItem);

			begiter = m_xObjectStatus.erase(begiter);
		}
		else
		{
			CalcObjectStatus(pStatusItem);
			++begiter;
		}
	}
}

void ObjectStatusControl::CalcObjectStatus(ObjectStatusItem* _pStatusItem)
{
	DWORD dwTimeDelta = GetTickCount() - (DWORD)_pStatusItem->nLastUpdateTime;

	PkgPlayerUpdateAttribNtf ntf;
	ntf.uTargetId = m_pAttach->GetID();

	//	1 second...
	if(_pStatusItem->nStatusID == OE_LVDU)
	{
		if(dwTimeDelta >= 2 * INTERVAL_INC_HP)
		{
			/*int nLeftHP = m_pAttach->GetObject_HP();
			if(nLeftHP > _pStatusItem->nExtraParam)
			{
				m_pAttach->DecHP(_pStatusItem->nExtraParam);
				ntf.bType = UPDATE_HP;
				ntf.dwParam = m_pAttach->GetObject_HP();

				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ntf;
				m_pAttach->GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			}*/
		}
	}
}

ObjectStatusItem* ObjectStatusControl::GetStatus(int _nStatusID)
{
	ObjectStatusMap::iterator fnditer = m_xObjectStatus.find(_nStatusID);
	ObjectStatusItem* pStatusItem = NULL;

	if(fnditer != m_xObjectStatus.end())
	{
		return fnditer->second;
	}
	return NULL;
}

bool ObjectStatusControl::CheckStatusExist(int _nStatusID)
{
	ObjectStatusMap::iterator fnditer = m_xObjectStatus.find(_nStatusID);
	ObjectStatusItem* pStatusItem = NULL;

	if(fnditer != m_xObjectStatus.end())
	{
		return true;
	}
	return false;
}

void ObjectStatusControl::RemoveStatus(int _nStatusID)
{
	ObjectStatusMap::iterator fnditer = m_xObjectStatus.find(_nStatusID);
	ObjectStatusItem* pStatusItem = NULL;

	if(fnditer != m_xObjectStatus.end())
	{
		pStatusItem = fnditer->second;
		ZeroMemory(pStatusItem, sizeof(ObjectStatusItem));
		g_xObjectStatusFreeList.push(pStatusItem);

		fnditer = m_xObjectStatus.erase(fnditer);
	}
}

bool ObjectStatusControl::InsertStatus(int _nStatusID, int _nExpireTime, int _nExtraParam)
{
	//	Already exists
	ObjectStatusMap::iterator fnditer = m_xObjectStatus.find(_nStatusID);
	ObjectStatusItem* pStatusItem = NULL;

	if(fnditer != m_xObjectStatus.end())
	{
		pStatusItem = fnditer->second;

		pStatusItem->nExpireTime = _nExpireTime;
		pStatusItem->nExtraParam = _nExtraParam;
		return true;
	}
	else
	{
		if(g_xObjectStatusFreeList.empty())
		{
			pStatusItem = g_xObjectStatusFreeList.top();
			g_xObjectStatusFreeList.pop();
			ZeroMemory(pStatusItem, sizeof(ObjectStatusItem));
		}
		else
		{
			pStatusItem = new ObjectStatusItem;
			ZeroMemory(pStatusItem, sizeof(ObjectStatusItem));
		}

		pStatusItem->nStatusID = _nStatusID;
		pStatusItem->nExpireTime = _nExpireTime;
		pStatusItem->nExtraParam = _nExtraParam;
		return true;
	}

	return false;
}

bool ObjectStatusControl::IsDecDC()
{
	return CheckStatusExist(OE_DECDC);
}

bool ObjectStatusControl::IsEnergyShield()
{
	return CheckStatusExist(OE_ENERGYSHIELD);
}

bool ObjectStatusControl::IsFreezon()
{
	return CheckStatusExist(OE_FREEZE);
}

bool ObjectStatusControl::IsHide()
{
	return CheckStatusExist(OE_HIDE);
}

bool ObjectStatusControl::IsHongDu()
{
	return CheckStatusExist(OE_HONGDU);
}

bool ObjectStatusControl::IsLvDu()
{
	return CheckStatusExist(OE_LVDU);
}

bool ObjectStatusControl::IsShield()
{
	return CheckStatusExist(OE_SHIELD);
}

bool ObjectStatusControl::IsStone()
{
	return CheckStatusExist(OE_STONE);
}