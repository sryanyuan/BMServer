#ifndef _INC_RUNARG_
#define _INC_RUNARG_
//////////////////////////////////////////////////////////////////////////
bool InitRunArg();
const char* GetRunArg(const char* _arg);
int GetRunArgInt(const char* _arg);

// Following are run command as serverid=<serverid> ...
// serverid
int GetServerID();
// cfgfile
const char* RunArgGetConfigFile();
//////////////////////////////////////////////////////////////////////////
#endif