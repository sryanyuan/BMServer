#include "../common/glog.h"
#include "GameDbBuffer.h"
#include "DBThread.h"
#include "../../CommonModule/LuaDataLoader.h"
#include "ObjectEngine.h"
//////////////////////////////////////////////////////////////////////////
std::map<int, ItemFullAttrib> g_xItemFullAttribMap;
std::map<int, MonsFullAttrib> g_xMonsFullAttribMap;
std::map<int, ItemExtraAttribList*> g_xItemExtraSuitAttribMap;
std::vector<HeroBaseInfo> g_xHeroBaseInfo;
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

bool CreateGameDbBufferLua(lua_State *L, bool bUsingLuaHeroBaseInfo) {
	bool res = LuaDataLoader::LoadItemAttrib(L, pszDefaultItemFullAttribTableName, g_xItemFullAttribMap);
	if (!res) {
		LOG(ERROR) << "Error on loading item attrib from lua";
		return res;
	}
	for (auto &pa : g_xItemFullAttribMap) {
		ObjectValid::EncryptAttrib(&pa.second.baseAttrib);
	}
	// Initialize item map to find id for drop items
	std::map<string, int> itemNameMap;
	char szName[20];
	for (auto &pa : g_xItemFullAttribMap) {
		ObjectValid::GetItemName(&pa.second.baseAttrib, szName);
		if (0 == strcmp(szName, "½ð±Ò")) {
			continue;
		}
		int nId = GETITEMATB(&pa.second.baseAttrib, ID);
		string s = szName;
		itemNameMap[std::move(s)] = nId;
	}
	for (int i = 0; i < 5; i++) {
		sprintf(szName, "½ð±Ò%d", i + 1);
		string s = szName;
		itemNameMap[std::move(s)] = 112 + i;
	}

	res = LuaDataLoader::LoadMonsAttrib(L, pszDefaultMonsFullAttribTableName, g_xMonsFullAttribMap);
	if (!res) {
		LOG(ERROR) << "Error on loading monster attrib from lua";
		return false;
	}
	for (auto &pa : g_xMonsFullAttribMap) {
		ObjectValid::EncryptAttrib(&pa.second.baseAttrib);
	}
	// Find drop item id
	for (auto &pa : g_xMonsFullAttribMap) {
		MonsFullAttrib &fa = pa.second;
		for (auto &di : fa.xDropItems) {
			auto it = itemNameMap.find(di.strItemName);
			if (it == itemNameMap.end()) {
				g_xConsole.CPrint("Can't find item %s in item name map", di.strItemName.c_str());
				return false;
			}
			di.nItemId = it->second;
			di.strItemName.clear();
		}
	}
	// Initialize item extra suit attrib
	if (!LuaDataLoader::LoadSuitAttrib(L, pszDefaultSuitAttribTableName, g_xItemExtraSuitAttribMap)) {
		LOG(ERROR) << "Error on loading extra suit attrib from lua";
		return false;
	}
	for (auto &pa : g_xItemExtraSuitAttribMap) {
#ifdef NDEBUG
		memset(pa.second->szSuitChName, 0, sizeof(pa.second->szSuitChName));
#endif
	}
	// Initialize hero base attrib
	if (bUsingLuaHeroBaseInfo) {
		g_xHeroBaseInfo.resize(MAX_LEVEL);
		if (!LuaDataLoader::LoadHeroBaseAttrib(L, pszDefaultHeroBaseAttirbTableName, g_xHeroBaseInfo)) {
			LOG(ERROR) << "Error on loading hero base attrib from lua";
			return false;
		}
		if (!compareLuaHeroBaseInfo()) {
			LOG(ERROR) << "Compare lua hero base info with share data failed";
			return false;
		}
	}
	
	return true;
}

bool compareLuaHeroBaseInfo() {
	for (int i = 1; i <= MAX_LEVEL; i++) {
		HeroBaseInfo heroBaseInfo;
		if (!GetRecordInHeroBaseAttribTable(i, &heroBaseInfo)) {
			return false;
		}

		for (int job = 0; job < 3; job++) {
			if (GetGlobalHP(i, job) != heroBaseInfo.GetHP(job)) {
				return false;
			}
			if (GetGlobalMP(i, job) != heroBaseInfo.GetMP(job)) {
				return false;
			}
			if (GetGlobalWanLi(i, job) != heroBaseInfo.GetWanli(job)) {
				return false;
			}
		}
	}
	return true;
}

int GetHeroBaseAttribExpr(int nLevel) {
	HeroBaseInfo baseInfo;
	if (!GetRecordInHeroBaseAttribTable(nLevel, &baseInfo)) {
		LOG(ERROR) << "Failed to get hero base expr";
		return 0;
	}
	return baseInfo.GetExpr();
}

int GetHeroBaseAttribHP(int nLevel, int nJob) {
	HeroBaseInfo baseInfo;
	if (!GetRecordInHeroBaseAttribTable(nLevel, &baseInfo)) {
		LOG(ERROR) << "Failed to get hero base hp";
		return 0;
	}
	return baseInfo.GetHP(nJob);
}

int GetHeroBaseAttribMP(int nLevel, int nJob) {
	HeroBaseInfo baseInfo;
	if (!GetRecordInHeroBaseAttribTable(nLevel, &baseInfo)) {
		LOG(ERROR) << "Failed to get hero base mp";
		return 0;
	}
	return baseInfo.GetMP(nJob);
}

