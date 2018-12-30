#include "IndexManager.h"
//////////////////////////////////////////////////////////////////////////
unsigned int IndexManager::s_uInvalidIndex = 0;
//////////////////////////////////////////////////////////////////////////
//	ASM source
static void __declspec(naked) InitializeIndexDesc(void* pEntry,DWORD dwNum)
{
	__asm
	{
		enter		0,0

			push		esi
			push		edi
			push		eax
			push		edx
			push		ecx

			xor			eax,eax
			xor			edx,edx
			inc			eax
			mov			edi,dword ptr[pEntry]
		mov			ecx,dword ptr[dwNum]
		mov			esi,edi
			add			edi,INDEX_DESC_SIZE
			dec			ecx

lb_loop:
		mov			dword ptr[edi-8],eax
			mov			dword ptr[edi-4],edi
			inc			eax
			add			edi,INDEX_DESC_SIZE

			loop		lb_loop

			mov			dword ptr[edi-4],edx
			mov			dword ptr[edi-8],eax

			pop			ecx
			pop			edx
			pop			eax
			pop			edi
			pop			esi

			leave
			ret 
	}
}

static DWORD __stdcall AllocIndex(IndexManager* pIM)
{
	DWORD	dwResult;
	__asm
	{
		xor			eax,eax
			mov			ebx,dword ptr[pIM]
		mov			esi,dword ptr[ebx+4]		; m_pBaseDesc
			or			esi,esi
			jz			lb_return

			mov			edx,dword ptr[esi]			; result dwIndex
			mov			edi,dword ptr[ebx+8]		; m_ppIndexDescTable
			mov			eax,dword ptr[esi+4]		; pDesc->m_pNext
			mov			dword ptr[ebx+4],eax		; m_pBaseDesc = pDesc->m_pNext


			; if (m_pBaseDesc == NULL)
			or			eax,eax
			jnz			lb_skip

			; m_pTailDesc = m_pBaseDesc
			mov			dword ptr[ebx+20],eax
lb_skip:



		mov			eax,edx						; backup result
			shl			edx,2						; * 4
			add			edi,edx
			mov			dword ptr[edi],esi			; m_ppIndexDescTable[dwIndex] = pDesc(result)
			inc			dword ptr[ebx+12]
lb_return:
		mov			dword ptr[dwResult],eax
	}
	return dwResult;
}

static void __stdcall FreeIndex(IndexManager* pIM,DWORD dwIndex)
{
	__asm
	{

		xor			edx,edx
			mov			ebx,dword ptr[pIM]
		mov			eax,dword ptr[dwIndex]
		mov			edi,dword ptr[ebx+8]		; m_ppIndexDescTable
			shl			eax,2
			add			edi,eax
			mov			esi,dword ptr[edi]			; pDesc

			or			esi,esi
			jz			lb_return

			mov			dword ptr[edi],edx			; m_ppIndexDescTable[dwIndex] = NULL
			mov			dword ptr[esi+4],edx		; pDesc->m_pNext = NULL

			mov			edi,dword ptr[ebx+20]		; m_pTailDesc

			; if (m_pTailDesc == NULL)
			or			edi,edi
			jnz			lb_not_null

			; m_pTailDesc = m_pBaseDesc = pDesc
			mov			dword ptr[ebx+4],esi
			mov			dword ptr[ebx+20],esi
			jmp			lb_end

lb_not_null:
		mov			dword ptr[edi+4],esi		; m_pTailDesc->pNext = pDesc
			mov			dword ptr[ebx+20],esi		; m_pTailDesc = pDesc

lb_end:

		dec			dword ptr[ebx+12]
lb_return:

	}
}
//////////////////////////////////////////////////////////////////////////
IndexManager::IndexManager()
{
	m_pIndexList = NULL;
	m_pBaseDesc = NULL;
	m_ppIndexDescTable = NULL;
	m_dwIndexNum = 0;
	m_dwMaxIndexNum = 0;
	m_pTailDesc = NULL;
}

IndexManager::~IndexManager()
{

}

void IndexManager::Init(size_t _uSize)
{
	m_dwMaxIndexNum = _uSize;
	m_pIndexList = new INDEX_DESC[_uSize];
	InitializeIndexDesc(m_pIndexList,_uSize);
	m_pBaseDesc = m_pIndexList;
	m_pTailDesc = m_pIndexList+_uSize-1;

	m_ppIndexDescTable = new INDEX_DESC*[_uSize+1];
	memset(m_ppIndexDescTable,0,sizeof(INDEX_DESC*)*(_uSize+1));
}

unsigned int IndexManager::Pop()
{
	unsigned int uIndex = AllocIndex(this);
	return uIndex;
}

void IndexManager::Push(unsigned int _uIndex)
{
	FreeIndex(this, DWORD(_uIndex));
}