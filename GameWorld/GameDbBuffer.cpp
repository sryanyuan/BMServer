#include "GameDbBuffer.h"
#include "DBThread.h"
//////////////////////////////////////////////////////////////////////////
DataRecordList g_xItemRecordList;
DataRecordList g_xMonsterRecordList;

//////////////////////////////////////////////////////////////////////////
bool CreateGameDbBuffer()
{
	bool bRet = true;
	const char* pszItemSQLExpr = "select * from Items";
	if(!DBThread::GetInstance()->ExtendOperation(pszItemSQLExpr, &DBItemAttribLoadCallBack, NULL))
	{
		bRet = false;
	}
	const char* pszMonsSQLExpr = "select * from Monsters";
	if(!DBThread::GetInstance()->ExtendOperation(pszMonsSQLExpr, &DBMonsAttribLoadCallBack, NULL))
	{
		bRet = false;
	}

	return bRet;
}

void ReleaseGameDbBuffer()
{
	if(!g_xItemRecordList.empty())
	{
		g_xItemRecordList.clear();
	}
	if(!g_xMonsterRecordList.empty())
	{
		g_xMonsterRecordList.clear();
	}
}

bool GetRecordInItemTable(int _id, ItemAttrib* _pOut)
{
	if(NULL == _pOut)
	{
		return false;
	}

	ItemAttrib item;
	item.id = 0;
	ObjectValid::SetItemID(&item, _id);

	DataRecordList::const_iterator fnditer = g_xItemRecordList.find(item.id);
	if(fnditer != g_xItemRecordList.end())
	{
		const ItemAttrib* pSrc = &(fnditer->second);
		memcpy(_pOut, pSrc, sizeof(ItemAttrib));
		ObjectValid::DecryptAttrib(_pOut);
		return true;
	}

	return false;
}

bool GetRecordInMonsterTable(int _id, ItemAttrib* _pOut)
{
	if(NULL == _pOut)
	{
		return false;
	}

	ItemAttrib item;
	item.id = 0;
	ObjectValid::SetItemID(&item, _id);

	DataRecordList::const_iterator fnditer = g_xMonsterRecordList.find(item.id);
	if(fnditer != g_xMonsterRecordList.end())
	{
		const ItemAttrib* pSrc = &(fnditer->second);
		memcpy(_pOut, pSrc, sizeof(ItemAttrib));
		ObjectValid::DecryptAttrib(_pOut);
		return true;
	}

	return false;
}

int __cdecl DBItemAttribLoadCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	int nId = atoi(_pValue[0]);
	DataRecordList::iterator fnditer = g_xItemRecordList.find(nId);
	if(fnditer != g_xItemRecordList.end())
	{
		//	Already exists, ignore it...
		return 0;
	}
	ItemAttrib item;
	ZeroMemory(&item, sizeof(ItemAttrib));
	ItemAttrib* pItem = &item;

	assert(_nCount == ITEMATTRIB_COL);

	pItem->id		 = atoi(_pValue[0]);
	strcpy(pItem->name, _pValue[1]);
	pItem->lucky	 = atoi(_pValue[2]);
	pItem->curse	 = atoi(_pValue[3]);
	pItem->hide		 = atoi(_pValue[4]);
	pItem->accuracy	 = atoi(_pValue[5]);
	pItem->atkSpeed	 = atoi(_pValue[6]);
	pItem->atkPalsy  = atoi(_pValue[7]);
	pItem->atkPois  = atoi(_pValue[8]);
	pItem->moveSpeed  = atoi(_pValue[9]);
	pItem->weight  = atoi(_pValue[10]);
	pItem->reqType  = atoi(_pValue[11]);
	pItem->reqValue  = atoi(_pValue[12]);
	pItem->sex  = atoi(_pValue[13]);
	pItem->type  = atoi(_pValue[14]);
	pItem->maxDC  = atoi(_pValue[15]);
	pItem->DC  = atoi(_pValue[16]);
	pItem->maxAC  = atoi(_pValue[17]);
	pItem->AC  = atoi(_pValue[18]);
	pItem->maxMAC  = atoi(_pValue[19]);
	pItem->MAC  = atoi(_pValue[20]);
	pItem->maxSC  = atoi(_pValue[21]);
	pItem->SC  = atoi(_pValue[22]);
	pItem->maxMC  = atoi(_pValue[23]);
	pItem->MC  = atoi(_pValue[24]);
	pItem->maxHP  = atoi(_pValue[25]);
	pItem->HP  = atoi(_pValue[26]);
	pItem->maxMP  = atoi(_pValue[27]);
	pItem->MP  = atoi(_pValue[28]);
	pItem->maxEXPR	= atoi(_pValue[29]);
	pItem->EXPR  = atoi(_pValue[30]);
	pItem->level  = atoi(_pValue[31]);
	pItem->extra  = atoi(_pValue[32]);
	pItem->tex  = atoi(_pValue[33]);
	pItem->price = atoi(_pValue[34]);

	//	We encrypt it to avoid being cheated by players
	ObjectValid::EncryptAttrib(pItem);
	g_xItemRecordList.insert(std::make_pair(pItem->id, item));

	return 0;
}

