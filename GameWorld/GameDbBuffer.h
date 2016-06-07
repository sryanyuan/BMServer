#ifndef _INC_GAMEDBBUFFER_
#define _INC_GAMEDBBUFFER_
//////////////////////////////////////////////////////////////////////////
#include "../../CommonModule/ObjectData.h"
#include "ObjectValid.h"
#include <map>
//////////////////////////////////////////////////////////////////////////
typedef std::map<int, ItemAttrib> DataRecordList;
//////////////////////////////////////////////////////////////////////////
bool CreateGameDbBuffer();
void ReleaseGameDbBuffer();

bool GetRecordInItemTable(int _id, ItemAttrib* _pOut);
bool GetRecordInMonsterTable(int _id, ItemAttrib* _pOut);

int __cdecl DBItemAttribLoadCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName);
int __cdecl DBMonsAttribLoadCallBack(void* _pParam,int _nCount, char** _pValue, char** _pName);
//////////////////////////////////////////////////////////////////////////
#endif