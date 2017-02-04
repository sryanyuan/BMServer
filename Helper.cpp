#include "Helper.h"
//#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <afxwin.h>
#include <Shlwapi.h>

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
		GetRootPath(s_szRootPath, MAX_PATH);
	}

	return s_szRootPath;
}

void GetRootPath(char* _pszBuf, unsigned int _sz)
{
	GetModuleFileName(NULL, _pszBuf, _sz);
	PathRemoveFileSpec(_pszBuf);
#ifdef _BIN_PATH
	// remove current path
	size_t uStrlen = strlen(_pszBuf);
	if (0 == uStrlen)
	{
		return;
	}
	for (size_t i = uStrlen - 1; i >= 0; --i)
	{
		if (_pszBuf[i] == '\\' ||
			_pszBuf[i] == '/')
		{
			// done
			break;
		}
		_pszBuf[i] = '\0';
	}
#endif
}

void UpdateDialogInfo(const ServerState* _pState)
{
	extern HWND g_hServerDlg;

	static ServerState s_State;
	memcpy(&s_State, _pState, sizeof(ServerState));
	::PostMessage(g_hServerDlg, WM_UPDATE_INFO, WPARAM(&s_State), 0);
}