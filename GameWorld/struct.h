#ifndef _INC_STRUCT_
#define _INC_STRUCT_
//////////////////////////////////////////////////////////////////////////
#include "../../CommonModule/ByteBuffer.h"
//////////////////////////////////////////////////////////////////////////
#define SQL_ITEMATTRIB		0
//////////////////////////////////////////////////////////////////////////
enum DELAYED_PROCESS
{
	DP_SYSTEM_START,
	DP_RELOADSCRIPT,
	DP_GETUSERINFO,
	DP_SENDSYSMSG,
	DP_SYSTEM_END,
	DP_GAME_START,
	DP_USERLOGIN,
	DP_USERLOGOUT,
	DP_GAME_END,
	DP_DATABASE_START,
	DP_ITEMLOADED,
	DP_MONSLOADED,
	DP_DROPITEMLOADED,
	DP_DROPITEMEXLOADED,
	DP_DATABASE_END,
};
//////////////////////////////////////////////////////////////////////////
enum ITEM_EXTRA_OPERATION
{
	IE_NONE = 0,
	IE_ADDPLAYERITEM,
	IE_ADDGROUNDITEM,
	IE_BUYSHOPITEM,
	IE_ADDBALEITEM,
};
enum MONSTER_EXTRA_OPERATION
{
	//ME_NONE = 0,
	ME_MAKESLAVE,
};
//////////////////////////////////////////////////////////////////////////
enum DatabaseOperate
{
	DO_QUERY_BEGIN,
	DO_QUERY_ITEMATTRIB,
	DO_QUERY_MONSATTRIB,
	DO_QUERY_DROPITEM,
	DO_QUERY_DROPITEMEX,
	DO_QUERY_DROPITEMEX2,
	DO_QUERY_RELOADSCRIPT,
	DO_QUERY,
	DO_QUERY_END,
	DO_INSERT_BEGIN,
	DO_INSERT,
	DO_INSERT_END,
	DO_UPATE_BEGIN,
	DO_UPDATE,
	DO_UPDATE_END,
};
//////////////////////////////////////////////////////////////////////////
struct DelayedProcess
{
	unsigned int uOp;
	unsigned int uParam0;
	unsigned int uParam1;
	unsigned int uParam2;
	unsigned int uParam3;
};

struct DelayedDBProcess
{
	unsigned int uOp;
	unsigned int uParam0;
	unsigned int uParam1;
	unsigned int uParam2;
	unsigned int uParam3;
};

BYTEBUFFER_STRUCT_OPERATOR(DelayedProcess);
BYTEBUFFER_STRUCT_OPERATOR(DelayedDBProcess);
//////////////////////////////////////////////////////////////////////////
#endif