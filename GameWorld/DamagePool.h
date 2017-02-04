#ifndef _INC_DAMAGEPOOL_
#define _INC_DAMAGEPOOL_
//////////////////////////////////////////////////////////////////////////
#include <map>
#include <vector>
#include "FastDelegate.h"
//////////////////////////////////////////////////////////////////////////
// accumulated damage info
struct AccumulatedInfo
{
	time_t nLastActiveTime;
	int nAccumulatedValue;
	int nUid;
};
typedef std::map<int, AccumulatedInfo> AccumulatedInfoMap;

typedef std::vector<AccumulatedInfo> AccumulatedInfoResults;
typedef fastdelegate::FastDelegate1<int, void*> AccumulatedUserdataCallback;

// sorter
bool SortAccumulatedInfoGreater(const AccumulatedInfo& _refLeft, const AccumulatedInfo& _refRight);
bool SortAccumulatedInfoLess(const AccumulatedInfo& _refLeft, const AccumulatedInfo& _refRight);

class DamagePool
{
public:
	DamagePool();
	~DamagePool();

public:
	void Insert(int _nUid, int _nValue);
	void GetSortedAccumulatedInfo(AccumulatedInfoResults& _refResults, int* _pSum);
	void Clear();

	const AccumulatedInfoMap& GetAccumulatedInfoMap()	{return m_xAccumulatedInfoMap;}
	void SetExpireTime(time_t _nEt)						{m_nExpireTime = _nEt;}

protected:
	AccumulatedInfoMap m_xAccumulatedInfoMap;
	time_t m_nExpireTime;
};
//////////////////////////////////////////////////////////////////////////
#endif