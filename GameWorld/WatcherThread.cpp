#include "../CMainServer/CMainServer.h"
#include "WatcherThread.h"
#include "GameWorld.h"
#include "GameSceneManager.h"
#include "MonsterObject.h"
#include "ObjectEngine.h"
#include "../../CommonModule/CRCVerifyThread/AntiRE.h"
//////////////////////////////////////////////////////////////////////////
int WatcherThread::s_nVerifySize[WatcherThread::VF_TOTAL] =
{
	20,
	20,
	20,
	20,
	20,
	40,
	20,
	50,
	20,
	300,
	10,
	20,
	100,
	//	Gain exp
	30,
	100,
	80,
	150,
	150,
	//	Dropdownex
	1266,
	300,
	150,
	250,
	100,
	150,
	150,
	150
};
//////////////////////////////////////////////////////////////////////////
bool WatcherThread::OnCRCVerifyFailed(const CRCVerifyElement* _pElement)
{
#ifndef _DISABLE_CONSOLE
	g_xConsole.CPrint("CRC check failed, tag:%d",
		_pElement->uTag);
#endif
	if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
	{
		LOG(ERROR) << "DESTORY_GAMEWORLD";
	}
	else
	{
		DESTORY_GAMEWORLD;
	}

	return true;
}

unsigned int WatcherThread::Thread_DoWork()
{
	unsigned int uRet = __super::Thread_DoWork();

	if(uRet == 0)
	{
		//	Check
		char szForbidAppName[3][30];
		ZeroMemory(szForbidAppName, sizeof(szForbidAppName));

		//	Ollyice
		int nWriteIndex = 0;
		szForbidAppName[0][nWriteIndex++] = 'o';
		szForbidAppName[0][nWriteIndex++] = 'l';
		szForbidAppName[0][nWriteIndex++] = 'l';
		szForbidAppName[0][nWriteIndex++] = 'y';
		szForbidAppName[0][nWriteIndex++] = 'i';
		szForbidAppName[0][nWriteIndex++] = 'c';
		szForbidAppName[0][nWriteIndex++] = 'e';
		szForbidAppName[0][nWriteIndex++] = '.';
		szForbidAppName[0][nWriteIndex++] = 'e';
		szForbidAppName[0][nWriteIndex++] = 'x';
		szForbidAppName[0][nWriteIndex++] = 'e';
		szForbidAppName[0][nWriteIndex] = 0;

		//	cheatengine-i386
		nWriteIndex = 0;
		szForbidAppName[1][nWriteIndex++] = 'c';
		szForbidAppName[1][nWriteIndex++] = 'h';
		szForbidAppName[1][nWriteIndex++] = 'e';
		szForbidAppName[1][nWriteIndex++] = 'a';
		szForbidAppName[1][nWriteIndex++] = 't';
		szForbidAppName[1][nWriteIndex++] = 'e';
		szForbidAppName[1][nWriteIndex++] = 'n';
		szForbidAppName[1][nWriteIndex++] = 'g';
		szForbidAppName[1][nWriteIndex++] = 'i';
		szForbidAppName[1][nWriteIndex++] = 'n';
		szForbidAppName[1][nWriteIndex++] = 'e';
		szForbidAppName[1][nWriteIndex++] = '-';
		szForbidAppName[1][nWriteIndex++] = 'i';
		szForbidAppName[1][nWriteIndex++] = '3';
		szForbidAppName[1][nWriteIndex++] = '8';
		szForbidAppName[1][nWriteIndex++] = '6';
		szForbidAppName[1][nWriteIndex++] = '.';
		szForbidAppName[1][nWriteIndex++] = 'e';
		szForbidAppName[1][nWriteIndex++] = 'x';
		szForbidAppName[1][nWriteIndex++] = 'e';
		szForbidAppName[0][nWriteIndex] = 0;

		//	Ollydbg
		nWriteIndex = 0;
		szForbidAppName[2][nWriteIndex++] = 'o';
		szForbidAppName[2][nWriteIndex++] = 'l';
		szForbidAppName[2][nWriteIndex++] = 'l';
		szForbidAppName[2][nWriteIndex++] = 'y';
		szForbidAppName[2][nWriteIndex++] = 'd';
		szForbidAppName[2][nWriteIndex++] = 'b';
		szForbidAppName[2][nWriteIndex++] = 'g';
		szForbidAppName[2][nWriteIndex++] = '.';
		szForbidAppName[2][nWriteIndex++] = 'e';
		szForbidAppName[2][nWriteIndex++] = 'x';
		szForbidAppName[2][nWriteIndex++] = 'e';
		szForbidAppName[2][nWriteIndex] = 0;

		const char* pszCheckApp = NULL;
		DWORD dwProcessID = 0;

		for(int i = 0; i < sizeof(szForbidAppName) / sizeof(szForbidAppName[0]); ++i)
		{
			pszCheckApp = szForbidAppName[i];
			dwProcessID = GetProcessIdFromName(pszCheckApp);

			if(0 != dwProcessID)
			{
#ifndef _DISABLE_CONSOLE
				g_xConsole.CPrint("Find a forbid app:%s",
					pszCheckApp);
#endif
				if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
				{
					LOG(ERROR) << "DESTORY_GAMEWORLD";
				}
				else
				{
					DESTORY_GAMEWORLD;
				}
			}
		}

		//	Send live message
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));
		msg.message = WM_WORLDCHECKCRC;
		msg.wParam = GetTickCount();
		WORD wCRC16 = 0;
		CalcCRC16(&wCRC16, 4, (LPBYTE)&msg.wParam);
		msg.lParam = wCRC16;
		GameWorld::GetInstance().PostRunMessage(&msg);
	}

	return uRet;
}

