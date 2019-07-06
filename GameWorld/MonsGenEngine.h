#ifndef _INC_MONSGENENGINE_
#define _INC_MONSGENENGINE_
//////////////////////////////////////////////////////////////////////////
#include <list>
#include <map>
#include <vector>
#include <time.h>
#include "fastdelegate.h"
#include "FastDelegateBind.h"
#include <functional>
//////////////////////////////////////////////////////////////////////////
#define DEFAULT_MONSGENRECORDVECTOR_SIZE	32
//////////////////////////////////////////////////////////////////////////
// param: monsID, monsType, posX, posY
// return: monsObjectID
typedef fastdelegate::FastDelegate4<int, int, int, int, int> GenMonsCallback;
typedef fastdelegate::FastDelegate2<int, int, bool> CanThroughCallback;
typedef std::function<float()> MonsGenMultipleCallback;
//////////////////////////////////////////////////////////////////////////
struct MonsGenRecord
{
	int nMonsID;
	int nCount;
	int nPosX;
	int nPosY;
	int nOffset;
	time_t nInterval;
	int nGenType;
	bool bBoss;

	// private fields
	int nRecordID;
	time_t nLastGenTime;

	MonsGenRecord()
	{
		memset(this, 0, sizeof(MonsGenRecord));
	}
};
typedef std::vector<MonsGenRecord*> MonsGenRecordVector;
typedef std::list<MonsGenRecord*> MonsGenRecordList;
typedef std::map<int, int> MonsGenCountMap;

// simple wrapper for MonsGenCountMap
class MonsCountRecorder
{
public:
	MonsCountRecorder();
	~MonsCountRecorder();

public:
	void Reset();
	void Set(int _nKey, int _nValue);
	void Inc(int _nKey, int _nValue);
	int Get(int _nKey) const;

public:
	MonsGenCountMap m_xCountMap;
};

// mons gen engine
class MonsGenEngine
{
public:
	MonsGenEngine();
	~MonsGenEngine();

public:
	int Insert(const MonsGenRecord* _pRecord);
	void Clear();
	// delete just set the element to NULL because we rarely remove gen record
	int DeleteByMonsID(int _nMonsID);
	int DeleteByRecordID(int _nRecordID);
	int GetGenRecordCount();

	int DoGen(const MonsCountRecorder& _refCurrentMonsCount);

	void SetGenMonsCallback(GenMonsCallback _cb)			{m_cbGenMons = _cb;}
	void SetCanThroughCallback(CanThroughCallback _cb)		{m_cbCanThrough = _cb;}
	void SetMonsGenMultipleCallback(MonsGenMultipleCallback _cb) { m_cbGenMultiple = _cb; }

private:
	int GenWithRecord(int _nExistsCount, const MonsGenRecord* _pRecord);

private:
	// removed gen record will be NULL, so we should ignore all NULL value
	// for remove efficient
	int m_nRecordCount;
	MonsGenRecordVector m_xMonsGenRecords;
	// record the total gen mons count
	MonsCountRecorder m_xMonsGenTotalCounter;
	GenMonsCallback m_cbGenMons;
	CanThroughCallback m_cbCanThrough;
	MonsGenMultipleCallback m_cbGenMultiple;
};
//////////////////////////////////////////////////////////////////////////
#endif