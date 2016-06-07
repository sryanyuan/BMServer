#include "LuaServerEngine.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include "../../CommonModule/SaveFile.h"
#include "../../CommonModule/ConsoleHelper.h"
#include <Shlwapi.h>
//////////////////////////////////////////////////////////////////////////
#ifdef _LUAJIT_
#pragma comment(lib, "lua51.lib")
#endif
//////////////////////////////////////////////////////////////////////////
extern ConsoleHelper g_xConsole;
//////////////////////////////////////////////////////////////////////////
LuaServerEngine::LuaServerEngine()
{
	m_nUserTag = 0;
	m_nLoadMode = LOADMODE_PATH;

#ifdef _DEBUG
	m_eFileExt = kLuaFileExt_Lua;
#else
	m_eFileExt = kLuaFileExt_Bjt;
#endif
	SetZipPassword(SaveFile::CalcInternalPassword());
}

void LuaServerEngine::Output(const char* _pszLog)
{
	g_xConsole.CPrint(_pszLog);
	LOG(INFO) << _pszLog;
}

bool LuaServerEngine::ExecuteZip(const char* _pszFileName, const char* _pszSubFile)
{
	return LoadFileInZip(_pszFileName, _pszSubFile, SaveFile::CalcInternalPassword());
}

bool LuaServerEngine::ExecuteFile(const char* _pszFileName)
{
	return LoadFile(_pszFileName);
}

/*bool LuaServerEngine::LoadModule(const char* _pszModuleFile)
{
	if(m_xModulePath.empty())
	{
		return false;
	}

	char szLoadFullFileName[MAX_PATH];
	strcpy(szLoadFullFileName, _pszModuleFile);

	const char* pszLuaExtension = ".lua";

#ifdef _DEBUG
	//strcat(szLoadFullFileName, ".lua");
#else
#ifdef _LUAJIT_
	//strcat(szLoadFullFileName, ".bjt");
	pszLuaExtension = ".bjt";
#else
	//strcat(szLoadFullFileName, ".bbt");
	pszLuaExtension = ".bbt";
#endif
#endif

	strcat(szLoadFullFileName, pszLuaExtension);

	if(LOADMODE_PATH == m_nLoadMode)
	{
		char szFullPath[MAX_PATH] = {0};
		sprintf(szFullPath, "%s/%s", m_xModulePath.c_str(), szLoadFullFileName);
		return LoadFile(szFullPath);
	}
	else if(LOADMODE_ZIP == m_nLoadMode)
	{
		//	check if need decrypt
		if(0 == strcmp(".bjt", pszLuaExtension))
		{
			return LoadFileInZipEncrypt(m_xModulePath.c_str(), szLoadFullFileName, SaveFile::CalcInternalPassword());
		}
		else
		{
			return LoadFileInZip(m_xModulePath.c_str(), szLoadFullFileName, SaveFile::CalcInternalPassword());
		}
	}

	return false;
}*/


int LuaServerEngine::OnDispatchEvent(const LuaDispatchEvent* _pEvent, LuaDispatchInfo* _pInfo)
{
	if(_pEvent->nEventId != kLuaEvent_WorldUpdate)
	{
		g_xConsole.CPrint("Dispatch lua event:%d", _pEvent->nEventId);
	}

	if(_pEvent->nEventId == kLuaEvent_WorldNPCActive)
	{
		const LuaEvent_WorldNPCActive* pEvent = (const LuaEvent_WorldNPCActive*)_pEvent->pEvent;

		tolua_pushusertype(GetVM(), pEvent->pHero, "HeroObject");
		tolua_pushusertype(GetVM(), pEvent->pNPC, "NPCObject");
		PushInt(pEvent->nButtonId);
		_pInfo->nArgs = 3;
	}
	else if(_pEvent->nEventId == kLuaEvent_ScenePlayerEnter)
	{
		const LuaEvent_ScenePlayerEnter* pEvent = (const LuaEvent_ScenePlayerEnter*)_pEvent->pEvent;

		tolua_pushusertype(L, pEvent->pScene, "GameScene");
		tolua_pushusertype(L, pEvent->pHero, "HeroObject");
		_pInfo->nArgs = 2;
	}
	else if(_pEvent->nEventId == kLuaEvent_WorldScheduleActive)
	{
		const LuaEvent_WorldScheduleActive* pEvent = (const LuaEvent_WorldScheduleActive*)_pEvent->pEvent;
		PushInt(pEvent->nScheduleId);
		_pInfo->nArgs = 1;
	}

	return 0;
}

void LuaServerEngine::OnDispatchEventResult(const LuaDispatchEvent* _pEvent, bool bResult)
{

}