int __cdecl DBMonsAttribLoadCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName)
{
	int nId = atoi(_pValue[0]);
	DataRecordList::iterator fnditer = g_xMonsterRecordList.find(nId);
	if(fnditer != g_xMonsterRecordList.end())
	{
		//	Already exists, ignore it...
		return 0;
	}
	ItemAttrib item;
	ZeroMemory(&item, sizeof(ItemAttrib));
	ItemAttrib* pItem = &item;

	assert(_nCount == ITEMATTRIB_COL);

	pItem->id		 = atoi(_pValue[0]);
	strcpy(pItem->name, _pValue[1]);
	pItem->lucky	 = atoi(_pValue[2]);
	pItem->curse	 = atoi(_pValue[3]);
	pItem->hide		 = atoi(_pValue[4]);
	pItem->accuracy	 = atoi(_pValue[5]);
	pItem->atkSpeed	 = atoi(_pValue[6]);
	pItem->atkPalsy  = atoi(_pValue[7]);
	pItem->atkPois  = atoi(_pValue[8]);
	pItem->moveSpeed  = atoi(_pValue[9]);
	pItem->weight  = atoi(_pValue[10]);
	pItem->reqType  = atoi(_pValue[11]);
	pItem->reqValue  = atoi(_pValue[12]);
	pItem->sex  = atoi(_pValue[13]);
	pItem->type  = atoi(_pValue[14]);
	pItem->maxDC  = atoi(_pValue[15]);
	pItem->DC  = atoi(_pValue[16]);
	pItem->maxAC  = atoi(_pValue[17]);
	pItem->AC  = atoi(_pValue[18]);
	pItem->maxMAC  = atoi(_pValue[19]);
	pItem->MAC  = atoi(_pValue[20]);
	pItem->maxSC  = atoi(_pValue[21]);
	pItem->SC  = atoi(_pValue[22]);
	pItem->maxMC  = atoi(_pValue[23]);
	pItem->MC  = atoi(_pValue[24]);
	pItem->maxHP  = atoi(_pValue[25]);
	pItem->HP  = atoi(_pValue[26]);
	pItem->maxMP  = atoi(_pValue[27]);
	pItem->MP  = atoi(_pValue[28]);
	pItem->maxEXPR	= atoi(_pValue[29]);
	pItem->EXPR  = atoi(_pValue[30]);
	pItem->level  = atoi(_pValue[31]);
	pItem->extra  = atoi(_pValue[32]);
	pItem->tex  = atoi(_pValue[33]);
	pItem->price = atoi(_pValue[34]);

	ObjectValid::EncryptAttrib(pItem);
	g_xMonsterRecordList.insert(std::make_pair(pItem->id, item));

	return 0;
}