#ifndef _INC_GAMEDBBUFFER_
#define _INC_GAMEDBBUFFER_
//////////////////////////////////////////////////////////////////////////
#include "../../CommonModule/ObjectData.h"
#include "ObjectValid.h"
#include <map>
//////////////////////////////////////////////////////////////////////////
struct lua_State;
typedef std::map<int, ItemAttrib> DataRecordList;
struct ItemExtraAttribList;
struct HeroBaseInfo;
//////////////////////////////////////////////////////////////////////////
bool CreateGameDbBuffer();
void ReleaseGameDbBuffer();

bool CreateGameDbBufferLua(lua_State *L, bool bUsingLuaHeroBaseInfo);
void ReleaseGameDbBufferLua();
bool compareLuaItem();
bool compareLuaHeroBaseInfo();

bool GetRecordInItemTable(int _id, ItemAttrib* _pOut);

bool GetRecordInMonsterTable(int id, ItemAttrib *pItem);

bool GetRecordsInMonsDropTable(int id, MonsDropItemInfoVec **pVec);

bool GetRecordInHeroBaseAttribTable(int nLevel, HeroBaseInfo *_pAttrib);
int GetHeroBaseAttribExpr(int nLevel);
int GetHeroBaseAttribHP(int nLevel, int nJob);
int GetHeroBaseAttribMP(int nLevel, int nJob);
int GetHeroBaseAttribWanLi(int nLevel, int nJob);

int GetItemGradeInFullAttrib(int id);

ItemExtraAttribList* GetItemExtraSuitAttribList(int nSuitID);

bool IsSuitIDIgnore(int _nSuitID);
void GetSuitIDIgnore(std::set<int> &refVals);

int __cdecl DBItemAttribLoadCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName);
int __cdecl DBMonsAttribLoadCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName);
//////////////////////////////////////////////////////////////////////////
#endif