void* WatcherThread::GetVerifyFunctionAddr(Verify_Function _eFunc)
{
	void* pFunc = NULL;
	USHORT uTag = (int)_eFunc;

	if(0 == uTag)
	{
		//pFunc = GetFuncAddr(&GameObject::GetObject_MaxAC);
		pFunc = GetFuncAddr(&GameObject::GetObject_MaxAC);
	}
	else if(1 == uTag)
	{
		pFunc = GetFuncAddr(&GameObject::GetObject_MaxMAC);
	}
	else if(2 == uTag)
	{
		pFunc = GetFuncAddr(&GameObject::GetObject_MaxDC);
	}
	else if(3 == uTag)
	{
		pFunc = GetFuncAddr(&GameObject::GetObject_MaxMC);
	}
	else if(4 == uTag)
	{
		pFunc = GetFuncAddr(&GameObject::GetObject_MaxSC);
	}
	else if(5 == uTag)
	{
		//	Get random
		GameObject xObj;
		pFunc = GetVirtualFuncAddr(&xObj, &GameObject::GetRandomAbility);
	}
	else if(6 == uTag)
	{
		//	Receive damage
		MonsterObject xMons;
		pFunc = GetVirtualFuncAddr(&xMons, &MonsterObject::ReceiveDamage);
	}
	else if(7 == uTag)
	{
		HeroObject xHero(0);
		pFunc = GetVirtualFuncAddr(&xHero, &HeroObject::ReceiveDamage);
		char* pWrtFunc = (char*)pFunc;
		pWrtFunc += 20;
		pFunc = pWrtFunc;
	}
	else if(8 == uTag)
	{
		//	Do spell
		pFunc = GetFuncAddr(&HeroObject::DoSpell);
		char* pWrtFunc = (char*)pFunc;
		pWrtFunc += 972;
		pFunc = pWrtFunc;
	}
	else if(9 == uTag)
	{
		//	Parse attack message
		pFunc = GetFuncAddr(&MonsterObject::ParseAttackMsg);
	}
	else if(10 == uTag)
	{
		//	Drop down monster items
		void (MonsterObject::* pFunc_v_ph)(HeroObject*);
		pFunc_v_ph = &MonsterObject::DropMonsterItems;
		pFunc = GetFuncAddr(pFunc_v_ph);
	}
	else if(11 == uTag)
	{
		//	attack target
		//pFunc = GetFuncAddr(&MonsterObject::AttackTarget);
		MonsterObject xMons;
		pFunc = GetVirtualFuncAddr(&xMons, &MonsterObject::IsOutOfView);
	}
	else if(12 == uTag)
	{
		//	add money
		pFunc = GetFuncAddr(&HeroObject::AddMoney);
	}
	else if(13 == uTag)
	{
		//	gain exp
		pFunc = GetFuncAddr(&HeroObject::GainExp);
		char* pWrtFunc = (char*)pFunc;
		pWrtFunc += 110;
		pFunc = pWrtFunc;
	}
	else if(14 == uTag)
	{
		//	autogeneratemonster
		pFunc = GetFuncAddr(&GameScene::AutoGenerateMonster);
	}
	else if(15 == uTag)
	{
		//	use drug item
		pFunc = GetFuncAddr(&HeroObject::UseDrugItem);
	}
	else if(16 == uTag)
	{
		//	upgrade items
		pFunc = GetFuncAddr(&DBThread::UpgradeItems);
	}
	else if(17 == uTag)
	{
		//	upgrade attrib
		pFunc = GetFuncAddr(&DBThread::UpgradeAttrib);
	}
	else if(18 == uTag)
	{
		//	db dropdown monster items
		pFunc = GetFuncAddr(&DBThread::DBDropItemExCallback);
	}
	else if(VF_HERO_DOPACKET_FORGEITEM == uTag)
	{
		//	OnPacket const pkgPlayerForgeItem
		void (HeroObject::* pFunc_v_ppfir)(const PkgPlayerForgeItemReq&);
		pFunc_v_ppfir = &HeroObject::DoPacket;
		pFunc = GetFuncAddr(pFunc_v_ppfir);
	}
	else if(VF_WORLD_UPGRADEATTRIB == uTag)
	{
		//	upgrade attrib[gameworld]
		pFunc = GetFuncAddr(&GameWorld::UpgradeAttrib);
	}
	else if(VF_WORLD_UPGRADEITEMSWITHADDITION == uTag)
	{
		//	upgrade items with addtion
		pFunc = GetFuncAddr(&GameWorld::UpgradeItemsWithAddition);
	}
	else if(VF_HERO_UPDATESTATUS == uTag)
	{
		//	updatestatus
		HeroObject xHero(0);
		pFunc = GetVirtualFuncAddr(&xHero, &HeroObject::UpdateStatus);
	}
	else if(VF_OBJ_UPDATESTATUS == uTag)
	{
		//	update status
		GameObject xObj;
		pFunc = GetVirtualFuncAddr(&xObj, &GameObject::UpdateStatus);
	}
	else if(VF_WATCHER_THREAD_DOWORK == uTag)
	{
		//	thread_dowork
		pFunc = GetFuncAddr(&WatcherThread::Thread_DoWork);
	}
	else if(VF_WORLD_DOWORK_DELAYEDPROCESS == uTag)
	{
		//	Dowork_delayedprocess
		pFunc = GetFuncAddr(&GameWorld::DoWork_DelayedProcess);
	}

	return pFunc;
}

size_t WatcherThread::GetVerifyFuncionSize(Verify_Function _eFunc)
{
	size_t uSize = 0;

	if(_eFunc < 0 ||
		_eFunc >= VF_TOTAL)
	{
		return 0;
	}

	uSize = s_nVerifySize[_eFunc];

	if(VF_DBTHREAD_DBDROPITEMEXCALLBACK == _eFunc)
	{
		uSize = 1266;
	}

	return uSize;
}