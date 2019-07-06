#include <algorithm>
#include "MonsGenEngine.h"
//////////////////////////////////////////////////////////////////////////
static int s_nMonsGenRecordIDSeed = 0;
//////////////////////////////////////////////////////////////////////////
MonsGenEngine::MonsGenEngine()
{
	m_xMonsGenRecords.reserve(DEFAULT_MONSGENRECORDVECTOR_SIZE);
	m_nRecordCount = 0;
}

MonsGenEngine::~MonsGenEngine()
{
	Clear();
}

int MonsGenEngine::Insert(const MonsGenRecord* _pRecord)
{
	if (0 == _pRecord->nMonsID)
	{
		// invalid record
		return 0;
	}
	MonsGenRecord* pRecord = new MonsGenRecord;
	memcpy(pRecord, _pRecord, sizeof(MonsGenRecord));
	//pRecord->nLastGenTime = 0;
	pRecord->nRecordID = ++s_nMonsGenRecordIDSeed;
	m_xMonsGenRecords.push_back(pRecord);
	// update total mons count
	m_xMonsGenTotalCounter.Inc(pRecord->nMonsID, pRecord->nCount);
	++m_nRecordCount;
	return pRecord->nRecordID;
}

int MonsGenEngine::DeleteByMonsID(int _nMonsID)
{
	MonsGenRecordVector::iterator it = m_xMonsGenRecords.begin();
	int nRemoveCount = 0;

	for (it;
		it != m_xMonsGenRecords.end();
		++it)
	{
		MonsGenRecord* pRecord = *it;
		if (NULL == pRecord)
		{
			continue;
		}

		if (pRecord->nMonsID == _nMonsID)
		{
			// decrease total count
			m_xMonsGenTotalCounter.Inc(pRecord->nMonsID, -pRecord->nCount);

			// free res
			delete pRecord;
			pRecord = NULL;
			++nRemoveCount;
			--m_nRecordCount;

			// set element to NULL
			*it = NULL;
		}
	}

	return nRemoveCount;
}

int MonsGenEngine::DeleteByRecordID(int _nRecordID)
{
	MonsGenRecordVector::iterator it = m_xMonsGenRecords.begin();
	int nRemoveCount = 0;

	for (it;
		it != m_xMonsGenRecords.end();
		++it)
	{
		MonsGenRecord* pRecord = *it;
		if (NULL == pRecord)
		{
			continue;
		}

		if (pRecord->nRecordID == _nRecordID)
		{
			// decrease total count
			m_xMonsGenTotalCounter.Inc(pRecord->nMonsID, -pRecord->nCount);

			delete pRecord;
			pRecord = NULL;
			--m_nRecordCount;

			// set element to NULL
			*it = NULL;

			return 1;
		}
	}

	return 0;
}

int MonsGenEngine::GetGenRecordCount()
{
	return m_nRecordCount;
}

void MonsGenEngine::Clear()
{
	MonsGenRecordVector::iterator it = m_xMonsGenRecords.begin();
	for (it;
		it != m_xMonsGenRecords.end();
		++it)
	{
		MonsGenRecord* pRecord = *it;
		if (NULL == pRecord)
		{
			continue;
		}
		delete pRecord;
		pRecord = NULL;
	}
	m_xMonsGenRecords.clear();
	m_xMonsGenTotalCounter.Reset();
	m_nRecordCount = 0;
}

int MonsGenEngine::DoGen(const MonsCountRecorder& _refCurrentMonsCount)
{
	if (m_xMonsGenRecords.size() == 0)
	{
		return 0;
	}
	if (!m_cbCanThrough)
	{
		return 0;
	}
	if (!m_cbGenMons)
	{
		return 0;
	}
	if (!m_cbGenMultiple) {
		return 0;
	}

	time_t tn;
	time(&tn);
	int nTotalGenCount = 0;
	MonsCountRecorder xCurrentGenCountMap;

	// generate random index
	std::vector<int> xRamdonIndexes(m_xMonsGenRecords.size(), 0);
	for (size_t i = 0; i < m_xMonsGenRecords.size(); ++i)
	{
		xRamdonIndexes[i] = i;
	}
	random_shuffle(xRamdonIndexes.begin(), xRamdonIndexes.end());

	for (size_t i = 0; i < xRamdonIndexes.size(); ++i)
	{
		int nRecordIndex = xRamdonIndexes[i];

		MonsGenRecord* pRecord = m_xMonsGenRecords[nRecordIndex];
		if (NULL == pRecord)
		{
			continue;
		}

		if (0 != pRecord->nInterval)
		{
			if (tn - pRecord->nLastGenTime < pRecord->nInterval)
			{
				// wait
				continue;
			}
		}
		pRecord->nLastGenTime = tn;

		// gen mons
		int nMonsExistsCount = 0;
		// find in already exists map
		nMonsExistsCount += _refCurrentMonsCount.Get(pRecord->nMonsID);
		// find in gen map
		nMonsExistsCount += xCurrentGenCountMap.Get(pRecord->nMonsID);

		// gen monster
		int nFinalGenCount = GenWithRecord(nMonsExistsCount, pRecord);
		if (0 != nFinalGenCount)
		{
			// record the gen count
			xCurrentGenCountMap.Inc(pRecord->nMonsID, nFinalGenCount);
			nTotalGenCount += nFinalGenCount;
		}
	}

	return nTotalGenCount;
}

