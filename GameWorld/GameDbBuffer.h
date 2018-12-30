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
//////////////////////////////////////////////////////////////////////////
bool CreateGameDbBuffer();
void ReleaseGameDbBuffer();

bool CreateGameDbBufferLua(lua_State *L);
void ReleaseGameDbBufferLua();
bool compareLuaItem();

bool GetRecordInItemTable(int _id, ItemAttrib* _pOut);

bool GetRecordInMonsterTable(int id, ItemAttrib *pItem);

bool GetRecordsInMonsDropTable(int id, MonsDropItemInfoVec **pVec);

int GetItemGradeInFullAttrib(int id);

ItemExtraAttribList* GetItemExtraSuitAttribList(int nSuitID);

int __cdecl DBItemAttribLoadCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName);
int __cdecl DBMonsAttribLoadCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName);
//////////////////////////////////////////////////////////////////////////
#endif
