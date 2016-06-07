#ifndef _INC_WATCHERTHREAD_
#define _INC_WATCHERTHREAD_
//////////////////////////////////////////////////////////////////////////
#include "../../CommonModule/CRCVerifyThread/CRCVerifyThread.h"
//////////////////////////////////////////////////////////////////////////
class WatcherThread : public CRCVerifyThread
{
public:
	enum Verify_Function
	{
		VF_OBJ_GETOBJECT_MAXAC,
		VF_OBJ_GETOBJECT_MAXMAC,
		VF_OBJ_GETOBJECT_MAXDC,
		VF_OBJ_GETOBJECT_MAXMC,
		VF_OBJ_GETOBJECT_MAXSC,
		VF_OBJ_GETRANDOMABILITY,
		VF_MONS_RECEIVEDAMAGE,
		VF_HERO_RECEIVEDAMAGE,
		VF_HERO_DOSPELL,
		VF_MONS_PARSEATTACKMSG,
		VF_MONS_DROPMONSTERITEMS,
		VF_MONS_ATTACKTARGET,
		VF_HERO_ADDMONEY,
		VF_HERO_GAINEXP,
		VF_SCENE_AUTOGENERATEMONSTER,
		VF_HERO_USEDRUGITEM,
		VF_DBTHREAD_UPGRADEITEMS,
		VF_DBTHREAD_UPGRADEATTRIB,
		VF_DBTHREAD_DBDROPITEMEXCALLBACK,
		VF_HERO_DOPACKET_FORGEITEM,
		VF_WORLD_UPGRADEATTRIB,
		VF_WORLD_UPGRADEITEMSWITHADDITION,
		VF_HERO_UPDATESTATUS,
		VF_OBJ_UPDATESTATUS,
		VF_WATCHER_THREAD_DOWORK,
		VF_WORLD_DOWORK_DELAYEDPROCESS,
		VF_TOTAL
	};

public:
	static int s_nVerifySize[VF_TOTAL];

public:
	WatcherThread()
	{
		SetRunSleepTime(1000);
	}

public:
	virtual bool OnCRCVerifyFailed(const CRCVerifyElement* _pElement);
	virtual unsigned int Thread_DoWork();

public:
	void* GetVerifyFunctionAddr(Verify_Function _eFunc);
	size_t GetVerifyFuncionSize(Verify_Function _eFunc);
};
//////////////////////////////////////////////////////////////////////////
#endif