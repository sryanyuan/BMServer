#ifndef _INC_INDEXMANAGER_
#define _INC_INDEXMANAGER_
//////////////////////////////////////////////////////////////////////////
#include <vector>
#include "namespace.h"
//////////////////////////////////////////////////////////////////////////
IONS_START

typedef std::vector<unsigned int> IndexContainer;

#ifndef DWORD
#define DWORD unsigned long
#endif

struct INDEX_DESC
{
	unsigned int			m_dwIndex;
	INDEX_DESC*		m_pNext;
};

#define		INDEX_DESC_SIZE		8
//////////////////////////////////////////////////////////////////////////
class IndexManager
{
public:
	IndexManager();
	~IndexManager();

public:
	void Init(size_t _uSize);
	unsigned int Pop();
	void Push(unsigned int _uIndex);

public:
	static unsigned int s_uInvalidIndex;

private:
	INDEX_DESC*			m_pIndexList;
	INDEX_DESC*			m_pBaseDesc;			// 4
	INDEX_DESC**		m_ppIndexDescTable;		// 8
	unsigned int				m_dwIndexNum;			// 12
	unsigned int				m_dwMaxIndexNum;		// 16
	INDEX_DESC*			m_pTailDesc;			// 20
};

IONS_END
//////////////////////////////////////////////////////////////////////////
#endif