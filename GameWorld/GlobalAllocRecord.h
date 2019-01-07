#ifndef _INC_GLOBALALLOCRECORD_
#define _INC_GLOBALALLOCRECORD_

#include <set>
#include <mutex>

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
		
	}
	~GlobalAllocRecord(){
		
	}

public:
	void Record(void* _pPtr) {
		std::unique_lock<std::mutex> locker(m_cs);
		m_xPtrs.insert(_pPtr);
	}
	void RecordArray(void* _pPtr) {
		std::unique_lock<std::mutex> locker(m_cs);
		m_xArrayPtrs.insert(_pPtr);
	}

	void DeleteAll() {
		std::unique_lock<std::mutex> locker(m_cs);

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
	}

protected:
	static GlobalAllocRecord* s_pGlobalAllocRecord;

private:
	AllocPtrList m_xPtrs;
	AllocPtrList m_xArrayPtrs;
	std::mutex m_cs;
};

#endif