int GetHeroBaseAttribWanLi(int nLevel, int nJob) {
	HeroBaseInfo baseInfo;
	if (!GetRecordInHeroBaseAttribTable(nLevel, &baseInfo)) {
		LOG(ERROR) << "Failed to get hero base wanli";
		return 0;
	}
	return baseInfo.GetWanli(nJob);
}

bool compareLuaItem() {
	for (auto &item : g_xItemRecordList) {
		auto it = g_xItemFullAttribMap.find(item.first);
		if (it == g_xItemFullAttribMap.end()) {
			g_xConsole.CPrint("Item %d not found", item.first);
			return false;
		}
		if (!(item.second == it->second.baseAttrib)) {
			g_xConsole.CPrint("Item %d not equal", item.first);
			ItemAttrib l = item.second;
			ObjectValid::DecryptAttrib(&l);
			ItemAttrib r = it->second.baseAttrib;
			ObjectValid::DecryptAttrib(&r);
			return false;
		}
	}
	for (auto &mons : g_xMonsterRecordList) {
		auto it = g_xMonsFullAttribMap.find(mons.first);
		if (it == g_xMonsFullAttribMap.end()) {
			g_xConsole.CPrint("Mons %d not found", mons.first);
			return false;
		}
		if (!(mons.second == it->second.baseAttrib)) {
			g_xConsole.CPrint("Mons %d not equal", mons.first);
			ItemAttrib l = mons.second;
			ObjectValid::DecryptAttrib(&l);
			ItemAttrib r = it->second.baseAttrib;
			ObjectValid::DecryptAttrib(&r);
			return false;
		}
	}
	return true;
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

void ReleaseGameDbBufferLua() {
	g_xItemFullAttribMap.clear();
}

bool GetRecordsInMonsDropTable(int id, MonsDropItemInfoVec **pVec) {
	auto it = g_xMonsFullAttribMap.find(id);
	if (it == g_xMonsFullAttribMap.end()) {
		// Not found
		return true;
	}
	*pVec = &it->second.xDropItems;
	return true;
}

int GetItemGradeInFullAttrib(int id) {
	auto it = g_xItemFullAttribMap.find(id);
	if (it != g_xItemFullAttribMap.end()) {
		return it->second.nGrade;
	}
	return 0;
}

ItemExtraAttribList* GetItemExtraSuitAttribList(int nSuitID) {
	auto it = g_xItemExtraSuitAttribMap.find(nSuitID);
	if (it == g_xItemExtraSuitAttribMap.end()) {
		return nullptr;
	}
	return it->second;
}

bool GetRecordInItemTable(int _id, ItemAttrib* _pOut)
{
	if(NULL == _pOut)
	{
		return false;
	}

	if (!g_xItemFullAttribMap.empty()) {
		auto it = g_xItemFullAttribMap.find(_id);
		if (it == g_xItemFullAttribMap.end()) {
			return false;
		}
		memcpy(_pOut, &it->second.baseAttrib, sizeof(ItemAttrib));
		ObjectValid::DecryptAttrib(_pOut);
		return true;
	}

	ItemAttrib item;
	item.id = 0;
	ObjectValid::SetItemID(&item, _id);

	DataRecordList::const_iterator fnditer = g_xItemRecordList.find(_id);
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

	if (!g_xMonsFullAttribMap.empty()) {
		auto it = g_xMonsFullAttribMap.find(_id);
		if (it == g_xMonsFullAttribMap.end()) {
			return false;
		}
		memcpy(_pOut, &it->second.baseAttrib, sizeof(ItemAttrib));
		ObjectValid::DecryptAttrib(_pOut);
		return true;
	}

	ItemAttrib item;
	item.id = 0;
	ObjectValid::SetItemID(&item, _id);

	DataRecordList::const_iterator fnditer = g_xMonsterRecordList.find(_id);
	if(fnditer != g_xMonsterRecordList.end())
	{
		const ItemAttrib* pSrc = &(fnditer->second);
		memcpy(_pOut, pSrc, sizeof(ItemAttrib));
		ObjectValid::DecryptAttrib(_pOut);
		return true;
	}

	return false;
}

bool GetRecordInHeroBaseAttribTable(int nLevel, HeroBaseInfo *_pAttrib) {
	if (nullptr == _pAttrib) {
		return false;
	}
	if (nLevel > MAX_LEVEL || nLevel <= 0) {
		return false;
	}

	if (g_xHeroBaseInfo.empty()) {
		// Using the share data
		_pAttrib->nExpr = GetHeroBaseAttribExpr(nLevel);
		for (int i = 0; i < 3; i++) {
			_pAttrib->nHP[i] = GetGlobalHP(nLevel, i);
			_pAttrib->nMP[i] = GetGlobalMP(nLevel, i);
			_pAttrib->nWanli[i] = GetGlobalWanLi(nLevel, i);
		}
		
		return true;
	}
	
	memcpy(_pAttrib, &g_xHeroBaseInfo[nLevel - 1], sizeof(HeroBaseInfo));
	return true;
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
	memset(&item, 0, sizeof(ItemAttrib));
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
	g_xItemRecordList.insert(std::make_pair(nId, item));

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
	memset(&item, 0, sizeof(ItemAttrib));
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
	g_xMonsterRecordList.insert(std::make_pair(nId, item));

	return 0;
}