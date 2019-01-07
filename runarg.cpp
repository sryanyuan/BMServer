#include "runarg.h"
#include "../CommonModule/CommandLineHelper.h"
#include "common/glog.h"
#include "Helper.h"
//////////////////////////////////////////////////////////////////////////
CommandLineHelper g_Cl;

bool InitRunArg()
{
	if (!g_Cl.InitParam())
	{
		return false;
	}

	// Load run arg values from configfile
	const char* pszCfgFile = RunArgGetConfigFile();

	// Verify run args
	const char* pszServerID = GetRunArg("serverid");
	if (NULL != pszServerID) {
		if (strlen(pszServerID) == 0) {
			// Invalid
			LOG(ERROR) << "Server id set to empty string";
			return false;
		}
		int nValue = atoi(pszServerID);
		if (0 == nValue) {
			LOG(ERROR) << "Server id set to zero";
			return false;
		}

		// If serverid is set, servername must be set
		const char* pszServerName = GetRunArg("servername");
		if (NULL == pszServerName) {
			LOG(ERROR) << "Server name must be set";
			return false;
		}
		if (strlen(pszServerName) == 0) {
			LOG(ERROR) << "Server name must be set";
			return false;
		}
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

const char* RunArgGetConfigFile() {
	static char s_szDefaultCfgFile[MAX_PATH] = {0};
	if (s_szDefaultCfgFile[0] == 0) {
		sprintf(s_szDefaultCfgFile, "%s\\conf\\cfg.ini", GetRootPath());
	}
	const char* pszValue = GetRunArg("cfgfile");
	if (NULL == pszValue) {
		return s_szDefaultCfgFile;
	}
	if (strlen(pszValue) == 0) {
		return s_szDefaultCfgFile;
	}
	return pszValue;
}