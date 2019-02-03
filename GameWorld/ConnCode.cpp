#include "ConnCode.h"
#include "GameWorld.h"
//////////////////////////////////////////////////////////////////////////
//	global variables
unsigned int g_dwConnCode[MAX_CONNECTIONS + 1] = {0};
unsigned int g_dwConnSeed = 0;
//////////////////////////////////////////////////////////////////////////
unsigned int GetNewConnCode()
{
	return ++g_dwConnSeed;
}

unsigned int GetConnCode(unsigned int _uConnIndex)
{
	if(_uConnIndex > MAX_CONNECTIONS)
	{
		return 0;
	}

	return g_dwConnCode[_uConnIndex];
}

void SetConnCode(unsigned int _uConnIndex, unsigned int _uConnCode)
{
	if(_uConnIndex > MAX_CONNECTIONS)
	{
		return;
	}

	g_dwConnCode[_uConnIndex] = _uConnCode;
}