int MonsGenEngine::GenWithRecord(int _nExistsCount, const MonsGenRecord* _pRecord)
{
	// gen mons
	int nCanGenMonsCount = 0;
	int nMonsExistsCount = _nExistsCount;
	int nTotalCanGenCount = 0;
	// find total count
	nTotalCanGenCount = m_xMonsGenTotalCounter.Get(_pRecord->nMonsID);
	if (!_pRecord->bBoss) {
		float fMulti = m_cbGenMultiple();
		if (fMulti > 0.09f) {
			if (fMulti > 4.0f) {
				fMulti = 4.0f;
			}
			nTotalCanGenCount = int((1.0f + fMulti) * float(nTotalCanGenCount));
		}
	}

	nCanGenMonsCount = nTotalCanGenCount - nMonsExistsCount;
	if (nCanGenMonsCount <= 0)
	{
		// mons count over limit
		return 0;
	}
	if (nCanGenMonsCount > _pRecord->nCount)
	{
		nCanGenMonsCount = _pRecord->nCount;
	}

	// gen monster
	int nFinalGenCount = 0;

	for (int i = 0; i < nCanGenMonsCount; ++i)
	{
		int nGenPosX = _pRecord->nPosX - _pRecord->nOffset + 2 * (rand() % (_pRecord->nOffset + 1));
		int nGenPosY = _pRecord->nPosY - _pRecord->nOffset + 2 * (rand() % (_pRecord->nOffset + 1));
		int nLoopCounter = 0;
		bool bCanGen = false;

		do
		{
			if (m_cbCanThrough(nGenPosX, nGenPosY))
			{
				// pos ok
				bCanGen = true;
				break;
			}

			++nLoopCounter;
			if (nLoopCounter > 50)
			{
				break;
			}
			// get next gen pos
			nGenPosX = _pRecord->nPosX - _pRecord->nOffset + 2 * (rand() % (_pRecord->nOffset + 1));
			nGenPosY = _pRecord->nPosY - _pRecord->nOffset + 2 * (rand() % (_pRecord->nOffset + 1));

		} while (1);

		if (bCanGen &&
			0 != m_cbGenMons(_pRecord->nMonsID, _pRecord->nGenType, nGenPosX, nGenPosY))
		{
			++nFinalGenCount;
		}
	}
	return nFinalGenCount;
}






//////////////////////////////////////////////////////////////////////////
// MonsExistCountRecorder
MonsCountRecorder::MonsCountRecorder()
{

}

MonsCountRecorder::~MonsCountRecorder()
{

}

void MonsCountRecorder::Reset()
{
	MonsGenCountMap::iterator it = m_xCountMap.begin();
	for (it;
		it != m_xCountMap.end();
		++it)
	{
		// just reset to zero
		it->second = 0;
	}
}

void MonsCountRecorder::Set(int _nKey, int _nValue)
{
	MonsGenCountMap::iterator it = m_xCountMap.find(_nKey);
	if (it != m_xCountMap.end())
	{
		it->second = _nValue;
	}
	else
	{
		m_xCountMap.insert(std::make_pair(_nKey, _nValue));
	}
}

void MonsCountRecorder::Inc(int _nKey, int _nValue)
{
	MonsGenCountMap::iterator it = m_xCountMap.find(_nKey);
	if (it != m_xCountMap.end())
	{
		it->second += _nValue;
	}
	else
	{
		m_xCountMap.insert(std::make_pair(_nKey, _nValue));
	}
}

int MonsCountRecorder::Get(int _nKey) const
{
	MonsGenCountMap::const_iterator it = m_xCountMap.find(_nKey);
	if (it != m_xCountMap.end())
	{
		return it->second;
	}
	return 0;
}