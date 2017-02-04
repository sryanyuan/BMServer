#include "DamagePool.h"
#include <time.h>
#include <algorithm>
//////////////////////////////////////////////////////////////////////////
bool SortAccumulatedInfoGreater(const AccumulatedInfo& _refLeft, const AccumulatedInfo& _refRight)
{
	if (_refLeft.nAccumulatedValue < _refRight.nAccumulatedValue)
	{
		return true;
	}
	return false;
}

bool SortAccumulatedInfoLess(const AccumulatedInfo& _refLeft, const AccumulatedInfo& _refRight)
{
	if (_refLeft.nAccumulatedValue > _refRight.nAccumulatedValue)
	{
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
DamagePool::DamagePool()
{
	// default expire seconds is 30
	m_nExpireTime = 30;
}

DamagePool::~DamagePool()
{

}

void DamagePool::Insert(int _nUid, int _nValue)
{
	time_t tn;
	time(&tn);

	AccumulatedInfoMap::iterator it = m_xAccumulatedInfoMap.find(_nUid);
	if (it != m_xAccumulatedInfoMap.end())
	{
		// find and update
		if (tn - it->second.nLastActiveTime > m_nExpireTime)
		{
			// expired, set to zero
			it->second.nAccumulatedValue = _nValue;
		}
		else
		{
			// not expired, update
			it->second.nAccumulatedValue += _nValue;
		}

		it->second.nLastActiveTime = tn;
		return;
	}

	// add a new record
	AccumulatedInfo info = {0};
	info.nUid = _nUid;
	info.nAccumulatedValue = _nValue;
	info.nLastActiveTime = tn;
	m_xAccumulatedInfoMap.insert(std::make_pair(_nUid, info));
}

void DamagePool::GetSortedAccumulatedInfo(AccumulatedInfoResults& _refResults, int* _pSum)
{
	time_t tn;
	time(&tn);
	// greater damage will be in front
	if (m_xAccumulatedInfoMap.empty())
	{
		return;
	}
	_refResults.reserve(m_xAccumulatedInfoMap.size());

	AccumulatedInfoMap::const_iterator it = m_xAccumulatedInfoMap.begin();
	int nInsertIdx = 0;
	int nSum = 0;
	for (it;
		it != m_xAccumulatedInfoMap.end();
		++it)
	{
		if (tn - it->second.nLastActiveTime > m_nExpireTime)
		{
			// expired
			continue;
		}
		_refResults.push_back(it->second);
		nSum += it->second.nAccumulatedValue;
	}
	sort(_refResults.begin(), _refResults.end(), SortAccumulatedInfoLess);

	if (NULL != _pSum)
	{
		*_pSum = nSum;
	}
}
