/*
** Lua binding: BackMirServer
** Generated automatically by tolua++-1.0.92 on 02/04/17 18:47:41.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_BackMirServer_open (lua_State* tolua_S);

#include "../CMainServer/CMainServer.h"
#include "../GameWorld/ObjectEngine.h"
#include "../GameWorld/MonsterObject.h"
#include "../../CommonModule/GamePacket.h"
#include "../../CommonModule/StoveManager.h"
#include "../GameWorld/GameSceneManager.h"
#include "../GameWorld/SceneEvent.h"
#include "../GameWorld/GameInstanceScene.h"
#include "../GameWorld/DBThread.h"
#include "../GameWorld/GameWorld.h"
#include "../GameWorld/GameDbBuffer.h"
#include "../GameWorld/LuaServerEngine.h"
#include "../GameWorld/DBDropDownContext.h"
#include "../GameWorld/LuaItemHelper.h"
#include "../GameWorld/OlShopManager.h"
#include "../../CommonModule/QuestContext.h"

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_DWORD (lua_State* tolua_S)
{
 DWORD* self = (DWORD*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_GameSceneManager (lua_State* tolua_S)
{
 GameSceneManager* self = (GameSceneManager*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_WORD (lua_State* tolua_S)
{
 WORD* self = (WORD*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_NPCObject (lua_State* tolua_S)
{
 NPCObject* self = (NPCObject*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_GameObject (lua_State* tolua_S)
{
 GameObject* self = (GameObject*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"OBJECT_STATE");
 tolua_usertype(tolua_S,"GroundItem");
 tolua_usertype(tolua_S,"WORD");
 tolua_usertype(tolua_S,"NPCObject");
 tolua_usertype(tolua_S,"LuaItemHelper");
 tolua_usertype(tolua_S,"CMainServer");
 tolua_usertype(tolua_S,"DWORD");
 tolua_usertype(tolua_S,"GameScene");
 tolua_usertype(tolua_S,"GameWorld");
 tolua_usertype(tolua_S,"OlShopManager");
 tolua_usertype(tolua_S,"DBDropDownContext");
 tolua_usertype(tolua_S,"GameInstanceScene");
 tolua_usertype(tolua_S,"ItemAttrib");
 tolua_usertype(tolua_S,"LuaBaseEngine");
 tolua_usertype(tolua_S,"DBThread");
 tolua_usertype(tolua_S,"USER_STATE");
 tolua_usertype(tolua_S,"GameSceneManager");
 tolua_usertype(tolua_S,"std::list<int>");
 tolua_usertype(tolua_S,"ItemIDList");
 tolua_usertype(tolua_S,"GameObject");
 tolua_usertype(tolua_S,"QuestContext");
 tolua_usertype(tolua_S,"LuaServerEngine");
 tolua_usertype(tolua_S,"PacketHeader");
 tolua_usertype(tolua_S,"MonsterObject");
 tolua_usertype(tolua_S,"LockObject");
 tolua_usertype(tolua_S,"DBOperationParam");
 tolua_usertype(tolua_S,"UserData");
 tolua_usertype(tolua_S,"HeroObject");
 tolua_usertype(tolua_S,"ByteBuffer");
}

/* function: MirLog */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_MirLog00
static int tolua_BackMirServer_MirLog00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* _pLog = ((const char*)  tolua_tostring(tolua_S,1,0));
  {
   MirLog(_pLog);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'MirLog'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: ConsolePrint */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ConsolePrint00
static int tolua_BackMirServer_ConsolePrint00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* _pszText = ((const char*)  tolua_tostring(tolua_S,1,0));
  {
   ConsolePrint(_pszText);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ConsolePrint'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_delete00
static int tolua_BackMirServer_GameObject_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddProcess of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_AddProcess00
static int tolua_BackMirServer_GameObject_AddProcess00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const PacketHeader",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
  const PacketHeader* _pPkt = ((const PacketHeader*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddProcess'", NULL);
#endif
  {
   self->AddProcess(_pPkt);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddProcess'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetID of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetID00
static int tolua_BackMirServer_GameObject_GetID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetID'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->GetID();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetType of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetType00
static int tolua_BackMirServer_GameObject_GetType00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetType'", NULL);
#endif
  {
   SERVER_OBJECT_TYPE tolua_ret = (SERVER_OBJECT_TYPE)  self->GetType();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetType'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetAttrib of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetAttrib00
static int tolua_BackMirServer_GameObject_GetAttrib00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetAttrib'", NULL);
#endif
  {
   ItemAttrib* tolua_ret = (ItemAttrib*)  self->GetAttrib();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ItemAttrib");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetAttrib'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMapID of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetMapID00
static int tolua_BackMirServer_GameObject_GetMapID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMapID'", NULL);
#endif
  {
   WORD tolua_ret = (WORD)  self->GetMapID();
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((WORD)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"WORD");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(WORD));
     tolua_pushusertype(tolua_S,tolua_obj,"WORD");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMapID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetUserData of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetUserData00
static int tolua_BackMirServer_GameObject_GetUserData00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetUserData'", NULL);
#endif
  {
   UserData* tolua_ret = (UserData*)  self->GetUserData();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"UserData");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetUserData'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetLocateScene of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetLocateScene00
static int tolua_BackMirServer_GameObject_GetLocateScene00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetLocateScene'", NULL);
#endif
  {
   GameScene* tolua_ret = (GameScene*)  self->GetLocateScene();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameScene");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetLocateScene'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetAttribID of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetAttribID00
static int tolua_BackMirServer_GameObject_GetAttribID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetAttribID'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetAttribID();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetAttribID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetLevel of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetLevel00
static int tolua_BackMirServer_GameObject_GetLevel00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetLevel'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetLevel();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetLevel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: FlyToInstanceMap of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_FlyToInstanceMap00
static int tolua_BackMirServer_GameObject_FlyToInstanceMap00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,4,"GameInstanceScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
  int _x = ((int)  tolua_tonumber(tolua_S,2,0));
  int _y = ((int)  tolua_tonumber(tolua_S,3,0));
  GameInstanceScene* _pInsScene = ((GameInstanceScene*)  tolua_tousertype(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'FlyToInstanceMap'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->FlyToInstanceMap(_x,_y,_pInsScene);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'FlyToInstanceMap'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: FlyToMap of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_FlyToMap00
static int tolua_BackMirServer_GameObject_FlyToMap00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
  int _x = ((int)  tolua_tonumber(tolua_S,2,0));
  int _y = ((int)  tolua_tonumber(tolua_S,3,0));
  int _nMapID = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'FlyToMap'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->FlyToMap(_x,_y,_nMapID);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'FlyToMap'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IncHP of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_IncHP00
static int tolua_BackMirServer_GameObject_IncHP00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwHP = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IncHP'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IncHP(_dwHP);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IncHP'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: DecHP of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_DecHP00
static int tolua_BackMirServer_GameObject_DecHP00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwHP = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'DecHP'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->DecHP(_dwHP);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DecHP'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IncMP of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_IncMP00
static int tolua_BackMirServer_GameObject_IncMP00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwMP = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IncMP'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IncMP(_dwMP);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IncMP'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: DecMP of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_DecMP00
static int tolua_BackMirServer_GameObject_DecMP00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwMP = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'DecMP'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->DecMP(_dwMP);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DecMP'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetCoordX of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetCoordX00
static int tolua_BackMirServer_GameObject_GetCoordX00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetCoordX'", NULL);
#endif
  {
   WORD tolua_ret = (WORD)  self->GetCoordX();
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((WORD)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"WORD");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(WORD));
     tolua_pushusertype(tolua_S,tolua_obj,"WORD");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetCoordX'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetCoordY of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetCoordY00
static int tolua_BackMirServer_GameObject_GetCoordY00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetCoordY'", NULL);
#endif
  {
   WORD tolua_ret = (WORD)  self->GetCoordY();
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((WORD)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"WORD");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(WORD));
     tolua_pushusertype(tolua_S,tolua_obj,"WORD");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetCoordY'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetCoordXInt of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetCoordXInt00
static int tolua_BackMirServer_GameObject_GetCoordXInt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetCoordXInt'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetCoordXInt();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetCoordXInt'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetCoordYInt of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetCoordYInt00
static int tolua_BackMirServer_GameObject_GetCoordYInt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetCoordYInt'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetCoordYInt();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetCoordYInt'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetState of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetState00
static int tolua_BackMirServer_GameObject_GetState00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetState'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetState();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetState'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetTotalRecvDamage of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetTotalRecvDamage00
static int tolua_BackMirServer_GameObject_GetTotalRecvDamage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetTotalRecvDamage'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetTotalRecvDamage();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetTotalRecvDamage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetHP of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetHP00
static int tolua_BackMirServer_GameObject_GetHP00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetHP'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetHP();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetHP'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMaxHP of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetMaxHP00
static int tolua_BackMirServer_GameObject_GetMaxHP00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMaxHP'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMaxHP();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMaxHP'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetExpr of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetExpr00
static int tolua_BackMirServer_GameObject_GetExpr00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetExpr'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetExpr();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetExpr'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetName of class  GameObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameObject_GetName00
static int tolua_BackMirServer_GameObject_GetName00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameObject* self = (GameObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetName'", NULL);
#endif
  {
   const char* tolua_ret = (const char*)  self->GetName();
   tolua_pushstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetName'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItemByIndex of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetItemByIndex00
static int tolua_BackMirServer_HeroObject_GetItemByIndex00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwIndex = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetItemByIndex'", NULL);
#endif
  {
   ItemAttrib* tolua_ret = (ItemAttrib*)  self->GetItemByIndex(_dwIndex);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ItemAttrib");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItemByIndex'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItemByTag of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetItemByTag00
static int tolua_BackMirServer_HeroObject_GetItemByTag00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwTag = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetItemByTag'", NULL);
#endif
  {
   ItemAttrib* tolua_ret = (ItemAttrib*)  self->GetItemByTag(_dwTag);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ItemAttrib");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItemByTag'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Lua_GetItemByAttribID of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_Lua_GetItemByAttribID00
static int tolua_BackMirServer_HeroObject_Lua_GetItemByAttribID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nId = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Lua_GetItemByAttribID'", NULL);
#endif
  {
   ItemAttrib* tolua_ret = (ItemAttrib*)  self->Lua_GetItemByAttribID(_nId);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ItemAttrib");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Lua_GetItemByAttribID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Lua_GetItemByTag of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_Lua_GetItemByTag00
static int tolua_BackMirServer_HeroObject_Lua_GetItemByTag00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nTag = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Lua_GetItemByTag'", NULL);
#endif
  {
   ItemAttrib* tolua_ret = (ItemAttrib*)  self->Lua_GetItemByTag(_nTag);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ItemAttrib");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Lua_GetItemByTag'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddBagItem of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_AddBagItem00
static int tolua_BackMirServer_HeroObject_AddBagItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  const ItemAttrib* _pItem = ((const ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddBagItem'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->AddBagItem(_pItem);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddBagItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddBagItem of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_AddBagItem01
static int tolua_BackMirServer_HeroObject_AddBagItem01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isusertype(tolua_S,3,"const ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwIndex = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
  const ItemAttrib* _pItem = ((const ItemAttrib*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddBagItem'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->AddBagItem(_dwIndex,_pItem);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_BackMirServer_HeroObject_AddBagItem00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: SendSystemMessage of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_SendSystemMessage00
static int tolua_BackMirServer_HeroObject_SendSystemMessage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  const char* _pszMsg = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SendSystemMessage'", NULL);
#endif
  {
   self->SendSystemMessage(_pszMsg);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SendSystemMessage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ShowQuestDlg of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_ShowQuestDlg00
static int tolua_BackMirServer_HeroObject_ShowQuestDlg00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  NPCObject* _pnpc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
  int _questid = ((int)  tolua_tonumber(tolua_S,3,0));
  int _step = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ShowQuestDlg'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->ShowQuestDlg(_pnpc,_questid,_step);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ShowQuestDlg'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: HideQuestDlg of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_HideQuestDlg00
static int tolua_BackMirServer_HeroObject_HideQuestDlg00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'HideQuestDlg'", NULL);
#endif
  {
   self->HideQuestDlg();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'HideQuestDlg'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ShowShopDlg of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_ShowShopDlg00
static int tolua_BackMirServer_HeroObject_ShowShopDlg00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  NPCObject* _pnpc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
  int _type = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ShowShopDlg'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->ShowShopDlg(_pnpc,_type);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ShowShopDlg'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetQuest of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetQuest00
static int tolua_BackMirServer_HeroObject_GetQuest00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetQuest'", NULL);
#endif
  {
   QuestContext* tolua_ret = (QuestContext*)  self->GetQuest();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"QuestContext");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetQuest'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CountItem of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_CountItem00
static int tolua_BackMirServer_HeroObject_CountItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nAttribID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CountItem'", NULL);
#endif
  {
   int tolua_ret = (int)  self->CountItem(_nAttribID);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CountItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ClearItem of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_ClearItem00
static int tolua_BackMirServer_HeroObject_ClearItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nAttribID = ((int)  tolua_tonumber(tolua_S,2,0));
  int _number = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ClearItem'", NULL);
#endif
  {
   self->ClearItem(_nAttribID,_number);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ClearItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddItem of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_AddItem00
static int tolua_BackMirServer_HeroObject_AddItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nAttribID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddItem'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->AddItem(_nAttribID);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddItemReturnTag of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_AddItemReturnTag00
static int tolua_BackMirServer_HeroObject_AddItemReturnTag00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nAttribID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddItemReturnTag'", NULL);
#endif
  {
   int tolua_ret = (int)  self->AddItemReturnTag(_nAttribID);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddItemReturnTag'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: RemoveItem of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_RemoveItem00
static int tolua_BackMirServer_HeroObject_RemoveItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nTag = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'RemoveItem'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->RemoveItem(_nTag);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'RemoveItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ItemTagToAttribID of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_ItemTagToAttribID00
static int tolua_BackMirServer_HeroObject_ItemTagToAttribID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nTag = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ItemTagToAttribID'", NULL);
#endif
  {
   int tolua_ret = (int)  self->ItemTagToAttribID(_nTag);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ItemTagToAttribID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetBagEmptySum of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetBagEmptySum00
static int tolua_BackMirServer_HeroObject_GetBagEmptySum00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetBagEmptySum'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetBagEmptySum();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetBagEmptySum'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetAssistEmptySum of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetAssistEmptySum00
static int tolua_BackMirServer_HeroObject_GetAssistEmptySum00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetAssistEmptySum'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetAssistEmptySum();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetAssistEmptySum'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SyncItemAttrib of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_SyncItemAttrib00
static int tolua_BackMirServer_HeroObject_SyncItemAttrib00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nTag = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SyncItemAttrib'", NULL);
#endif
  {
   self->SyncItemAttrib(_nTag);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SyncItemAttrib'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMoney of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetMoney00
static int tolua_BackMirServer_HeroObject_GetMoney00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMoney'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMoney();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMoney'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddMoney of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_AddMoney00
static int tolua_BackMirServer_HeroObject_AddMoney00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nMoney = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddMoney'", NULL);
#endif
  {
   self->AddMoney(_nMoney);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddMoney'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: MinusMoney of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_MinusMoney00
static int tolua_BackMirServer_HeroObject_MinusMoney00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nMoney = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'MinusMoney'", NULL);
#endif
  {
   self->MinusMoney(_nMoney);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'MinusMoney'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GainExp of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GainExp00
static int tolua_BackMirServer_HeroObject_GainExp00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _expr = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GainExp'", NULL);
#endif
  {
   self->GainExp(_expr);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GainExp'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetHeroJob of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetHeroJob00
static int tolua_BackMirServer_HeroObject_GetHeroJob00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetHeroJob'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetHeroJob();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetHeroJob'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetHeroSex of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetHeroSex00
static int tolua_BackMirServer_HeroObject_GetHeroSex00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetHeroSex'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetHeroSex();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetHeroSex'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ClearAllItem of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_ClearAllItem00
static int tolua_BackMirServer_HeroObject_ClearAllItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ClearAllItem'", NULL);
#endif
  {
   self->ClearAllItem();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ClearAllItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: FlyToHome of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_FlyToHome00
static int tolua_BackMirServer_HeroObject_FlyToHome00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'FlyToHome'", NULL);
#endif
  {
   self->FlyToHome();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'FlyToHome'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetValidState of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_SetValidState00
static int tolua_BackMirServer_HeroObject_SetValidState00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetValidState'", NULL);
#endif
  {
   self->SetValidState();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetValidState'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SyncQuestData of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_SyncQuestData00
static int tolua_BackMirServer_HeroObject_SyncQuestData00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nStage = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SyncQuestData'", NULL);
#endif
  {
   self->SyncQuestData(_nStage);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SyncQuestData'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetTeamID of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetTeamID00
static int tolua_BackMirServer_HeroObject_GetTeamID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetTeamID'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetTeamID();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetTeamID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetTeamMate of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetTeamMate00
static int tolua_BackMirServer_HeroObject_GetTeamMate00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _idx = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetTeamMate'", NULL);
#endif
  {
   HeroObject* tolua_ret = (HeroObject*)  self->GetTeamMate(_idx);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"HeroObject");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetTeamMate'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: TeamMateFlyToInstanceMap of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_TeamMateFlyToInstanceMap00
static int tolua_BackMirServer_HeroObject_TeamMateFlyToInstanceMap00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,4,"GameInstanceScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _x = ((int)  tolua_tonumber(tolua_S,2,0));
  int _y = ((int)  tolua_tonumber(tolua_S,3,0));
  GameInstanceScene* _pInsScene = ((GameInstanceScene*)  tolua_tousertype(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'TeamMateFlyToInstanceMap'", NULL);
#endif
  {
   self->TeamMateFlyToInstanceMap(_x,_y,_pInsScene);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'TeamMateFlyToInstanceMap'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsTeamLeader of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_IsTeamLeader00
static int tolua_BackMirServer_HeroObject_IsTeamLeader00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsTeamLeader'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsTeamLeader();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsTeamLeader'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Lua_GetActiveDropParam of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_Lua_GetActiveDropParam00
static int tolua_BackMirServer_HeroObject_Lua_GetActiveDropParam00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Lua_GetActiveDropParam'", NULL);
#endif
  {
   int tolua_ret = (int)  self->Lua_GetActiveDropParam();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Lua_GetActiveDropParam'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetChallengeItemID of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_GetChallengeItemID00
static int tolua_BackMirServer_HeroObject_GetChallengeItemID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetChallengeItemID'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetChallengeItemID();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetChallengeItemID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: UseChallengeItem of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_UseChallengeItem00
static int tolua_BackMirServer_HeroObject_UseChallengeItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nClgID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'UseChallengeItem'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->UseChallengeItem(_nClgID);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'UseChallengeItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsTeamMateAround of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_IsTeamMateAround00
static int tolua_BackMirServer_HeroObject_IsTeamMateAround00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nOffset = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsTeamMateAround'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsTeamMateAround(_nOffset);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsTeamMateAround'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: TeamMateFlyToMap of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_TeamMateFlyToMap00
static int tolua_BackMirServer_HeroObject_TeamMateFlyToMap00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _x = ((int)  tolua_tonumber(tolua_S,2,0));
  int _y = ((int)  tolua_tonumber(tolua_S,3,0));
  int _nMapID = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'TeamMateFlyToMap'", NULL);
#endif
  {
   self->TeamMateFlyToMap(_x,_y,_nMapID);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'TeamMateFlyToMap'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ReceiveGift of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_ReceiveGift00
static int tolua_BackMirServer_HeroObject_ReceiveGift00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nGiftID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ReceiveGift'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->ReceiveGift(_nGiftID);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ReceiveGift'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ResetIDlg of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_ResetIDlg00
static int tolua_BackMirServer_HeroObject_ResetIDlg00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ResetIDlg'", NULL);
#endif
  {
   self->ResetIDlg();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ResetIDlg'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddIDlg_CloseButton of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_AddIDlg_CloseButton00
static int tolua_BackMirServer_HeroObject_AddIDlg_CloseButton00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isstring(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nX = ((int)  tolua_tonumber(tolua_S,2,0));
  int _nY = ((int)  tolua_tonumber(tolua_S,3,0));
  const char* _pszText = ((const char*)  tolua_tostring(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddIDlg_CloseButton'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->AddIDlg_CloseButton(_nX,_nY,_pszText);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddIDlg_CloseButton'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddIDlg_Button of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_AddIDlg_Button00
static int tolua_BackMirServer_HeroObject_AddIDlg_Button00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isstring(tolua_S,5,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nX = ((int)  tolua_tonumber(tolua_S,2,0));
  int _nY = ((int)  tolua_tonumber(tolua_S,3,0));
  int _nId = ((int)  tolua_tonumber(tolua_S,4,0));
  const char* _pszText = ((const char*)  tolua_tostring(tolua_S,5,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddIDlg_Button'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->AddIDlg_Button(_nX,_nY,_nId,_pszText);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddIDlg_Button'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddIDlg_String of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_AddIDlg_String00
static int tolua_BackMirServer_HeroObject_AddIDlg_String00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isstring(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nX = ((int)  tolua_tonumber(tolua_S,2,0));
  int _nY = ((int)  tolua_tonumber(tolua_S,3,0));
  const char* _pszText = ((const char*)  tolua_tostring(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddIDlg_String'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->AddIDlg_String(_nX,_nY,_pszText);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddIDlg_String'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ShowIDlg of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_ShowIDlg00
static int tolua_BackMirServer_HeroObject_ShowIDlg00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  NPCObject* _pNpc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ShowIDlg'", NULL);
#endif
  {
   self->ShowIDlg(_pNpc);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ShowIDlg'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: HideIDlg of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_HideIDlg00
static int tolua_BackMirServer_HeroObject_HideIDlg00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'HideIDlg'", NULL);
#endif
  {
   self->HideIDlg();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'HideIDlg'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetEnterTimeLimitScene of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_SetEnterTimeLimitScene00
static int tolua_BackMirServer_HeroObject_SetEnterTimeLimitScene00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  GameScene* _pScene = ((GameScene*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetEnterTimeLimitScene'", NULL);
#endif
  {
   self->SetEnterTimeLimitScene(_pScene);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetEnterTimeLimitScene'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: UpdateLifeSkillLevel of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_UpdateLifeSkillLevel00
static int tolua_BackMirServer_HeroObject_UpdateLifeSkillLevel00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  LifeSkillType _eType = ((LifeSkillType) (int)  tolua_tonumber(tolua_S,2,0));
  int _nLevel = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'UpdateLifeSkillLevel'", NULL);
#endif
  {
   self->UpdateLifeSkillLevel(_eType,_nLevel);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'UpdateLifeSkillLevel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Lua_SetQuestStep of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_Lua_SetQuestStep00
static int tolua_BackMirServer_HeroObject_Lua_SetQuestStep00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nQuestId = ((int)  tolua_tonumber(tolua_S,2,0));
  int _nStep = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Lua_SetQuestStep'", NULL);
#endif
  {
   self->Lua_SetQuestStep(_nQuestId,_nStep);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Lua_SetQuestStep'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Lua_GetQuestStep of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_Lua_GetQuestStep00
static int tolua_BackMirServer_HeroObject_Lua_GetQuestStep00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nQuestId = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Lua_GetQuestStep'", NULL);
#endif
  {
   int tolua_ret = (int)  self->Lua_GetQuestStep(_nQuestId);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Lua_GetQuestStep'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Lua_SetQuestCounter of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_Lua_SetQuestCounter00
static int tolua_BackMirServer_HeroObject_Lua_SetQuestCounter00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nQuestId = ((int)  tolua_tonumber(tolua_S,2,0));
  int _nCounter = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Lua_SetQuestCounter'", NULL);
#endif
  {
   self->Lua_SetQuestCounter(_nQuestId,_nCounter);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Lua_SetQuestCounter'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Lua_GetQuestCounter of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_Lua_GetQuestCounter00
static int tolua_BackMirServer_HeroObject_Lua_GetQuestCounter00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nQuestId = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Lua_GetQuestCounter'", NULL);
#endif
  {
   int tolua_ret = (int)  self->Lua_GetQuestCounter(_nQuestId);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Lua_GetQuestCounter'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: TransferIdentifyAttrib of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_TransferIdentifyAttrib00
static int tolua_BackMirServer_HeroObject_TransferIdentifyAttrib00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nItemTag0 = ((int)  tolua_tonumber(tolua_S,2,0));
  int _nItemTag1 = ((int)  tolua_tonumber(tolua_S,3,0));
  int _nCount = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'TransferIdentifyAttrib'", NULL);
#endif
  {
   int tolua_ret = (int)  self->TransferIdentifyAttrib(_nItemTag0,_nItemTag1,_nCount);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'TransferIdentifyAttrib'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: TransferIdentifyAttribFailed of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_TransferIdentifyAttribFailed00
static int tolua_BackMirServer_HeroObject_TransferIdentifyAttribFailed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  int _nItemTag0 = ((int)  tolua_tonumber(tolua_S,2,0));
  int _nItemTag1 = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'TransferIdentifyAttribFailed'", NULL);
#endif
  {
   int tolua_ret = (int)  self->TransferIdentifyAttribFailed(_nItemTag0,_nItemTag1);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'TransferIdentifyAttribFailed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SlavesFlyToMaster of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_SlavesFlyToMaster00
static int tolua_BackMirServer_HeroObject_SlavesFlyToMaster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SlavesFlyToMaster'", NULL);
#endif
  {
   self->SlavesFlyToMaster();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SlavesFlyToMaster'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Lua_OpenChestBox of class  HeroObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_HeroObject_Lua_OpenChestBox00
static int tolua_BackMirServer_HeroObject_Lua_OpenChestBox00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"HeroObject",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  HeroObject* self = (HeroObject*)  tolua_tousertype(tolua_S,1,0);
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  int _nItemID = ((int)  tolua_tonumber(tolua_S,3,0));
  int _nItemLv = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Lua_OpenChestBox'", NULL);
#endif
  {
   self->Lua_OpenChestBox(_pItem,_nItemID,_nItemLv);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Lua_OpenChestBox'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  NPCObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_NPCObject_delete00
static int tolua_BackMirServer_NPCObject_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"NPCObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  NPCObject* self = (NPCObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ClearAllSellItem of class  NPCObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_NPCObject_ClearAllSellItem00
static int tolua_BackMirServer_NPCObject_ClearAllSellItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"NPCObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  NPCObject* self = (NPCObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ClearAllSellItem'", NULL);
#endif
  {
   self->ClearAllSellItem();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ClearAllSellItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddSellItem of class  NPCObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_NPCObject_AddSellItem00
static int tolua_BackMirServer_NPCObject_AddSellItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"NPCObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  NPCObject* self = (NPCObject*)  tolua_tousertype(tolua_S,1,0);
  int _id = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddSellItem'", NULL);
#endif
  {
   self->AddSellItem(_id);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddSellItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsItemExist of class  NPCObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_NPCObject_IsItemExist00
static int tolua_BackMirServer_NPCObject_IsItemExist00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"NPCObject",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  NPCObject* self = (NPCObject*)  tolua_tousertype(tolua_S,1,0);
  int _id = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsItemExist'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsItemExist(_id);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsItemExist'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMonsterID of class  MonsterObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_MonsterObject_GetMonsterID00
static int tolua_BackMirServer_MonsterObject_GetMonsterID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MonsterObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MonsterObject* self = (MonsterObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMonsterID'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMonsterID();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMonsterID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetCanDropItems of class  MonsterObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_MonsterObject_GetCanDropItems00
static int tolua_BackMirServer_MonsterObject_GetCanDropItems00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MonsterObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MonsterObject* self = (MonsterObject*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetCanDropItems'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->GetCanDropItems();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetCanDropItems'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Say of class  MonsterObject */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_MonsterObject_Say00
static int tolua_BackMirServer_MonsterObject_Say00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MonsterObject",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MonsterObject* self = (MonsterObject*)  tolua_tousertype(tolua_S,1,0);
  const char* tolua_var_1 = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Say'", NULL);
#endif
  {
   self->Say(tolua_var_1);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Say'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetPlayer of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetPlayer00
static int tolua_BackMirServer_GameScene_GetPlayer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwID = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetPlayer'", NULL);
#endif
  {
   GameObject* tolua_ret = (GameObject*)  self->GetPlayer(_dwID);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameObject");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetPlayer'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetPlayerWithoutLock of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetPlayerWithoutLock00
static int tolua_BackMirServer_GameScene_GetPlayerWithoutLock00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwID = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetPlayerWithoutLock'", NULL);
#endif
  {
   GameObject* tolua_ret = (GameObject*)  self->GetPlayerWithoutLock(_dwID);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameObject");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetPlayerWithoutLock'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetPlayerWithoutLockInt of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetPlayerWithoutLockInt00
static int tolua_BackMirServer_GameScene_GetPlayerWithoutLockInt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  int _dwID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetPlayerWithoutLockInt'", NULL);
#endif
  {
   GameObject* tolua_ret = (GameObject*)  self->GetPlayerWithoutLockInt(_dwID);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameObject");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetPlayerWithoutLockInt'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItem of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetItem00
static int tolua_BackMirServer_GameScene_GetItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwTag = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetItem'", NULL);
#endif
  {
   GroundItem* tolua_ret = (GroundItem*)  self->GetItem(_dwTag);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GroundItem");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: BroadcastPacket of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_BroadcastPacket00
static int tolua_BackMirServer_GameScene_BroadcastPacket00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ByteBuffer",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,3,&tolua_err) || !tolua_isusertype(tolua_S,3,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  ByteBuffer* _pBuf = ((ByteBuffer*)  tolua_tousertype(tolua_S,2,0));
  DWORD _dwIgnoreID = *((DWORD*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'BroadcastPacket'", NULL);
#endif
  {
   DWORD tolua_ret = (DWORD)  self->BroadcastPacket(_pBuf,_dwIgnoreID);
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((DWORD)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"DWORD");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(DWORD));
     tolua_pushusertype(tolua_S,tolua_obj,"DWORD");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'BroadcastPacket'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetSceneData of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetSceneData00
static int tolua_BackMirServer_GameScene_GetSceneData00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"HeroObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  HeroObject* _pObj = ((HeroObject*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetSceneData'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->GetSceneData(_pObj);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetSceneData'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetRandomPosition of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetRandomPosition00
static int tolua_BackMirServer_GameScene_GetRandomPosition00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  DWORD* _pOut = ((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetRandomPosition'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->GetRandomPosition(_pOut);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetRandomPosition'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: BroadcastChatMessage of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_BroadcastChatMessage00
static int tolua_BackMirServer_GameScene_BroadcastChatMessage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  std::string _xMsg = ((std::string)  tolua_tocppstring(tolua_S,2,0));
  unsigned int _dwExtra = ((unsigned int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'BroadcastChatMessage'", NULL);
#endif
  {
   self->BroadcastChatMessage(_xMsg,_dwExtra);
   tolua_pushcppstring(tolua_S,(const char*)_xMsg);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'BroadcastChatMessage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: BroadcastSceneSystemMessage of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_BroadcastSceneSystemMessage00
static int tolua_BackMirServer_GameScene_BroadcastSceneSystemMessage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  const char* _pszMsg = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'BroadcastSceneSystemMessage'", NULL);
#endif
  {
   self->BroadcastSceneSystemMessage(_pszMsg);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'BroadcastSceneSystemMessage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateNPC of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateNPC00
static int tolua_BackMirServer_GameScene_CreateNPC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _uX = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uY = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateNPC'", NULL);
#endif
  {
   self->CreateNPC(_uID,_uX,_uY);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateNPC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateMonster00
static int tolua_BackMirServer_GameScene_CreateMonster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _uX = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uY = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateMonster'", NULL);
#endif
  {
   int tolua_ret = (int)  self->CreateMonster(_uID,_uX,_uY);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateMonster'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateEliteMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateEliteMonster00
static int tolua_BackMirServer_GameScene_CreateEliteMonster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _uX = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uY = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateEliteMonster'", NULL);
#endif
  {
   int tolua_ret = (int)  self->CreateEliteMonster(_uID,_uX,_uY);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateEliteMonster'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateLeaderMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateLeaderMonster00
static int tolua_BackMirServer_GameScene_CreateLeaderMonster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _uX = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uY = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateLeaderMonster'", NULL);
#endif
  {
   int tolua_ret = (int)  self->CreateLeaderMonster(_uID,_uX,_uY);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateLeaderMonster'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateNormalMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateNormalMonster00
static int tolua_BackMirServer_GameScene_CreateNormalMonster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _uX = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uY = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateNormalMonster'", NULL);
#endif
  {
   int tolua_ret = (int)  self->CreateNormalMonster(_uID,_uX,_uY);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateNormalMonster'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateMonster01
static int tolua_BackMirServer_GameScene_CreateMonster01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateMonster'", NULL);
#endif
  {
   int tolua_ret = (int)  self->CreateMonster(_uID);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_BackMirServer_GameScene_CreateMonster00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateNormalMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateNormalMonster01
static int tolua_BackMirServer_GameScene_CreateNormalMonster01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateNormalMonster'", NULL);
#endif
  {
   int tolua_ret = (int)  self->CreateNormalMonster(_uID);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_BackMirServer_GameScene_CreateNormalMonster00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateEliteMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateEliteMonster01
static int tolua_BackMirServer_GameScene_CreateEliteMonster01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateEliteMonster'", NULL);
#endif
  {
   int tolua_ret = (int)  self->CreateEliteMonster(_uID);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_BackMirServer_GameScene_CreateEliteMonster00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateLeaderMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateLeaderMonster01
static int tolua_BackMirServer_GameScene_CreateLeaderMonster01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateLeaderMonster'", NULL);
#endif
  {
   int tolua_ret = (int)  self->CreateLeaderMonster(_uID);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_BackMirServer_GameScene_CreateLeaderMonster00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateMonster02
static int tolua_BackMirServer_GameScene_CreateMonster02(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _ux = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uy = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
  unsigned short _oft = ((unsigned short)  tolua_tonumber(tolua_S,5,0));
  unsigned short _number = ((unsigned short)  tolua_tonumber(tolua_S,6,0));
  unsigned int _uinterval = ((unsigned int)  tolua_tonumber(tolua_S,7,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateMonster'", NULL);
#endif
  {
   self->CreateMonster(_uID,_ux,_uy,_oft,_number,_uinterval);
  }
 }
 return 0;
tolua_lerror:
 return tolua_BackMirServer_GameScene_CreateMonster01(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateEliteMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateEliteMonster02
static int tolua_BackMirServer_GameScene_CreateEliteMonster02(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _ux = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uy = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
  unsigned short _oft = ((unsigned short)  tolua_tonumber(tolua_S,5,0));
  unsigned short _number = ((unsigned short)  tolua_tonumber(tolua_S,6,0));
  unsigned int _uinterval = ((unsigned int)  tolua_tonumber(tolua_S,7,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateEliteMonster'", NULL);
#endif
  {
   self->CreateEliteMonster(_uID,_ux,_uy,_oft,_number,_uinterval);
  }
 }
 return 0;
tolua_lerror:
 return tolua_BackMirServer_GameScene_CreateEliteMonster01(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateLeaderMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateLeaderMonster02
static int tolua_BackMirServer_GameScene_CreateLeaderMonster02(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _ux = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uy = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
  unsigned short _oft = ((unsigned short)  tolua_tonumber(tolua_S,5,0));
  unsigned short _number = ((unsigned short)  tolua_tonumber(tolua_S,6,0));
  unsigned int _uinterval = ((unsigned int)  tolua_tonumber(tolua_S,7,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateLeaderMonster'", NULL);
#endif
  {
   self->CreateLeaderMonster(_uID,_ux,_uy,_oft,_number,_uinterval);
  }
 }
 return 0;
tolua_lerror:
 return tolua_BackMirServer_GameScene_CreateLeaderMonster01(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateMonsterLater of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateMonsterLater00
static int tolua_BackMirServer_GameScene_CreateMonsterLater00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _ux = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uy = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
  unsigned short _oft = ((unsigned short)  tolua_tonumber(tolua_S,5,0));
  unsigned short _number = ((unsigned short)  tolua_tonumber(tolua_S,6,0));
  unsigned int _uinterval = ((unsigned int)  tolua_tonumber(tolua_S,7,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateMonsterLater'", NULL);
#endif
  {
   self->CreateMonsterLater(_uID,_ux,_uy,_oft,_number,_uinterval);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateMonsterLater'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateEliteMonsterLater of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateEliteMonsterLater00
static int tolua_BackMirServer_GameScene_CreateEliteMonsterLater00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _ux = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uy = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
  unsigned short _oft = ((unsigned short)  tolua_tonumber(tolua_S,5,0));
  unsigned short _number = ((unsigned short)  tolua_tonumber(tolua_S,6,0));
  unsigned int _uinterval = ((unsigned int)  tolua_tonumber(tolua_S,7,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateEliteMonsterLater'", NULL);
#endif
  {
   self->CreateEliteMonsterLater(_uID,_ux,_uy,_oft,_number,_uinterval);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateEliteMonsterLater'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateLeaderMonsterLater of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateLeaderMonsterLater00
static int tolua_BackMirServer_GameScene_CreateLeaderMonsterLater00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned short _ux = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uy = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
  unsigned short _oft = ((unsigned short)  tolua_tonumber(tolua_S,5,0));
  unsigned short _number = ((unsigned short)  tolua_tonumber(tolua_S,6,0));
  unsigned int _uinterval = ((unsigned int)  tolua_tonumber(tolua_S,7,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateLeaderMonsterLater'", NULL);
#endif
  {
   self->CreateLeaderMonsterLater(_uID,_ux,_uy,_oft,_number,_uinterval);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateLeaderMonsterLater'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateGroundItem of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateGroundItem00
static int tolua_BackMirServer_GameScene_CreateGroundItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned _uID = ((unsigned)  tolua_tonumber(tolua_S,2,0));
  unsigned short _uX = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  unsigned short _uY = ((unsigned short)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateGroundItem'", NULL);
#endif
  {
   self->CreateGroundItem(_uID,_uX,_uY);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateGroundItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateDoorEvent of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateDoorEvent00
static int tolua_BackMirServer_GameScene_CreateDoorEvent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,7,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uMapID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned int _ux = ((unsigned int)  tolua_tonumber(tolua_S,3,0));
  unsigned int _uy = ((unsigned int)  tolua_tonumber(tolua_S,4,0));
  unsigned int _nextx = ((unsigned int)  tolua_tonumber(tolua_S,5,0));
  unsigned int _nexty = ((unsigned int)  tolua_tonumber(tolua_S,6,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateDoorEvent'", NULL);
#endif
  {
   self->CreateDoorEvent(_uMapID,_ux,_uy,_nextx,_nexty);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateDoorEvent'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateDoorEvent of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateDoorEvent01
static int tolua_BackMirServer_GameScene_CreateDoorEvent01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uMapID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned int _ux = ((unsigned int)  tolua_tonumber(tolua_S,3,0));
  unsigned int _uy = ((unsigned int)  tolua_tonumber(tolua_S,4,0));
  unsigned int _nextx = ((unsigned int)  tolua_tonumber(tolua_S,5,0));
  unsigned int _nexty = ((unsigned int)  tolua_tonumber(tolua_S,6,0));
  unsigned int _utime = ((unsigned int)  tolua_tonumber(tolua_S,7,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateDoorEvent'", NULL);
#endif
  {
   self->CreateDoorEvent(_uMapID,_ux,_uy,_nextx,_nexty,_utime);
  }
 }
 return 0;
tolua_lerror:
 return tolua_BackMirServer_GameScene_CreateDoorEvent00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateDoorEventWithNotification of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_CreateDoorEventWithNotification00
static int tolua_BackMirServer_GameScene_CreateDoorEventWithNotification00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uMapID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned int _ux = ((unsigned int)  tolua_tonumber(tolua_S,3,0));
  unsigned int _uy = ((unsigned int)  tolua_tonumber(tolua_S,4,0));
  unsigned int _nextx = ((unsigned int)  tolua_tonumber(tolua_S,5,0));
  unsigned int _nexty = ((unsigned int)  tolua_tonumber(tolua_S,6,0));
  unsigned int _utime = ((unsigned int)  tolua_tonumber(tolua_S,7,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateDoorEventWithNotification'", NULL);
#endif
  {
   self->CreateDoorEventWithNotification(_uMapID,_ux,_uy,_nextx,_nexty,_utime);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateDoorEventWithNotification'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetCityRange of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetCityRange00
static int tolua_BackMirServer_GameScene_SetCityRange00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  int _x = ((int)  tolua_tonumber(tolua_S,2,0));
  int _y = ((int)  tolua_tonumber(tolua_S,3,0));
  int _width = ((int)  tolua_tonumber(tolua_S,4,0));
  int _height = ((int)  tolua_tonumber(tolua_S,5,0));
  int _centerx = ((int)  tolua_tonumber(tolua_S,6,0));
  int _centery = ((int)  tolua_tonumber(tolua_S,7,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetCityRange'", NULL);
#endif
  {
   self->SetCityRange(_x,_y,_width,_height,_centerx,_centery);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetCityRange'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetAutoReset of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetAutoReset00
static int tolua_BackMirServer_GameScene_SetAutoReset00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetAutoReset'", NULL);
#endif
  {
   self->SetAutoReset();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetAutoReset'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetTreasureMap of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetTreasureMap00
static int tolua_BackMirServer_GameScene_SetTreasureMap00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  bool _b = ((bool)  tolua_toboolean(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetTreasureMap'", NULL);
#endif
  {
   self->SetTreasureMap(_b);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetTreasureMap'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetKilledMonster of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetKilledMonster00
static int tolua_BackMirServer_GameScene_SetKilledMonster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetKilledMonster'", NULL);
#endif
  {
   self->SetKilledMonster();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetKilledMonster'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetCannotStay of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetCannotStay00
static int tolua_BackMirServer_GameScene_SetCannotStay00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetCannotStay'", NULL);
#endif
  {
   self->SetCannotStay();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetCannotStay'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetCannotUseScroll of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetCannotUseScroll00
static int tolua_BackMirServer_GameScene_SetCannotUseScroll00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetCannotUseScroll'", NULL);
#endif
  {
   self->SetCannotUseScroll();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetCannotUseScroll'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetCanUseMove of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetCanUseMove00
static int tolua_BackMirServer_GameScene_SetCanUseMove00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  bool _b = ((bool)  tolua_toboolean(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetCanUseMove'", NULL);
#endif
  {
   self->SetCanUseMove(_b);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetCanUseMove'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsItemInGround of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_IsItemInGround00
static int tolua_BackMirServer_GameScene_IsItemInGround00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  int _nAttribID = ((int)  tolua_tonumber(tolua_S,2,0));
  int _x = ((int)  tolua_tonumber(tolua_S,3,0));
  int _y = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsItemInGround'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsItemInGround(_nAttribID,_x,_y);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsItemInGround'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: RemoveGroundItem of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_RemoveGroundItem00
static int tolua_BackMirServer_GameScene_RemoveGroundItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  int _nAttribID = ((int)  tolua_tonumber(tolua_S,2,0));
  int _x = ((int)  tolua_tonumber(tolua_S,3,0));
  int _y = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'RemoveGroundItem'", NULL);
#endif
  {
   self->RemoveGroundItem(_nAttribID,_x,_y);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'RemoveGroundItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMonsterSum of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetMonsterSum00
static int tolua_BackMirServer_GameScene_GetMonsterSum00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _uID = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMonsterSum'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMonsterSum(_uID);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMonsterSum'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetTimeCount of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetTimeCount00
static int tolua_BackMirServer_GameScene_GetTimeCount00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetTimeCount'", NULL);
#endif
  {
   DWORD tolua_ret = (DWORD)  self->GetTimeCount();
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((DWORD)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"DWORD");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(DWORD));
     tolua_pushusertype(tolua_S,tolua_obj,"DWORD");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetTimeCount'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: RegisterCallback of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_RegisterCallback00
static int tolua_BackMirServer_GameScene_RegisterCallback00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  SceneEvent _eType = ((SceneEvent) (int)  tolua_tonumber(tolua_S,2,0));
  const char* _pszFuncName = ((const char*)  tolua_tostring(tolua_S,3,0));
  int _nInterval = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'RegisterCallback'", NULL);
#endif
  {
   self->RegisterCallback(_eType,_pszFuncName,_nInterval);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'RegisterCallback'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetLuaEngine of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetLuaEngine00
static int tolua_BackMirServer_GameScene_GetLuaEngine00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetLuaEngine'", NULL);
#endif
  {
   LuaServerEngine* tolua_ret = (LuaServerEngine*)  self->GetLuaEngine();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"LuaServerEngine");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetLuaEngine'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetMonsterDefenceMulti of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetMonsterDefenceMulti00
static int tolua_BackMirServer_GameScene_SetMonsterDefenceMulti00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  float _fMulti = ((float)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetMonsterDefenceMulti'", NULL);
#endif
  {
   self->SetMonsterDefenceMulti(_fMulti);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMonsterDefenceMulti'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetMonsterAttackMulti of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetMonsterAttackMulti00
static int tolua_BackMirServer_GameScene_SetMonsterAttackMulti00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  float _fMulti = ((float)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetMonsterAttackMulti'", NULL);
#endif
  {
   self->SetMonsterAttackMulti(_fMulti);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMonsterAttackMulti'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetPlayerCount of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetPlayerCount00
static int tolua_BackMirServer_GameScene_GetPlayerCount00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetPlayerCount'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetPlayerCount();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetPlayerCount'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMonsterCount of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetMonsterCount00
static int tolua_BackMirServer_GameScene_GetMonsterCount00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMonsterCount'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMonsterCount();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMonsterCount'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMapIDInt of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetMapIDInt00
static int tolua_BackMirServer_GameScene_GetMapIDInt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMapIDInt'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMapIDInt();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMapIDInt'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetPlayerSum of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetPlayerSum00
static int tolua_BackMirServer_GameScene_GetPlayerSum00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetPlayerSum'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetPlayerSum();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetPlayerSum'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetSlaveSum of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetSlaveSum00
static int tolua_BackMirServer_GameScene_GetSlaveSum00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetSlaveSum'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetSlaveSum();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetSlaveSum'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ChallengeResetScene of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_ChallengeResetScene00
static int tolua_BackMirServer_GameScene_ChallengeResetScene00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  int _nClgID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ChallengeResetScene'", NULL);
#endif
  {
   self->ChallengeResetScene(_nClgID);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ChallengeResetScene'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ChallengeSuccess of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_ChallengeSuccess00
static int tolua_BackMirServer_GameScene_ChallengeSuccess00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  int _nItemID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ChallengeSuccess'", NULL);
#endif
  {
   self->ChallengeSuccess(_nItemID);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ChallengeSuccess'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetChallengeID of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetChallengeID00
static int tolua_BackMirServer_GameScene_GetChallengeID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetChallengeID'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetChallengeID();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetChallengeID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AllHeroFlyToHome of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_AllHeroFlyToHome00
static int tolua_BackMirServer_GameScene_AllHeroFlyToHome00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AllHeroFlyToHome'", NULL);
#endif
  {
   self->AllHeroFlyToHome();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AllHeroFlyToHome'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetTimeLimitID of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetTimeLimitID00
static int tolua_BackMirServer_GameScene_GetTimeLimitID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetTimeLimitID'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetTimeLimitID();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetTimeLimitID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetTimeLimitID of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetTimeLimitID00
static int tolua_BackMirServer_GameScene_SetTimeLimitID00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  int _id = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetTimeLimitID'", NULL);
#endif
  {
   self->SetTimeLimitID(_id);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetTimeLimitID'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetTimeLimitSec of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_GetTimeLimitSec00
static int tolua_BackMirServer_GameScene_GetTimeLimitSec00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetTimeLimitSec'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetTimeLimitSec();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetTimeLimitSec'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetTimeLimitSec of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetTimeLimitSec00
static int tolua_BackMirServer_GameScene_SetTimeLimitSec00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  int _sec = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetTimeLimitSec'", NULL);
#endif
  {
   self->SetTimeLimitSec(_sec);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetTimeLimitSec'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetMapPkType of class  GameScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameScene_SetMapPkType00
static int tolua_BackMirServer_GameScene_SetMapPkType00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameScene* self = (GameScene*)  tolua_tousertype(tolua_S,1,0);
  MapPkType _eT = ((MapPkType) (int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetMapPkType'", NULL);
#endif
  {
   self->SetMapPkType(_eT);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMapPkType'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_delete00
static int tolua_BackMirServer_GameSceneManager_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetInstance of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_GetInstance00
static int tolua_BackMirServer_GameSceneManager_GetInstance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   GameSceneManager* tolua_ret = (GameSceneManager*)  GameSceneManager::GetInstance();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameSceneManager");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetInstance'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: InsertPlayer of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_InsertPlayer00
static int tolua_BackMirServer_GameSceneManager_InsertPlayer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  GameObject* _pObj = ((GameObject*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'InsertPlayer'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->InsertPlayer(_pObj);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'InsertPlayer'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: InsertNPC of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_InsertNPC00
static int tolua_BackMirServer_GameSceneManager_InsertNPC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"GameObject",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  GameObject* _pObj = ((GameObject*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'InsertNPC'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->InsertNPC(_pObj);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'InsertNPC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: RemovePlayer of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_RemovePlayer00
static int tolua_BackMirServer_GameSceneManager_RemovePlayer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwID = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'RemovePlayer'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->RemovePlayer(_dwID);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'RemovePlayer'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetPlayer of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_GetPlayer00
static int tolua_BackMirServer_GameSceneManager_GetPlayer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"WORD",0,&tolua_err)) ||
     (tolua_isvaluenil(tolua_S,3,&tolua_err) || !tolua_isusertype(tolua_S,3,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  WORD _wMapID = *((WORD*)  tolua_tousertype(tolua_S,2,0));
  DWORD _dwID = *((DWORD*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetPlayer'", NULL);
#endif
  {
   GameObject* tolua_ret = (GameObject*)  self->GetPlayer(_wMapID,_dwID);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameObject");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetPlayer'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetPlayer of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_GetPlayer01
static int tolua_BackMirServer_GameSceneManager_GetPlayer01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwID = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetPlayer'", NULL);
#endif
  {
   GameObject* tolua_ret = (GameObject*)  self->GetPlayer(_dwID);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameObject");
  }
 }
 return 1;
tolua_lerror:
 return tolua_BackMirServer_GameSceneManager_GetPlayer00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetScene of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_GetScene00
static int tolua_BackMirServer_GameSceneManager_GetScene00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"DWORD",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  DWORD _dwMapID = *((DWORD*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetScene'", NULL);
#endif
  {
   GameScene* tolua_ret = (GameScene*)  self->GetScene(_dwMapID);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameScene");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetScene'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetSceneInt of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_GetSceneInt00
static int tolua_BackMirServer_GameSceneManager_GetSceneInt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  int _nMapID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetSceneInt'", NULL);
#endif
  {
   GameScene* tolua_ret = (GameScene*)  self->GetSceneInt(_nMapID);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameScene");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetSceneInt'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetFreeInstanceScene of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_GetFreeInstanceScene00
static int tolua_BackMirServer_GameSceneManager_GetFreeInstanceScene00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  int _id = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetFreeInstanceScene'", NULL);
#endif
  {
   GameInstanceScene* tolua_ret = (GameInstanceScene*)  self->GetFreeInstanceScene(_id);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameInstanceScene");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetFreeInstanceScene'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SendSystemMessageAllScene of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_SendSystemMessageAllScene00
static int tolua_BackMirServer_GameSceneManager_SendSystemMessageAllScene00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  const char* _pszMsg = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SendSystemMessageAllScene'", NULL);
#endif
  {
   self->SendSystemMessageAllScene(_pszMsg);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SendSystemMessageAllScene'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SendSystemNotifyAllScene of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_SendSystemNotifyAllScene00
static int tolua_BackMirServer_GameSceneManager_SendSystemNotifyAllScene00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  const char* _pszMsg = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SendSystemNotifyAllScene'", NULL);
#endif
  {
   self->SendSystemNotifyAllScene(_pszMsg);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SendSystemNotifyAllScene'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: CreateInstanceScene of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_CreateInstanceScene00
static int tolua_BackMirServer_GameSceneManager_CreateInstanceScene00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  int _nResID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'CreateInstanceScene'", NULL);
#endif
  {
   GameScene* tolua_ret = (GameScene*)  self->CreateInstanceScene(_nResID);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameScene");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CreateInstanceScene'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMonsterSum of class  GameSceneManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameSceneManager_GetMonsterSum00
static int tolua_BackMirServer_GameSceneManager_GetMonsterSum00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameSceneManager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameSceneManager* self = (GameSceneManager*)  tolua_tousertype(tolua_S,1,0);
  int _id = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMonsterSum'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMonsterSum(_id);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMonsterSum'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: BeginInstance of class  GameInstanceScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameInstanceScene_BeginInstance00
static int tolua_BackMirServer_GameInstanceScene_BeginInstance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameInstanceScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameInstanceScene* self = (GameInstanceScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'BeginInstance'", NULL);
#endif
  {
   self->BeginInstance();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'BeginInstance'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: OnRound of class  GameInstanceScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameInstanceScene_OnRound00
static int tolua_BackMirServer_GameInstanceScene_OnRound00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameInstanceScene",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameInstanceScene* self = (GameInstanceScene*)  tolua_tousertype(tolua_S,1,0);
  int _nRound = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'OnRound'", NULL);
#endif
  {
   self->OnRound(_nRound);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'OnRound'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: EndInstance of class  GameInstanceScene */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameInstanceScene_EndInstance00
static int tolua_BackMirServer_GameInstanceScene_EndInstance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameInstanceScene",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameInstanceScene* self = (GameInstanceScene*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'EndInstance'", NULL);
#endif
  {
   self->EndInstance();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'EndInstance'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: stAttrib of class  UserData */
#ifndef TOLUA_DISABLE_tolua_get_UserData_stAttrib
static int tolua_get_UserData_stAttrib(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'stAttrib'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->stAttrib,"ItemAttrib");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: stAttrib of class  UserData */
#ifndef TOLUA_DISABLE_tolua_set_UserData_stAttrib
static int tolua_set_UserData_stAttrib(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'stAttrib'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->stAttrib = *((ItemAttrib*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: wCoordX of class  UserData */
#ifndef TOLUA_DISABLE_tolua_get_UserData_wCoordX
static int tolua_get_UserData_wCoordX(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wCoordX'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->wCoordX,"WORD");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: wCoordX of class  UserData */
#ifndef TOLUA_DISABLE_tolua_set_UserData_wCoordX
static int tolua_set_UserData_wCoordX(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wCoordX'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"WORD",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->wCoordX = *((WORD*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: wCoordY of class  UserData */
#ifndef TOLUA_DISABLE_tolua_get_UserData_wCoordY
static int tolua_get_UserData_wCoordY(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wCoordY'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->wCoordY,"WORD");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: wCoordY of class  UserData */
#ifndef TOLUA_DISABLE_tolua_set_UserData_wCoordY
static int tolua_set_UserData_wCoordY(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wCoordY'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"WORD",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->wCoordY = *((WORD*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: wMapID of class  UserData */
#ifndef TOLUA_DISABLE_tolua_get_UserData_wMapID
static int tolua_get_UserData_wMapID(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wMapID'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->wMapID,"WORD");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: wMapID of class  UserData */
#ifndef TOLUA_DISABLE_tolua_set_UserData_wMapID
static int tolua_set_UserData_wMapID(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wMapID'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"WORD",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->wMapID = *((WORD*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: eServerState of class  UserData */
#ifndef TOLUA_DISABLE_tolua_get_UserData_eServerState
static int tolua_get_UserData_eServerState(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'eServerState'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->eServerState,"USER_STATE");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: eServerState of class  UserData */
#ifndef TOLUA_DISABLE_tolua_set_UserData_eServerState
static int tolua_set_UserData_eServerState(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'eServerState'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"USER_STATE",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->eServerState = *((USER_STATE*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: eGameState of class  UserData */
#ifndef TOLUA_DISABLE_tolua_get_UserData_eGameState
static int tolua_get_UserData_eGameState(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'eGameState'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->eGameState,"OBJECT_STATE");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: eGameState of class  UserData */
#ifndef TOLUA_DISABLE_tolua_set_UserData_eGameState
static int tolua_set_UserData_eGameState(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'eGameState'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"OBJECT_STATE",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->eGameState = *((OBJECT_STATE*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: nDrt of class  UserData */
#ifndef TOLUA_DISABLE_tolua_get_UserData_nDrt
static int tolua_get_UserData_nDrt(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'nDrt'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->nDrt);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: nDrt of class  UserData */
#ifndef TOLUA_DISABLE_tolua_set_UserData_nDrt
static int tolua_set_UserData_nDrt(lua_State* tolua_S)
{
  UserData* self = (UserData*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'nDrt'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->nDrt = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: name of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_get_ItemAttrib_name
static int tolua_get_ItemAttrib_name(lua_State* tolua_S)
{
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
#endif
  tolua_pushstring(tolua_S,(const char*)self->name);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: name of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_set_ItemAttrib_name
static int tolua_set_ItemAttrib_name(lua_State* tolua_S)
{
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
  if (!tolua_istable(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
 strncpy((char*)
self->name,(const char*)tolua_tostring(tolua_S,2,0),20-1);
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetLucky of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetLucky00
static int tolua_BackMirServer_ItemAttrib_GetLucky00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetLucky'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetLucky();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetLucky'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetLucky of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetLucky00
static int tolua_BackMirServer_ItemAttrib_SetLucky00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  int _lucky = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetLucky'", NULL);
#endif
  {
   self->SetLucky(_lucky);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetLucky'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetHide of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetHide00
static int tolua_BackMirServer_ItemAttrib_GetHide00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetHide'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetHide();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetHide'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetHide of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetHide00
static int tolua_BackMirServer_ItemAttrib_SetHide00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  int _hide = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetHide'", NULL);
#endif
  {
   self->SetHide(_hide);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetHide'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetAccuracy of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetAccuracy00
static int tolua_BackMirServer_ItemAttrib_GetAccuracy00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetAccuracy'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetAccuracy();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetAccuracy'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetAccuracy of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetAccuracy00
static int tolua_BackMirServer_ItemAttrib_SetAccuracy00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  int _accuracy = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetAccuracy'", NULL);
#endif
  {
   self->SetAccuracy(_accuracy);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetAccuracy'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetAtkSpeed of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetAtkSpeed00
static int tolua_BackMirServer_ItemAttrib_GetAtkSpeed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetAtkSpeed'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetAtkSpeed();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetAtkSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetAtkSpeed of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetAtkSpeed00
static int tolua_BackMirServer_ItemAttrib_SetAtkSpeed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  int _atkSpeed = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetAtkSpeed'", NULL);
#endif
  {
   self->SetAtkSpeed(_atkSpeed);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetAtkSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMaxDC of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetMaxDC00
static int tolua_BackMirServer_ItemAttrib_GetMaxDC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMaxDC'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMaxDC();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMaxDC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetMaxDC of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetMaxDC00
static int tolua_BackMirServer_ItemAttrib_SetMaxDC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  int _maxDC = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetMaxDC'", NULL);
#endif
  {
   self->SetMaxDC(_maxDC);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMaxDC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMaxAC of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetMaxAC00
static int tolua_BackMirServer_ItemAttrib_GetMaxAC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMaxAC'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMaxAC();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMaxAC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetMaxAC of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetMaxAC00
static int tolua_BackMirServer_ItemAttrib_SetMaxAC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  int _maxAC = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetMaxAC'", NULL);
#endif
  {
   self->SetMaxAC(_maxAC);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMaxAC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMaxMAC of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetMaxMAC00
static int tolua_BackMirServer_ItemAttrib_GetMaxMAC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMaxMAC'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMaxMAC();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMaxMAC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetMaxMAC of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetMaxMAC00
static int tolua_BackMirServer_ItemAttrib_SetMaxMAC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  int _maxMAC = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetMaxMAC'", NULL);
#endif
  {
   self->SetMaxMAC(_maxMAC);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMaxMAC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMaxSC of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetMaxSC00
static int tolua_BackMirServer_ItemAttrib_GetMaxSC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMaxSC'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMaxSC();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMaxSC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetMaxSC of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetMaxSC00
static int tolua_BackMirServer_ItemAttrib_SetMaxSC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  int _maxSC = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetMaxSC'", NULL);
#endif
  {
   self->SetMaxSC(_maxSC);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMaxSC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMaxMC of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetMaxMC00
static int tolua_BackMirServer_ItemAttrib_GetMaxMC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMaxMC'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMaxMC();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMaxMC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetMaxMC of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetMaxMC00
static int tolua_BackMirServer_ItemAttrib_SetMaxMC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  int _maxMC = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetMaxMC'", NULL);
#endif
  {
   self->SetMaxMC(_maxMC);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMaxMC'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetLevel of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetLevel00
static int tolua_BackMirServer_ItemAttrib_GetLevel00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetLevel'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetLevel();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetLevel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetLevel of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetLevel00
static int tolua_BackMirServer_ItemAttrib_SetLevel00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  int _level = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetLevel'", NULL);
#endif
  {
   self->SetLevel(_level);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetLevel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMaxHP of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetMaxHP00
static int tolua_BackMirServer_ItemAttrib_GetMaxHP00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMaxHP'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->GetMaxHP();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMaxHP'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetMaxHP of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_SetMaxHP00
static int tolua_BackMirServer_ItemAttrib_SetMaxHP00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
  unsigned int _maxHP = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetMaxHP'", NULL);
#endif
  {
   self->SetMaxHP(_maxHP);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMaxHP'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetType of class  ItemAttrib */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemAttrib_GetType00
static int tolua_BackMirServer_ItemAttrib_GetType00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* self = (ItemAttrib*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetType'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetType();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetType'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetPosX of class  GroundItem */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GroundItem_SetPosX00
static int tolua_BackMirServer_GroundItem_SetPosX00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GroundItem",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GroundItem* self = (GroundItem*)  tolua_tousertype(tolua_S,1,0);
  int _x = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetPosX'", NULL);
#endif
  {
   self->SetPosX(_x);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetPosX'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetPosY of class  GroundItem */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GroundItem_SetPosY00
static int tolua_BackMirServer_GroundItem_SetPosY00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GroundItem",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GroundItem* self = (GroundItem*)  tolua_tousertype(tolua_S,1,0);
  int _y = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetPosY'", NULL);
#endif
  {
   self->SetPosY(_y);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetPosY'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItemAttrib of class  GroundItem */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GroundItem_GetItemAttrib00
static int tolua_BackMirServer_GroundItem_GetItemAttrib00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GroundItem",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GroundItem* self = (GroundItem*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetItemAttrib'", NULL);
#endif
  {
   ItemAttrib* tolua_ret = (ItemAttrib*)  self->GetItemAttrib();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ItemAttrib");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItemAttrib'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Lua_MakeItemUpgrade */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_Lua_MakeItemUpgrade00
static int tolua_BackMirServer_Lua_MakeItemUpgrade00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int _preLevel = ((int)  tolua_tonumber(tolua_S,1,0));
  int _upgradeValue = ((int)  tolua_tonumber(tolua_S,2,0));
  {
   int tolua_ret = (int)  Lua_MakeItemUpgrade(_preLevel,_upgradeValue);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Lua_MakeItemUpgrade'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsQuestComplete of class  QuestContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_QuestContext_IsQuestComplete00
static int tolua_BackMirServer_QuestContext_IsQuestComplete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"QuestContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  QuestContext* self = (QuestContext*)  tolua_tousertype(tolua_S,1,0);
  int _questid = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsQuestComplete'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsQuestComplete(_questid);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsQuestComplete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetQuestComplete of class  QuestContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_QuestContext_SetQuestComplete00
static int tolua_BackMirServer_QuestContext_SetQuestComplete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"QuestContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  QuestContext* self = (QuestContext*)  tolua_tousertype(tolua_S,1,0);
  int _questid = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetQuestComplete'", NULL);
#endif
  {
   self->SetQuestComplete(_questid);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetQuestComplete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsQuestAccept of class  QuestContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_QuestContext_IsQuestAccept00
static int tolua_BackMirServer_QuestContext_IsQuestAccept00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"QuestContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  QuestContext* self = (QuestContext*)  tolua_tousertype(tolua_S,1,0);
  int _questid = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsQuestAccept'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsQuestAccept(_questid);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsQuestAccept'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetQuestStep of class  QuestContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_QuestContext_GetQuestStep00
static int tolua_BackMirServer_QuestContext_GetQuestStep00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"QuestContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  QuestContext* self = (QuestContext*)  tolua_tousertype(tolua_S,1,0);
  int _questid = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetQuestStep'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetQuestStep(_questid);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetQuestStep'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetQuestStep of class  QuestContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_QuestContext_SetQuestStep00
static int tolua_BackMirServer_QuestContext_SetQuestStep00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"QuestContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  QuestContext* self = (QuestContext*)  tolua_tousertype(tolua_S,1,0);
  int _questid = ((int)  tolua_tonumber(tolua_S,2,0));
  int _step = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetQuestStep'", NULL);
#endif
  {
   self->SetQuestStep(_questid,_step);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetQuestStep'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetQuestCounter of class  QuestContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_QuestContext_GetQuestCounter00
static int tolua_BackMirServer_QuestContext_GetQuestCounter00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"QuestContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  QuestContext* self = (QuestContext*)  tolua_tousertype(tolua_S,1,0);
  int _questid = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetQuestCounter'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetQuestCounter(_questid);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetQuestCounter'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetQuestCounter of class  QuestContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_QuestContext_SetQuestCounter00
static int tolua_BackMirServer_QuestContext_SetQuestCounter00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"QuestContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  QuestContext* self = (QuestContext*)  tolua_tousertype(tolua_S,1,0);
  int _questid = ((int)  tolua_tonumber(tolua_S,2,0));
  int _counter = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetQuestCounter'", NULL);
#endif
  {
   self->SetQuestCounter(_questid,_counter);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetQuestCounter'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Push of class  ItemIDList */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_ItemIDList_Push00
static int tolua_BackMirServer_ItemIDList_Push00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ItemIDList",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemIDList* self = (ItemIDList*)  tolua_tousertype(tolua_S,1,0);
  int _value = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Push'", NULL);
#endif
  {
   self->Push(_value);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Push'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetOperation of class  DBOperationParam */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBOperationParam_SetOperation00
static int tolua_BackMirServer_DBOperationParam_SetOperation00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBOperationParam",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBOperationParam* self = (DBOperationParam*)  tolua_tousertype(tolua_S,1,0);
  int _nOp = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetOperation'", NULL);
#endif
  {
   self->SetOperation(_nOp);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetOperation'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetParam of class  DBOperationParam */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBOperationParam_SetParam00
static int tolua_BackMirServer_DBOperationParam_SetParam00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBOperationParam",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBOperationParam* self = (DBOperationParam*)  tolua_tousertype(tolua_S,1,0);
  int _idx = ((int)  tolua_tonumber(tolua_S,2,0));
  int _value = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetParam'", NULL);
#endif
  {
   self->SetParam(_idx,_value);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetParam'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetInstance of class  DBThread */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBThread_GetInstance00
static int tolua_BackMirServer_DBThread_GetInstance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"DBThread",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   DBThread* tolua_ret = (DBThread*)  DBThread::GetInstance();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"DBThread");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetInstance'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AsynExecute of class  DBThread */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBThread_AsynExecute00
static int tolua_BackMirServer_DBThread_AsynExecute00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBThread",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"DBOperationParam",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBThread* self = (DBThread*)  tolua_tousertype(tolua_S,1,0);
  DBOperationParam* tolua_var_2 = ((DBOperationParam*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AsynExecute'", NULL);
#endif
  {
   self->AsynExecute(tolua_var_2);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AsynExecute'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetNewParam of class  DBThread */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBThread_GetNewParam00
static int tolua_BackMirServer_DBThread_GetNewParam00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBThread",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBThread* self = (DBThread*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetNewParam'", NULL);
#endif
  {
   DBOperationParam* tolua_ret = (DBOperationParam*)  self->GetNewParam();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"DBOperationParam");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetNewParam'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: NewGroundItem of class  DBThread */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBThread_NewGroundItem00
static int tolua_BackMirServer_DBThread_NewGroundItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBThread",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBThread* self = (DBThread*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'NewGroundItem'", NULL);
#endif
  {
   GroundItem* tolua_ret = (GroundItem*)  self->NewGroundItem();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GroundItem");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'NewGroundItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: DeleteGroundItem of class  DBThread */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBThread_DeleteGroundItem00
static int tolua_BackMirServer_DBThread_DeleteGroundItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBThread",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"GroundItem",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBThread* self = (DBThread*)  tolua_tousertype(tolua_S,1,0);
  GroundItem* _pItem = ((GroundItem*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'DeleteGroundItem'", NULL);
#endif
  {
   self->DeleteGroundItem(_pItem);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DeleteGroundItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GroundItemToInt of class  DBThread */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBThread_GroundItemToInt00
static int tolua_BackMirServer_DBThread_GroundItemToInt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBThread",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"GroundItem",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBThread* self = (DBThread*)  tolua_tousertype(tolua_S,1,0);
  GroundItem* _pItem = ((GroundItem*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GroundItemToInt'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GroundItemToInt(_pItem);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GroundItemToInt'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: UpgradeItems of class  DBThread */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBThread_UpgradeItems00
static int tolua_BackMirServer_DBThread_UpgradeItems00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBThread",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBThread* self = (DBThread*)  tolua_tousertype(tolua_S,1,0);
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  int _nProb = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'UpgradeItems'", NULL);
#endif
  {
   self->UpgradeItems(_pItem,_nProb);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'UpgradeItems'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetLuaEngine of class  DBThread */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBThread_GetLuaEngine00
static int tolua_BackMirServer_DBThread_GetLuaEngine00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBThread",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBThread* self = (DBThread*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetLuaEngine'", NULL);
#endif
  {
   LuaServerEngine* tolua_ret = (LuaServerEngine*)  self->GetLuaEngine();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"LuaServerEngine");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetLuaEngine'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetRecordInItemTable */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GetRecordInItemTable00
static int tolua_BackMirServer_GetRecordInItemTable00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int _id = ((int)  tolua_tonumber(tolua_S,1,0));
  ItemAttrib* _pOut = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   bool tolua_ret = (bool)  GetRecordInItemTable(_id,_pOut);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetRecordInItemTable'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: DoModule of class  LuaBaseEngine */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaBaseEngine_DoModule00
static int tolua_BackMirServer_LuaBaseEngine_DoModule00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"LuaBaseEngine",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  LuaBaseEngine* self = (LuaBaseEngine*)  tolua_tousertype(tolua_S,1,0);
  const char* _pszModuleFile = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'DoModule'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->DoModule(_pszModuleFile);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DoModule'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsDropValid of class  DBDropDownContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBDropDownContext_IsDropValid00
static int tolua_BackMirServer_DBDropDownContext_IsDropValid00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBDropDownContext",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBDropDownContext* self = (DBDropDownContext*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsDropValid'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsDropValid();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsDropValid'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetDropMultiple of class  DBDropDownContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBDropDownContext_GetDropMultiple00
static int tolua_BackMirServer_DBDropDownContext_GetDropMultiple00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBDropDownContext",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBDropDownContext* self = (DBDropDownContext*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetDropMultiple'", NULL);
#endif
  {
   float tolua_ret = (float)  self->GetDropMultiple();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetDropMultiple'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetMagicDropMultiple of class  DBDropDownContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBDropDownContext_GetMagicDropMultiple00
static int tolua_BackMirServer_DBDropDownContext_GetMagicDropMultiple00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBDropDownContext",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBDropDownContext* self = (DBDropDownContext*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetMagicDropMultiple'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetMagicDropMultiple();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMagicDropMultiple'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetDropScene of class  DBDropDownContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBDropDownContext_GetDropScene00
static int tolua_BackMirServer_DBDropDownContext_GetDropScene00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBDropDownContext",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBDropDownContext* self = (DBDropDownContext*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetDropScene'", NULL);
#endif
  {
   GameScene* tolua_ret = (GameScene*)  self->GetDropScene();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameScene");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetDropScene'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: InitDropPosition of class  DBDropDownContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBDropDownContext_InitDropPosition00
static int tolua_BackMirServer_DBDropDownContext_InitDropPosition00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBDropDownContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBDropDownContext* self = (DBDropDownContext*)  tolua_tousertype(tolua_S,1,0);
  int _nItems = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'InitDropPosition'", NULL);
#endif
  {
   int tolua_ret = (int)  self->InitDropPosition(_nItems);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'InitDropPosition'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetDropPosX of class  DBDropDownContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBDropDownContext_GetDropPosX00
static int tolua_BackMirServer_DBDropDownContext_GetDropPosX00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBDropDownContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBDropDownContext* self = (DBDropDownContext*)  tolua_tousertype(tolua_S,1,0);
  int _nItemIndex = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetDropPosX'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetDropPosX(_nItemIndex);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetDropPosX'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetDropPosY of class  DBDropDownContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBDropDownContext_GetDropPosY00
static int tolua_BackMirServer_DBDropDownContext_GetDropPosY00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBDropDownContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBDropDownContext* self = (DBDropDownContext*)  tolua_tousertype(tolua_S,1,0);
  int _nItemIndex = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetDropPosY'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetDropPosY(_nItemIndex);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetDropPosY'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: NewGroundItem of class  DBDropDownContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBDropDownContext_NewGroundItem00
static int tolua_BackMirServer_DBDropDownContext_NewGroundItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBDropDownContext",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBDropDownContext* self = (DBDropDownContext*)  tolua_tousertype(tolua_S,1,0);
  int _nItemID = ((int)  tolua_tonumber(tolua_S,2,0));
  int _nPosX = ((int)  tolua_tonumber(tolua_S,3,0));
  int _nPosY = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'NewGroundItem'", NULL);
#endif
  {
   GroundItem* tolua_ret = (GroundItem*)  self->NewGroundItem(_nItemID,_nPosX,_nPosY);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GroundItem");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'NewGroundItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetAdditionPoint of class  DBDropDownContext */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_DBDropDownContext_GetAdditionPoint00
static int tolua_BackMirServer_DBDropDownContext_GetAdditionPoint00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DBDropDownContext",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DBDropDownContext* self = (DBDropDownContext*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetAdditionPoint'", NULL);
#endif
  {
   int tolua_ret = (int)  self->GetAdditionPoint();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetAdditionPoint'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItemType of class  LuaItemHelper */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaItemHelper_GetItemType00
static int tolua_BackMirServer_LuaItemHelper_GetItemType00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"LuaItemHelper",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   int tolua_ret = (int)  LuaItemHelper::GetItemType(_pItem);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItemType'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItemAtkSpeed of class  LuaItemHelper */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaItemHelper_GetItemAtkSpeed00
static int tolua_BackMirServer_LuaItemHelper_GetItemAtkSpeed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"LuaItemHelper",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   int tolua_ret = (int)  LuaItemHelper::GetItemAtkSpeed(_pItem);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItemAtkSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItemUpgrade of class  LuaItemHelper */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaItemHelper_GetItemUpgrade00
static int tolua_BackMirServer_LuaItemHelper_GetItemUpgrade00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"LuaItemHelper",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   int tolua_ret = (int)  LuaItemHelper::GetItemUpgrade(_pItem);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItemUpgrade'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItemAtkPalsy of class  LuaItemHelper */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaItemHelper_GetItemAtkPalsy00
static int tolua_BackMirServer_LuaItemHelper_GetItemAtkPalsy00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"LuaItemHelper",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   int tolua_ret = (int)  LuaItemHelper::GetItemAtkPalsy(_pItem);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItemAtkPalsy'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItemLucky of class  LuaItemHelper */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaItemHelper_GetItemLucky00
static int tolua_BackMirServer_LuaItemHelper_GetItemLucky00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"LuaItemHelper",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   int tolua_ret = (int)  LuaItemHelper::GetItemLucky(_pItem);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItemLucky'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItemTag of class  LuaItemHelper */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaItemHelper_GetItemTag00
static int tolua_BackMirServer_LuaItemHelper_GetItemTag00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"LuaItemHelper",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   int tolua_ret = (int)  LuaItemHelper::GetItemTag(_pItem);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItemTag'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetItemMP of class  LuaItemHelper */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaItemHelper_GetItemMP00
static int tolua_BackMirServer_LuaItemHelper_GetItemMP00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"LuaItemHelper",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   int tolua_ret = (int)  LuaItemHelper::GetItemMP(_pItem);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetItemMP'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: EncryptItem of class  LuaItemHelper */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaItemHelper_EncryptItem00
static int tolua_BackMirServer_LuaItemHelper_EncryptItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"LuaItemHelper",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   LuaItemHelper::EncryptItem(_pItem);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'EncryptItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: DecryptItem of class  LuaItemHelper */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaItemHelper_DecryptItem00
static int tolua_BackMirServer_LuaItemHelper_DecryptItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"LuaItemHelper",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   LuaItemHelper::DecryptItem(_pItem);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DecryptItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsEncrypt of class  LuaItemHelper */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_LuaItemHelper_IsEncrypt00
static int tolua_BackMirServer_LuaItemHelper_IsEncrypt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"LuaItemHelper",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ItemAttrib",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ItemAttrib* _pItem = ((ItemAttrib*)  tolua_tousertype(tolua_S,2,0));
  {
   bool tolua_ret = (bool)  LuaItemHelper::IsEncrypt(_pItem);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsEncrypt'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetLoWord */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GetLoWord00
static int tolua_BackMirServer_GetLoWord00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  unsigned int _nValue = ((unsigned int)  tolua_tonumber(tolua_S,1,0));
  {
   unsigned int tolua_ret = (unsigned int)  GetLoWord(_nValue);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetLoWord'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetHiWord */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GetHiWord00
static int tolua_BackMirServer_GetHiWord00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  unsigned int _nValue = ((unsigned int)  tolua_tonumber(tolua_S,1,0));
  {
   unsigned int tolua_ret = (unsigned int)  GetHiWord(_nValue);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetHiWord'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: MakeLong */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_MakeLong00
static int tolua_BackMirServer_MakeLong00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  unsigned int _nLow = ((unsigned int)  tolua_tonumber(tolua_S,1,0));
  unsigned int _nHigh = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  {
   unsigned int tolua_ret = (unsigned int)  MakeLong(_nLow,_nHigh);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'MakeLong'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetInstance of class  OlShopManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_OlShopManager_GetInstance00
static int tolua_BackMirServer_OlShopManager_GetInstance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"OlShopManager",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   OlShopManager* tolua_ret = (OlShopManager*)  OlShopManager::GetInstance();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"OlShopManager");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetInstance'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddShopItem of class  OlShopManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_OlShopManager_AddShopItem00
static int tolua_BackMirServer_OlShopManager_AddShopItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"OlShopManager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  OlShopManager* self = (OlShopManager*)  tolua_tousertype(tolua_S,1,0);
  int _nID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddShopItem'", NULL);
#endif
  {
   self->AddShopItem(_nID);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddShopItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: RemoveShopItem of class  OlShopManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_OlShopManager_RemoveShopItem00
static int tolua_BackMirServer_OlShopManager_RemoveShopItem00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"OlShopManager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  OlShopManager* self = (OlShopManager*)  tolua_tousertype(tolua_S,1,0);
  int _nID = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'RemoveShopItem'", NULL);
#endif
  {
   self->RemoveShopItem(_nID);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'RemoveShopItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ClearShopItems of class  OlShopManager */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_OlShopManager_ClearShopItems00
static int tolua_BackMirServer_OlShopManager_ClearShopItems00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"OlShopManager",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  OlShopManager* self = (OlShopManager*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ClearShopItems'", NULL);
#endif
  {
   self->ClearShopItems();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ClearShopItems'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetInstance of class  CMainServer */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_CMainServer_GetInstance00
static int tolua_BackMirServer_CMainServer_GetInstance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CMainServer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CMainServer* tolua_ret = (CMainServer*)  CMainServer::GetInstance();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CMainServer");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetInstance'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsLoginServerMode of class  CMainServer */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_CMainServer_IsLoginServerMode00
static int tolua_BackMirServer_CMainServer_IsLoginServerMode00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CMainServer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CMainServer* self = (CMainServer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsLoginServerMode'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsLoginServerMode();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsLoginServerMode'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetInstancePtr of class  GameWorld */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameWorld_GetInstancePtr00
static int tolua_BackMirServer_GameWorld_GetInstancePtr00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"GameWorld",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   GameWorld* tolua_ret = (GameWorld*)  GameWorld::GetInstancePtr();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameWorld");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetInstancePtr'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetSchedule of class  GameWorld */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameWorld_SetSchedule00
static int tolua_BackMirServer_GameWorld_SetSchedule00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameWorld",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameWorld* self = (GameWorld*)  tolua_tousertype(tolua_S,1,0);
  int _nEventId = ((int)  tolua_tonumber(tolua_S,2,0));
  const char* _pszCronExpr = ((const char*)  tolua_tostring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetSchedule'", NULL);
#endif
  {
   self->SetSchedule(_nEventId,_pszCronExpr);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetSchedule'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ResetSchedule of class  GameWorld */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameWorld_ResetSchedule00
static int tolua_BackMirServer_GameWorld_ResetSchedule00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameWorld",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameWorld* self = (GameWorld*)  tolua_tousertype(tolua_S,1,0);
  int _nEventId = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ResetSchedule'", NULL);
#endif
  {
   self->ResetSchedule(_nEventId);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ResetSchedule'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: AddAdditionPointWeight of class  GameWorld */
#ifndef TOLUA_DISABLE_tolua_BackMirServer_GameWorld_AddAdditionPointWeight00
static int tolua_BackMirServer_GameWorld_AddAdditionPointWeight00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameWorld",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameWorld* self = (GameWorld*)  tolua_tousertype(tolua_S,1,0);
  int _nPoint = ((int)  tolua_tonumber(tolua_S,2,0));
  int _nWeight = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'AddAdditionPointWeight'", NULL);
#endif
  {
   self->AddAdditionPointWeight(_nPoint,_nWeight);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddAdditionPointWeight'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_BackMirServer_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_constant(tolua_S,"SOT_NONE",SOT_NONE);
  tolua_constant(tolua_S,"SOT_HERO",SOT_HERO);
  tolua_constant(tolua_S,"SOT_NPC",SOT_NPC);
  tolua_constant(tolua_S,"SOT_MONSTER",SOT_MONSTER);
  tolua_function(tolua_S,"MirLog",tolua_BackMirServer_MirLog00);
  tolua_function(tolua_S,"ConsolePrint",tolua_BackMirServer_ConsolePrint00);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"GameObject","GameObject","LockObject",tolua_collect_GameObject);
  #else
  tolua_cclass(tolua_S,"GameObject","GameObject","LockObject",NULL);
  #endif
  tolua_beginmodule(tolua_S,"GameObject");
   tolua_function(tolua_S,"delete",tolua_BackMirServer_GameObject_delete00);
   tolua_function(tolua_S,"AddProcess",tolua_BackMirServer_GameObject_AddProcess00);
   tolua_function(tolua_S,"GetID",tolua_BackMirServer_GameObject_GetID00);
   tolua_function(tolua_S,"GetType",tolua_BackMirServer_GameObject_GetType00);
   tolua_function(tolua_S,"GetAttrib",tolua_BackMirServer_GameObject_GetAttrib00);
   tolua_function(tolua_S,"GetMapID",tolua_BackMirServer_GameObject_GetMapID00);
   tolua_function(tolua_S,"GetUserData",tolua_BackMirServer_GameObject_GetUserData00);
   tolua_function(tolua_S,"GetLocateScene",tolua_BackMirServer_GameObject_GetLocateScene00);
   tolua_function(tolua_S,"GetAttribID",tolua_BackMirServer_GameObject_GetAttribID00);
   tolua_function(tolua_S,"GetLevel",tolua_BackMirServer_GameObject_GetLevel00);
   tolua_function(tolua_S,"FlyToInstanceMap",tolua_BackMirServer_GameObject_FlyToInstanceMap00);
   tolua_function(tolua_S,"FlyToMap",tolua_BackMirServer_GameObject_FlyToMap00);
   tolua_function(tolua_S,"IncHP",tolua_BackMirServer_GameObject_IncHP00);
   tolua_function(tolua_S,"DecHP",tolua_BackMirServer_GameObject_DecHP00);
   tolua_function(tolua_S,"IncMP",tolua_BackMirServer_GameObject_IncMP00);
   tolua_function(tolua_S,"DecMP",tolua_BackMirServer_GameObject_DecMP00);
   tolua_function(tolua_S,"GetCoordX",tolua_BackMirServer_GameObject_GetCoordX00);
   tolua_function(tolua_S,"GetCoordY",tolua_BackMirServer_GameObject_GetCoordY00);
   tolua_function(tolua_S,"GetCoordXInt",tolua_BackMirServer_GameObject_GetCoordXInt00);
   tolua_function(tolua_S,"GetCoordYInt",tolua_BackMirServer_GameObject_GetCoordYInt00);
   tolua_function(tolua_S,"GetState",tolua_BackMirServer_GameObject_GetState00);
   tolua_function(tolua_S,"GetTotalRecvDamage",tolua_BackMirServer_GameObject_GetTotalRecvDamage00);
   tolua_function(tolua_S,"GetHP",tolua_BackMirServer_GameObject_GetHP00);
   tolua_function(tolua_S,"GetMaxHP",tolua_BackMirServer_GameObject_GetMaxHP00);
   tolua_function(tolua_S,"GetExpr",tolua_BackMirServer_GameObject_GetExpr00);
   tolua_function(tolua_S,"GetName",tolua_BackMirServer_GameObject_GetName00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"HeroObject","HeroObject","GameObject",NULL);
  tolua_beginmodule(tolua_S,"HeroObject");
   tolua_function(tolua_S,"GetItemByIndex",tolua_BackMirServer_HeroObject_GetItemByIndex00);
   tolua_function(tolua_S,"GetItemByTag",tolua_BackMirServer_HeroObject_GetItemByTag00);
   tolua_function(tolua_S,"Lua_GetItemByAttribID",tolua_BackMirServer_HeroObject_Lua_GetItemByAttribID00);
   tolua_function(tolua_S,"Lua_GetItemByTag",tolua_BackMirServer_HeroObject_Lua_GetItemByTag00);
   tolua_function(tolua_S,"AddBagItem",tolua_BackMirServer_HeroObject_AddBagItem00);
   tolua_function(tolua_S,"AddBagItem",tolua_BackMirServer_HeroObject_AddBagItem01);
   tolua_function(tolua_S,"SendSystemMessage",tolua_BackMirServer_HeroObject_SendSystemMessage00);
   tolua_function(tolua_S,"ShowQuestDlg",tolua_BackMirServer_HeroObject_ShowQuestDlg00);
   tolua_function(tolua_S,"HideQuestDlg",tolua_BackMirServer_HeroObject_HideQuestDlg00);
   tolua_function(tolua_S,"ShowShopDlg",tolua_BackMirServer_HeroObject_ShowShopDlg00);
   tolua_function(tolua_S,"GetQuest",tolua_BackMirServer_HeroObject_GetQuest00);
   tolua_function(tolua_S,"CountItem",tolua_BackMirServer_HeroObject_CountItem00);
   tolua_function(tolua_S,"ClearItem",tolua_BackMirServer_HeroObject_ClearItem00);
   tolua_function(tolua_S,"AddItem",tolua_BackMirServer_HeroObject_AddItem00);
   tolua_function(tolua_S,"AddItemReturnTag",tolua_BackMirServer_HeroObject_AddItemReturnTag00);
   tolua_function(tolua_S,"RemoveItem",tolua_BackMirServer_HeroObject_RemoveItem00);
   tolua_function(tolua_S,"ItemTagToAttribID",tolua_BackMirServer_HeroObject_ItemTagToAttribID00);
   tolua_function(tolua_S,"GetBagEmptySum",tolua_BackMirServer_HeroObject_GetBagEmptySum00);
   tolua_function(tolua_S,"GetAssistEmptySum",tolua_BackMirServer_HeroObject_GetAssistEmptySum00);
   tolua_function(tolua_S,"SyncItemAttrib",tolua_BackMirServer_HeroObject_SyncItemAttrib00);
   tolua_function(tolua_S,"GetMoney",tolua_BackMirServer_HeroObject_GetMoney00);
   tolua_function(tolua_S,"AddMoney",tolua_BackMirServer_HeroObject_AddMoney00);
   tolua_function(tolua_S,"MinusMoney",tolua_BackMirServer_HeroObject_MinusMoney00);
   tolua_function(tolua_S,"GainExp",tolua_BackMirServer_HeroObject_GainExp00);
   tolua_function(tolua_S,"GetHeroJob",tolua_BackMirServer_HeroObject_GetHeroJob00);
   tolua_function(tolua_S,"GetHeroSex",tolua_BackMirServer_HeroObject_GetHeroSex00);
   tolua_function(tolua_S,"ClearAllItem",tolua_BackMirServer_HeroObject_ClearAllItem00);
   tolua_function(tolua_S,"FlyToHome",tolua_BackMirServer_HeroObject_FlyToHome00);
   tolua_function(tolua_S,"SetValidState",tolua_BackMirServer_HeroObject_SetValidState00);
   tolua_function(tolua_S,"SyncQuestData",tolua_BackMirServer_HeroObject_SyncQuestData00);
   tolua_function(tolua_S,"GetTeamID",tolua_BackMirServer_HeroObject_GetTeamID00);
   tolua_function(tolua_S,"GetTeamMate",tolua_BackMirServer_HeroObject_GetTeamMate00);
   tolua_function(tolua_S,"TeamMateFlyToInstanceMap",tolua_BackMirServer_HeroObject_TeamMateFlyToInstanceMap00);
   tolua_function(tolua_S,"IsTeamLeader",tolua_BackMirServer_HeroObject_IsTeamLeader00);
   tolua_function(tolua_S,"Lua_GetActiveDropParam",tolua_BackMirServer_HeroObject_Lua_GetActiveDropParam00);
   tolua_function(tolua_S,"GetChallengeItemID",tolua_BackMirServer_HeroObject_GetChallengeItemID00);
   tolua_function(tolua_S,"UseChallengeItem",tolua_BackMirServer_HeroObject_UseChallengeItem00);
   tolua_function(tolua_S,"IsTeamMateAround",tolua_BackMirServer_HeroObject_IsTeamMateAround00);
   tolua_function(tolua_S,"TeamMateFlyToMap",tolua_BackMirServer_HeroObject_TeamMateFlyToMap00);
   tolua_function(tolua_S,"ReceiveGift",tolua_BackMirServer_HeroObject_ReceiveGift00);
   tolua_function(tolua_S,"ResetIDlg",tolua_BackMirServer_HeroObject_ResetIDlg00);
   tolua_function(tolua_S,"AddIDlg_CloseButton",tolua_BackMirServer_HeroObject_AddIDlg_CloseButton00);
   tolua_function(tolua_S,"AddIDlg_Button",tolua_BackMirServer_HeroObject_AddIDlg_Button00);
   tolua_function(tolua_S,"AddIDlg_String",tolua_BackMirServer_HeroObject_AddIDlg_String00);
   tolua_function(tolua_S,"ShowIDlg",tolua_BackMirServer_HeroObject_ShowIDlg00);
   tolua_function(tolua_S,"HideIDlg",tolua_BackMirServer_HeroObject_HideIDlg00);
   tolua_function(tolua_S,"SetEnterTimeLimitScene",tolua_BackMirServer_HeroObject_SetEnterTimeLimitScene00);
   tolua_function(tolua_S,"UpdateLifeSkillLevel",tolua_BackMirServer_HeroObject_UpdateLifeSkillLevel00);
   tolua_function(tolua_S,"Lua_SetQuestStep",tolua_BackMirServer_HeroObject_Lua_SetQuestStep00);
   tolua_function(tolua_S,"Lua_GetQuestStep",tolua_BackMirServer_HeroObject_Lua_GetQuestStep00);
   tolua_function(tolua_S,"Lua_SetQuestCounter",tolua_BackMirServer_HeroObject_Lua_SetQuestCounter00);
   tolua_function(tolua_S,"Lua_GetQuestCounter",tolua_BackMirServer_HeroObject_Lua_GetQuestCounter00);
   tolua_function(tolua_S,"TransferIdentifyAttrib",tolua_BackMirServer_HeroObject_TransferIdentifyAttrib00);
   tolua_function(tolua_S,"TransferIdentifyAttribFailed",tolua_BackMirServer_HeroObject_TransferIdentifyAttribFailed00);
   tolua_function(tolua_S,"SlavesFlyToMaster",tolua_BackMirServer_HeroObject_SlavesFlyToMaster00);
   tolua_function(tolua_S,"Lua_OpenChestBox",tolua_BackMirServer_HeroObject_Lua_OpenChestBox00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"NPCObject","NPCObject","GameObject",tolua_collect_NPCObject);
  #else
  tolua_cclass(tolua_S,"NPCObject","NPCObject","GameObject",NULL);
  #endif
  tolua_beginmodule(tolua_S,"NPCObject");
   tolua_function(tolua_S,"delete",tolua_BackMirServer_NPCObject_delete00);
   tolua_function(tolua_S,"ClearAllSellItem",tolua_BackMirServer_NPCObject_ClearAllSellItem00);
   tolua_function(tolua_S,"AddSellItem",tolua_BackMirServer_NPCObject_AddSellItem00);
   tolua_function(tolua_S,"IsItemExist",tolua_BackMirServer_NPCObject_IsItemExist00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"MonsterObject","MonsterObject","GameObject",NULL);
  tolua_beginmodule(tolua_S,"MonsterObject");
   tolua_function(tolua_S,"GetMonsterID",tolua_BackMirServer_MonsterObject_GetMonsterID00);
   tolua_function(tolua_S,"GetCanDropItems",tolua_BackMirServer_MonsterObject_GetCanDropItems00);
   tolua_function(tolua_S,"Say",tolua_BackMirServer_MonsterObject_Say00);
  tolua_endmodule(tolua_S);
  tolua_constant(tolua_S,"kMapPkType_None",kMapPkType_None);
  tolua_constant(tolua_S,"kMapPkType_All",kMapPkType_All);
  tolua_cclass(tolua_S,"GameScene","GameScene","",NULL);
  tolua_beginmodule(tolua_S,"GameScene");
   tolua_function(tolua_S,"GetPlayer",tolua_BackMirServer_GameScene_GetPlayer00);
   tolua_function(tolua_S,"GetPlayerWithoutLock",tolua_BackMirServer_GameScene_GetPlayerWithoutLock00);
   tolua_function(tolua_S,"GetPlayerWithoutLockInt",tolua_BackMirServer_GameScene_GetPlayerWithoutLockInt00);
   tolua_function(tolua_S,"GetItem",tolua_BackMirServer_GameScene_GetItem00);
   tolua_function(tolua_S,"BroadcastPacket",tolua_BackMirServer_GameScene_BroadcastPacket00);
   tolua_function(tolua_S,"GetSceneData",tolua_BackMirServer_GameScene_GetSceneData00);
   tolua_function(tolua_S,"GetRandomPosition",tolua_BackMirServer_GameScene_GetRandomPosition00);
   tolua_function(tolua_S,"BroadcastChatMessage",tolua_BackMirServer_GameScene_BroadcastChatMessage00);
   tolua_function(tolua_S,"BroadcastSceneSystemMessage",tolua_BackMirServer_GameScene_BroadcastSceneSystemMessage00);
   tolua_function(tolua_S,"CreateNPC",tolua_BackMirServer_GameScene_CreateNPC00);
   tolua_function(tolua_S,"CreateMonster",tolua_BackMirServer_GameScene_CreateMonster00);
   tolua_function(tolua_S,"CreateEliteMonster",tolua_BackMirServer_GameScene_CreateEliteMonster00);
   tolua_function(tolua_S,"CreateLeaderMonster",tolua_BackMirServer_GameScene_CreateLeaderMonster00);
   tolua_function(tolua_S,"CreateNormalMonster",tolua_BackMirServer_GameScene_CreateNormalMonster00);
   tolua_function(tolua_S,"CreateMonster",tolua_BackMirServer_GameScene_CreateMonster01);
   tolua_function(tolua_S,"CreateNormalMonster",tolua_BackMirServer_GameScene_CreateNormalMonster01);
   tolua_function(tolua_S,"CreateEliteMonster",tolua_BackMirServer_GameScene_CreateEliteMonster01);
   tolua_function(tolua_S,"CreateLeaderMonster",tolua_BackMirServer_GameScene_CreateLeaderMonster01);
   tolua_function(tolua_S,"CreateMonster",tolua_BackMirServer_GameScene_CreateMonster02);
   tolua_function(tolua_S,"CreateEliteMonster",tolua_BackMirServer_GameScene_CreateEliteMonster02);
   tolua_function(tolua_S,"CreateLeaderMonster",tolua_BackMirServer_GameScene_CreateLeaderMonster02);
   tolua_function(tolua_S,"CreateMonsterLater",tolua_BackMirServer_GameScene_CreateMonsterLater00);
   tolua_function(tolua_S,"CreateEliteMonsterLater",tolua_BackMirServer_GameScene_CreateEliteMonsterLater00);
   tolua_function(tolua_S,"CreateLeaderMonsterLater",tolua_BackMirServer_GameScene_CreateLeaderMonsterLater00);
   tolua_function(tolua_S,"CreateGroundItem",tolua_BackMirServer_GameScene_CreateGroundItem00);
   tolua_function(tolua_S,"CreateDoorEvent",tolua_BackMirServer_GameScene_CreateDoorEvent00);
   tolua_function(tolua_S,"CreateDoorEvent",tolua_BackMirServer_GameScene_CreateDoorEvent01);
   tolua_function(tolua_S,"CreateDoorEventWithNotification",tolua_BackMirServer_GameScene_CreateDoorEventWithNotification00);
   tolua_function(tolua_S,"SetCityRange",tolua_BackMirServer_GameScene_SetCityRange00);
   tolua_function(tolua_S,"SetAutoReset",tolua_BackMirServer_GameScene_SetAutoReset00);
   tolua_function(tolua_S,"SetTreasureMap",tolua_BackMirServer_GameScene_SetTreasureMap00);
   tolua_function(tolua_S,"SetKilledMonster",tolua_BackMirServer_GameScene_SetKilledMonster00);
   tolua_function(tolua_S,"SetCannotStay",tolua_BackMirServer_GameScene_SetCannotStay00);
   tolua_function(tolua_S,"SetCannotUseScroll",tolua_BackMirServer_GameScene_SetCannotUseScroll00);
   tolua_function(tolua_S,"SetCanUseMove",tolua_BackMirServer_GameScene_SetCanUseMove00);
   tolua_function(tolua_S,"IsItemInGround",tolua_BackMirServer_GameScene_IsItemInGround00);
   tolua_function(tolua_S,"RemoveGroundItem",tolua_BackMirServer_GameScene_RemoveGroundItem00);
   tolua_function(tolua_S,"GetMonsterSum",tolua_BackMirServer_GameScene_GetMonsterSum00);
   tolua_function(tolua_S,"GetTimeCount",tolua_BackMirServer_GameScene_GetTimeCount00);
   tolua_function(tolua_S,"RegisterCallback",tolua_BackMirServer_GameScene_RegisterCallback00);
   tolua_function(tolua_S,"GetLuaEngine",tolua_BackMirServer_GameScene_GetLuaEngine00);
   tolua_function(tolua_S,"SetMonsterDefenceMulti",tolua_BackMirServer_GameScene_SetMonsterDefenceMulti00);
   tolua_function(tolua_S,"SetMonsterAttackMulti",tolua_BackMirServer_GameScene_SetMonsterAttackMulti00);
   tolua_function(tolua_S,"GetPlayerCount",tolua_BackMirServer_GameScene_GetPlayerCount00);
   tolua_function(tolua_S,"GetMonsterCount",tolua_BackMirServer_GameScene_GetMonsterCount00);
   tolua_function(tolua_S,"GetMapIDInt",tolua_BackMirServer_GameScene_GetMapIDInt00);
   tolua_function(tolua_S,"GetPlayerSum",tolua_BackMirServer_GameScene_GetPlayerSum00);
   tolua_function(tolua_S,"GetSlaveSum",tolua_BackMirServer_GameScene_GetSlaveSum00);
   tolua_function(tolua_S,"ChallengeResetScene",tolua_BackMirServer_GameScene_ChallengeResetScene00);
   tolua_function(tolua_S,"ChallengeSuccess",tolua_BackMirServer_GameScene_ChallengeSuccess00);
   tolua_function(tolua_S,"GetChallengeID",tolua_BackMirServer_GameScene_GetChallengeID00);
   tolua_function(tolua_S,"AllHeroFlyToHome",tolua_BackMirServer_GameScene_AllHeroFlyToHome00);
   tolua_function(tolua_S,"GetTimeLimitID",tolua_BackMirServer_GameScene_GetTimeLimitID00);
   tolua_function(tolua_S,"SetTimeLimitID",tolua_BackMirServer_GameScene_SetTimeLimitID00);
   tolua_function(tolua_S,"GetTimeLimitSec",tolua_BackMirServer_GameScene_GetTimeLimitSec00);
   tolua_function(tolua_S,"SetTimeLimitSec",tolua_BackMirServer_GameScene_SetTimeLimitSec00);
   tolua_function(tolua_S,"SetMapPkType",tolua_BackMirServer_GameScene_SetMapPkType00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"GameSceneManager","GameSceneManager","",tolua_collect_GameSceneManager);
  #else
  tolua_cclass(tolua_S,"GameSceneManager","GameSceneManager","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"GameSceneManager");
   tolua_function(tolua_S,"delete",tolua_BackMirServer_GameSceneManager_delete00);
   tolua_function(tolua_S,"GetInstance",tolua_BackMirServer_GameSceneManager_GetInstance00);
   tolua_function(tolua_S,"InsertPlayer",tolua_BackMirServer_GameSceneManager_InsertPlayer00);
   tolua_function(tolua_S,"InsertNPC",tolua_BackMirServer_GameSceneManager_InsertNPC00);
   tolua_function(tolua_S,"RemovePlayer",tolua_BackMirServer_GameSceneManager_RemovePlayer00);
   tolua_function(tolua_S,"GetPlayer",tolua_BackMirServer_GameSceneManager_GetPlayer00);
   tolua_function(tolua_S,"GetPlayer",tolua_BackMirServer_GameSceneManager_GetPlayer01);
   tolua_function(tolua_S,"GetScene",tolua_BackMirServer_GameSceneManager_GetScene00);
   tolua_function(tolua_S,"GetSceneInt",tolua_BackMirServer_GameSceneManager_GetSceneInt00);
   tolua_function(tolua_S,"GetFreeInstanceScene",tolua_BackMirServer_GameSceneManager_GetFreeInstanceScene00);
   tolua_function(tolua_S,"SendSystemMessageAllScene",tolua_BackMirServer_GameSceneManager_SendSystemMessageAllScene00);
   tolua_function(tolua_S,"SendSystemNotifyAllScene",tolua_BackMirServer_GameSceneManager_SendSystemNotifyAllScene00);
   tolua_function(tolua_S,"CreateInstanceScene",tolua_BackMirServer_GameSceneManager_CreateInstanceScene00);
   tolua_function(tolua_S,"GetMonsterSum",tolua_BackMirServer_GameSceneManager_GetMonsterSum00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"GameInstanceScene","GameInstanceScene","",NULL);
  tolua_beginmodule(tolua_S,"GameInstanceScene");
   tolua_function(tolua_S,"BeginInstance",tolua_BackMirServer_GameInstanceScene_BeginInstance00);
   tolua_function(tolua_S,"OnRound",tolua_BackMirServer_GameInstanceScene_OnRound00);
   tolua_function(tolua_S,"EndInstance",tolua_BackMirServer_GameInstanceScene_EndInstance00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"UserData","UserData","",NULL);
  tolua_beginmodule(tolua_S,"UserData");
   tolua_variable(tolua_S,"stAttrib",tolua_get_UserData_stAttrib,tolua_set_UserData_stAttrib);
   tolua_variable(tolua_S,"wCoordX",tolua_get_UserData_wCoordX,tolua_set_UserData_wCoordX);
   tolua_variable(tolua_S,"wCoordY",tolua_get_UserData_wCoordY,tolua_set_UserData_wCoordY);
   tolua_variable(tolua_S,"wMapID",tolua_get_UserData_wMapID,tolua_set_UserData_wMapID);
   tolua_variable(tolua_S,"eServerState",tolua_get_UserData_eServerState,tolua_set_UserData_eServerState);
   tolua_variable(tolua_S,"eGameState",tolua_get_UserData_eGameState,tolua_set_UserData_eGameState);
   tolua_variable(tolua_S,"nDrt",tolua_get_UserData_nDrt,tolua_set_UserData_nDrt);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"ItemAttrib","ItemAttrib","",NULL);
  tolua_beginmodule(tolua_S,"ItemAttrib");
   tolua_variable(tolua_S,"name",tolua_get_ItemAttrib_name,tolua_set_ItemAttrib_name);
   tolua_function(tolua_S,"GetLucky",tolua_BackMirServer_ItemAttrib_GetLucky00);
   tolua_function(tolua_S,"SetLucky",tolua_BackMirServer_ItemAttrib_SetLucky00);
   tolua_function(tolua_S,"GetHide",tolua_BackMirServer_ItemAttrib_GetHide00);
   tolua_function(tolua_S,"SetHide",tolua_BackMirServer_ItemAttrib_SetHide00);
   tolua_function(tolua_S,"GetAccuracy",tolua_BackMirServer_ItemAttrib_GetAccuracy00);
   tolua_function(tolua_S,"SetAccuracy",tolua_BackMirServer_ItemAttrib_SetAccuracy00);
   tolua_function(tolua_S,"GetAtkSpeed",tolua_BackMirServer_ItemAttrib_GetAtkSpeed00);
   tolua_function(tolua_S,"SetAtkSpeed",tolua_BackMirServer_ItemAttrib_SetAtkSpeed00);
   tolua_function(tolua_S,"GetMaxDC",tolua_BackMirServer_ItemAttrib_GetMaxDC00);
   tolua_function(tolua_S,"SetMaxDC",tolua_BackMirServer_ItemAttrib_SetMaxDC00);
   tolua_function(tolua_S,"GetMaxAC",tolua_BackMirServer_ItemAttrib_GetMaxAC00);
   tolua_function(tolua_S,"SetMaxAC",tolua_BackMirServer_ItemAttrib_SetMaxAC00);
   tolua_function(tolua_S,"GetMaxMAC",tolua_BackMirServer_ItemAttrib_GetMaxMAC00);
   tolua_function(tolua_S,"SetMaxMAC",tolua_BackMirServer_ItemAttrib_SetMaxMAC00);
   tolua_function(tolua_S,"GetMaxSC",tolua_BackMirServer_ItemAttrib_GetMaxSC00);
   tolua_function(tolua_S,"SetMaxSC",tolua_BackMirServer_ItemAttrib_SetMaxSC00);
   tolua_function(tolua_S,"GetMaxMC",tolua_BackMirServer_ItemAttrib_GetMaxMC00);
   tolua_function(tolua_S,"SetMaxMC",tolua_BackMirServer_ItemAttrib_SetMaxMC00);
   tolua_function(tolua_S,"GetLevel",tolua_BackMirServer_ItemAttrib_GetLevel00);
   tolua_function(tolua_S,"SetLevel",tolua_BackMirServer_ItemAttrib_SetLevel00);
   tolua_function(tolua_S,"GetMaxHP",tolua_BackMirServer_ItemAttrib_GetMaxHP00);
   tolua_function(tolua_S,"SetMaxHP",tolua_BackMirServer_ItemAttrib_SetMaxHP00);
   tolua_function(tolua_S,"GetType",tolua_BackMirServer_ItemAttrib_GetType00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"GroundItem","GroundItem","",NULL);
  tolua_beginmodule(tolua_S,"GroundItem");
   tolua_function(tolua_S,"SetPosX",tolua_BackMirServer_GroundItem_SetPosX00);
   tolua_function(tolua_S,"SetPosY",tolua_BackMirServer_GroundItem_SetPosY00);
   tolua_function(tolua_S,"GetItemAttrib",tolua_BackMirServer_GroundItem_GetItemAttrib00);
  tolua_endmodule(tolua_S);
  tolua_function(tolua_S,"Lua_MakeItemUpgrade",tolua_BackMirServer_Lua_MakeItemUpgrade00);
  tolua_cclass(tolua_S,"QuestContext","QuestContext","",NULL);
  tolua_beginmodule(tolua_S,"QuestContext");
   tolua_function(tolua_S,"IsQuestComplete",tolua_BackMirServer_QuestContext_IsQuestComplete00);
   tolua_function(tolua_S,"SetQuestComplete",tolua_BackMirServer_QuestContext_SetQuestComplete00);
   tolua_function(tolua_S,"IsQuestAccept",tolua_BackMirServer_QuestContext_IsQuestAccept00);
   tolua_function(tolua_S,"GetQuestStep",tolua_BackMirServer_QuestContext_GetQuestStep00);
   tolua_function(tolua_S,"SetQuestStep",tolua_BackMirServer_QuestContext_SetQuestStep00);
   tolua_function(tolua_S,"GetQuestCounter",tolua_BackMirServer_QuestContext_GetQuestCounter00);
   tolua_function(tolua_S,"SetQuestCounter",tolua_BackMirServer_QuestContext_SetQuestCounter00);
  tolua_endmodule(tolua_S);
  tolua_constant(tolua_S,"SceneEvent_None",SceneEvent_None);
  tolua_constant(tolua_S,"SceneEvent_Update",SceneEvent_Update);
  tolua_constant(tolua_S,"SceneEvent_PlayerEnter",SceneEvent_PlayerEnter);
  tolua_constant(tolua_S,"SceneEvent_PlayerLeave",SceneEvent_PlayerLeave);
  tolua_constant(tolua_S,"SceneEvent_Total",SceneEvent_Total);
  tolua_cclass(tolua_S,"ItemIDList","ItemIDList","std::list<int>",NULL);
  tolua_beginmodule(tolua_S,"ItemIDList");
   tolua_function(tolua_S,"Push",tolua_BackMirServer_ItemIDList_Push00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"DBOperationParam","DBOperationParam","",NULL);
  tolua_beginmodule(tolua_S,"DBOperationParam");
   tolua_function(tolua_S,"SetOperation",tolua_BackMirServer_DBOperationParam_SetOperation00);
   tolua_function(tolua_S,"SetParam",tolua_BackMirServer_DBOperationParam_SetParam00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"DBThread","DBThread","",NULL);
  tolua_beginmodule(tolua_S,"DBThread");
   tolua_function(tolua_S,"GetInstance",tolua_BackMirServer_DBThread_GetInstance00);
   tolua_function(tolua_S,"AsynExecute",tolua_BackMirServer_DBThread_AsynExecute00);
   tolua_function(tolua_S,"GetNewParam",tolua_BackMirServer_DBThread_GetNewParam00);
   tolua_function(tolua_S,"NewGroundItem",tolua_BackMirServer_DBThread_NewGroundItem00);
   tolua_function(tolua_S,"DeleteGroundItem",tolua_BackMirServer_DBThread_DeleteGroundItem00);
   tolua_function(tolua_S,"GroundItemToInt",tolua_BackMirServer_DBThread_GroundItemToInt00);
   tolua_function(tolua_S,"UpgradeItems",tolua_BackMirServer_DBThread_UpgradeItems00);
   tolua_function(tolua_S,"GetLuaEngine",tolua_BackMirServer_DBThread_GetLuaEngine00);
  tolua_endmodule(tolua_S);
  tolua_function(tolua_S,"GetRecordInItemTable",tolua_BackMirServer_GetRecordInItemTable00);
  tolua_cclass(tolua_S,"LuaBaseEngine","LuaBaseEngine","",NULL);
  tolua_beginmodule(tolua_S,"LuaBaseEngine");
   tolua_function(tolua_S,"DoModule",tolua_BackMirServer_LuaBaseEngine_DoModule00);
  tolua_endmodule(tolua_S);
  tolua_constant(tolua_S,"kLuaEvent_None",kLuaEvent_None);
  tolua_constant(tolua_S,"kLuaEvent_WorldUpdate",kLuaEvent_WorldUpdate);
  tolua_constant(tolua_S,"kLuaEvent_WorldScheduleActive",kLuaEvent_WorldScheduleActive);
  tolua_constant(tolua_S,"kLuaEvent_WorldNPCActive",kLuaEvent_WorldNPCActive);
  tolua_constant(tolua_S,"kLuaEvent_WorldStartRunning",kLuaEvent_WorldStartRunning);
  tolua_constant(tolua_S,"kLuaEvent_ScenePlayerEnter",kLuaEvent_ScenePlayerEnter);
  tolua_constant(tolua_S,"kLuaEvent_WorldLoginServerConnected",kLuaEvent_WorldLoginServerConnected);
  tolua_constant(tolua_S,"kLuaEvent_ScenePlayerLeave",kLuaEvent_ScenePlayerLeave);
  tolua_cclass(tolua_S,"DBDropDownContext","DBDropDownContext","",NULL);
  tolua_beginmodule(tolua_S,"DBDropDownContext");
   tolua_function(tolua_S,"IsDropValid",tolua_BackMirServer_DBDropDownContext_IsDropValid00);
   tolua_function(tolua_S,"GetDropMultiple",tolua_BackMirServer_DBDropDownContext_GetDropMultiple00);
   tolua_function(tolua_S,"GetMagicDropMultiple",tolua_BackMirServer_DBDropDownContext_GetMagicDropMultiple00);
   tolua_function(tolua_S,"GetDropScene",tolua_BackMirServer_DBDropDownContext_GetDropScene00);
   tolua_function(tolua_S,"InitDropPosition",tolua_BackMirServer_DBDropDownContext_InitDropPosition00);
   tolua_function(tolua_S,"GetDropPosX",tolua_BackMirServer_DBDropDownContext_GetDropPosX00);
   tolua_function(tolua_S,"GetDropPosY",tolua_BackMirServer_DBDropDownContext_GetDropPosY00);
   tolua_function(tolua_S,"NewGroundItem",tolua_BackMirServer_DBDropDownContext_NewGroundItem00);
   tolua_function(tolua_S,"GetAdditionPoint",tolua_BackMirServer_DBDropDownContext_GetAdditionPoint00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"LuaItemHelper","LuaItemHelper","",NULL);
  tolua_beginmodule(tolua_S,"LuaItemHelper");
   tolua_function(tolua_S,"GetItemType",tolua_BackMirServer_LuaItemHelper_GetItemType00);
   tolua_function(tolua_S,"GetItemAtkSpeed",tolua_BackMirServer_LuaItemHelper_GetItemAtkSpeed00);
   tolua_function(tolua_S,"GetItemUpgrade",tolua_BackMirServer_LuaItemHelper_GetItemUpgrade00);
   tolua_function(tolua_S,"GetItemAtkPalsy",tolua_BackMirServer_LuaItemHelper_GetItemAtkPalsy00);
   tolua_function(tolua_S,"GetItemLucky",tolua_BackMirServer_LuaItemHelper_GetItemLucky00);
   tolua_function(tolua_S,"GetItemTag",tolua_BackMirServer_LuaItemHelper_GetItemTag00);
   tolua_function(tolua_S,"GetItemMP",tolua_BackMirServer_LuaItemHelper_GetItemMP00);
   tolua_function(tolua_S,"EncryptItem",tolua_BackMirServer_LuaItemHelper_EncryptItem00);
   tolua_function(tolua_S,"DecryptItem",tolua_BackMirServer_LuaItemHelper_DecryptItem00);
   tolua_function(tolua_S,"IsEncrypt",tolua_BackMirServer_LuaItemHelper_IsEncrypt00);
  tolua_endmodule(tolua_S);
  tolua_function(tolua_S,"GetLoWord",tolua_BackMirServer_GetLoWord00);
  tolua_function(tolua_S,"GetHiWord",tolua_BackMirServer_GetHiWord00);
  tolua_function(tolua_S,"MakeLong",tolua_BackMirServer_MakeLong00);
  tolua_cclass(tolua_S,"OlShopManager","OlShopManager","",NULL);
  tolua_beginmodule(tolua_S,"OlShopManager");
   tolua_function(tolua_S,"GetInstance",tolua_BackMirServer_OlShopManager_GetInstance00);
   tolua_function(tolua_S,"AddShopItem",tolua_BackMirServer_OlShopManager_AddShopItem00);
   tolua_function(tolua_S,"RemoveShopItem",tolua_BackMirServer_OlShopManager_RemoveShopItem00);
   tolua_function(tolua_S,"ClearShopItems",tolua_BackMirServer_OlShopManager_ClearShopItems00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CMainServer","CMainServer","",NULL);
  tolua_beginmodule(tolua_S,"CMainServer");
   tolua_function(tolua_S,"GetInstance",tolua_BackMirServer_CMainServer_GetInstance00);
   tolua_function(tolua_S,"IsLoginServerMode",tolua_BackMirServer_CMainServer_IsLoginServerMode00);
  tolua_endmodule(tolua_S);
  tolua_constant(tolua_S,"kLifeSkill_SmeltOre",kLifeSkill_SmeltOre);
  tolua_constant(tolua_S,"kLifeSkill_SmeltWood",kLifeSkill_SmeltWood);
  tolua_constant(tolua_S,"kLifeSkill_SmeltCloth",kLifeSkill_SmeltCloth);
  tolua_constant(tolua_S,"kLifeSkill_SmeltGem",kLifeSkill_SmeltGem);
  tolua_constant(tolua_S,"kLifeSkill_SmeltEquip",kLifeSkill_SmeltEquip);
  tolua_constant(tolua_S,"kLifeSkill_Total",kLifeSkill_Total);
  tolua_cclass(tolua_S,"GameWorld","GameWorld","",NULL);
  tolua_beginmodule(tolua_S,"GameWorld");
   tolua_function(tolua_S,"GetInstancePtr",tolua_BackMirServer_GameWorld_GetInstancePtr00);
   tolua_function(tolua_S,"SetSchedule",tolua_BackMirServer_GameWorld_SetSchedule00);
   tolua_function(tolua_S,"ResetSchedule",tolua_BackMirServer_GameWorld_ResetSchedule00);
   tolua_function(tolua_S,"AddAdditionPointWeight",tolua_BackMirServer_GameWorld_AddAdditionPointWeight00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_BackMirServer (lua_State* tolua_S) {
 return tolua_BackMirServer_open(tolua_S);
};
#endif

