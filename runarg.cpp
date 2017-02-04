#include "runarg.h"
#include "../CommonModule/CommandLineHelper.h"
//////////////////////////////////////////////////////////////////////////
CommandLineHelper g_Cl;

bool InitRunArg()
{
	if (!g_Cl.InitParam())
	{
		return false;
	}

	return true;
}

const char* GetRunArg(const char* _arg)
{
	return g_Cl.GetParam(_arg);
}

int GetRunArgInt(const char* _arg)
{
	const char* arg = g_Cl.GetParam(_arg);
	int nVal = atoi(arg);
	return nVal;
}

int GetServerID()
{
	return GetRunArgInt("serverid");
}