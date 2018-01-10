#ifndef _INC_GLOBALALLOCRECORD_
#define _INC_GLOBALALLOCRECORD_

#include <set>
#include <Windows.h>

typedef std::set<void*> AllocPtrList;

class GlobalAllocRecord {
public:
	static GlobalAllocRecord* GetInstance() {
		if (NULL == s_pGlobalAllocRecord) {
			s_pGlobalAllocRecord = new GlobalAllocRecord;
		}
		return s_pGlobalAllocRecord;
	}
	static void DestroyInstance() {
		delete s_pGlobalAllocRecord;
		s_pGlobalAllocRecord = NULL;
	}

protected:
	GlobalAllocRecord(){
		InitializeCriticalSection(&m_cs);	
	}
	~GlobalAllocRecord(){
		DeleteCriticalSection(&m_cs);
	}

public:
	void Record(void* _pPtr) {
		EnterCriticalSection(&m_cs);
		m_xPtrs.insert(_pPtr);
		LeaveCriticalSection(&m_cs);
	}
	void RecordArray(void* _pPtr) {
		EnterCriticalSection(&m_cs);
		m_xArrayPtrs.insert(_pPtr);
		LeaveCriticalSection(&m_cs);
	}

	void DeleteAll() {
		EnterCriticalSection(&m_cs);

		AllocPtrList::iterator it = m_xPtrs.begin();
		for (it; it != m_xPtrs.end(); ++it) {
			delete *it;
		}
		m_xPtrs.clear();

		it = m_xArrayPtrs.begin();
		for (it; it != m_xArrayPtrs.end(); ++it) {
			delete[] *it;
		}
		m_xArrayPtrs.clear();
		LeaveCriticalSection(&m_cs);
	}

protected:
	static GlobalAllocRecord* s_pGlobalAllocRecord;

private:
	AllocPtrList m_xPtrs;
	AllocPtrList m_xArrayPtrs;
	CRITICAL_SECTION m_cs;
};

#endif