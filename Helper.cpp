#include "Helper.h"
//#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <afxwin.h>
#include <Shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

void AddInfomation(const char* fmt, ...)
{
	extern HWND g_hServerDlg;

	char buffer[MAX_PATH];

	va_list args;
	va_start( args, fmt );
	_vsnprintf( buffer, sizeof(buffer), fmt, args);

	SYSTEMTIME lpTime;
	GetLocalTime(&lpTime);

	char logTime[MAX_PATH];
	sprintf(logTime, "[%d-%d-%d %02d:%02d:%02d] ", lpTime.wYear, lpTime.wMonth, lpTime.wDay, lpTime.wHour, lpTime.wMinute, lpTime.wSecond);


	CString strOut;
	//strOut.Format("[%s] %s", logTime, buffer);
	strcat(logTime, buffer);

	if(!SendMessageTimeout(g_hServerDlg, WM_HELPER_ADDINFO, (WPARAM)(LPCTSTR)logTime, NULL,
		SMTO_ABORTIFHUNG | SMTO_BLOCK,
		500,
		NULL
		))
	{

	}
}

const char* GetRootPath()
{
	static char s_szRootPath[MAX_PATH] = {0};

	if(s_szRootPath[0] == 0)
	{
		GetModuleFileName(NULL, s_szRootPath, MAX_PATH);
		PathRemoveFileSpec(s_szRootPath);
	}

	return s_szRootPath;
}

void UpdateDialogInfo(const ServerState* _pState)
{
	extern HWND g_hServerDlg;

	static ServerState s_State;
	memcpy(&s_State, _pState, sizeof(ServerState));
	::PostMessage(g_hServerDlg, WM_UPDATE_INFO, WPARAM(&s_State), 0);
}