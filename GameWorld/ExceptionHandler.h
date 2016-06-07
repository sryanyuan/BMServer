#ifndef _INC_EXCEPTIONHANDLER_
#define _INC_EXCEPTIONHANDLER_
//////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include "../../CommonModule/StackWalker/StackWalker.h"
//////////////////////////////////////////////////////////////////////////
extern const char* g_pszExecuteFunc_WorldThread;
extern const char* g_pszExecuteFunc_ServerThread;
extern const char* g_pszExecuteFunc_DBThread;
extern const char* g_pszExecuteFunc_UIThread;
extern int g_nExecuteFunc_WorldThread;

//#define RECORD_FUNCNAME_WORLD		g_pszExecuteFunc_WorldThread = __FUNCTION__
#define RECORD_FUNCNAME_WORLD		
//#define RECORD_FUNCNAME_SERVER		g_pszExecuteFunc_ServerThread = __FUNCTION__
#define RECORD_FUNCNAME_SERVER		
//#define RECORD_FUNCNAME_DB			g_pszExecuteFunc_DBThread = __FUNCTION__
#define RECORD_FUNCNAME_DB			
//#define RECORD_FUNCNAME_UI			g_pszExecuteFunc_UIThread = __FUNCTION__
#define RECORD_FUNCNAME_UI			

//#define RECORD_FUNCLINE_WORLD		g_nExecuteFunc_WorldThread = __LINE__
#define RECORD_FUNCLINE_WORLD		
//////////////////////////////////////////////////////////////////////////
LONG WINAPI BM_UnhandledExceptionFilter(_EXCEPTION_POINTERS* pExceptionInfo);

/************************************************************************/
/* Stack walker
/************************************************************************/
class StackWalkerLog : public StackWalker
{
public:
	virtual void OnOutput(LPCSTR szText);
};
//////////////////////////////////////////////////////////////////////////
#endif