#ifndef _INC_LOGGER_
#define _INC_LOGGER_
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdarg.h>
//////////////////////////////////////////////////////////////////////////
#define LOGLEVEL_DEBUG	0
#define LOGLEVEL_INFO	1
#define LOGLEVEL_ERROR	2

inline void logPrint(int _nLevel, const char* _pszFunction, int _nLine, const char* _pszFormat, ...)
{
	static const char* s_szLogLevelStr[] = {
		"[DEBUG]",
		"[INFO ]",
		"[ERROR]"
	};

	char szLogBuffer[512];
	szLogBuffer[0] = 0;
	szLogBuffer[sizeof(szLogBuffer) / sizeof(szLogBuffer[0]) - 1] = 0;
	int nWrite = _snprintf(szLogBuffer, sizeof(szLogBuffer) / sizeof(szLogBuffer[0]) - 1, "%s %s:%d ", s_szLogLevelStr[_nLevel], _pszFunction, _nLine);
	if(nWrite < 0)
	{
		//	this log is truncated
		return;
	}
	size_t uPtr = strlen(szLogBuffer);
	if(uPtr >= sizeof(szLogBuffer) / sizeof(szLogBuffer[0] - 1))
	{
		return;
	}

	va_list args;
	va_start(args, _pszFormat);
	int nRet = vsnprintf(szLogBuffer + uPtr, sizeof(szLogBuffer) / sizeof(szLogBuffer[0]) - uPtr, _pszFormat, args);
	va_end(args);

	if(nRet <= 0 ||
		nRet >= int(sizeof(szLogBuffer) / sizeof(szLogBuffer[0]) - uPtr - 1))
	{
		return;
	}

	szLogBuffer[nRet + uPtr] = '\n';
	szLogBuffer[nRet + uPtr + 1] = '\0';
	printf(szLogBuffer);
}

#define LOGDEBUG(FORMAT, ...)	logPrint(LOGLEVEL_DEBUG, __FUNCTION__, __LINE__, FORMAT, __VA_ARGS__)
#define LOGPRINT(FORMAT, ...)	logPrint(LOGLEVEL_INFO, __FUNCTION__, __LINE__, FORMAT, __VA_ARGS__)
#define LOGINFO(FORMAT, ...)	logPrint(LOGLEVEL_INFO, __FUNCTION__, __LINE__, FORMAT, __VA_ARGS__)
#define LOGERROR(FORMAT, ...)	logPrint(LOGLEVEL_ERROR, __FUNCTION__, __LINE__, FORMAT, __VA_ARGS__)
//////////////////////////////////////////////////////////////////////////
#endif