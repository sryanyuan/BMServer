#include "CMainServer.h"
#include "CMainServerMacros.h"
#include "../GameWorld/ObjectEngine.h"
#include "../GameWorld/GameDbBuffer.h"
#include "../GameWorld/DBThread.h"
#include "../GameWorld/GameWorld.h"
#include "../GameWorld/ExceptionHandler.h"
#include "../GameWorld/ConnCode.h"
#include "../Helper.h"
#include "../../CommonModule/LoginExtendInfoParser.h"
#include "../../CommonModule/version.h"
#include "../GameWorld/GlobalAllocRecord.h"
#include <zlib.h>

/************************************************************************/
/* 提前处理的数据包
/************************************************************************/
bool CMainServer::OnPreProcessPacket(DWORD _dwIndex, DWORD _dwLSIndex, DWORD _dwUID, const char* _pExtendInfo, PkgUserLoginReq& req)
{
	RECORD_FUNCNAME_SERVER;
#ifdef _DEBUG
	if (NULL != _pExtendInfo)
	{
		LOG(INFO) << "额外登录信息:" << _pExtendInfo;
	}
#endif

	// Check connection index
	if (_dwIndex > MAX_CONNECTIONS) {
		LOG(ERROR) << "Reach max connection limit " << MAX_CONNECTIONS << ", Conn id is " << _dwIndex;
		return false;
	}

	//	登入游戏 获得ID
	LoginExtendInfoParser xLoginInfoParser(NULL);
	if (NULL != _pExtendInfo)
	{
		xLoginInfoParser.SetContent(_pExtendInfo);
		if (!xLoginInfoParser.Parse())
		{
			LOG(ERROR) << "无法解析附加登录信息" << _pExtendInfo;
		}
	}

	//	进行一次合法性检测
	if (strlen(req.stHeader.szName) > 19)
	{
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return false;;
	}

	if (req.stHeader.bJob > 2 ||
		req.stHeader.bSex > 2)
	{
		LOG(WARNING) << "玩家[" << req.stHeader.szName << "]数据非法，强行踢出";
		return false;
	}

	//	首先，检查用户是否已经在游戏中
	LoginQueryInfo info;
	info.dwConnID = 0;
	strcpy(info.szName, req.stHeader.szName);
	info.bExists = true;
	GameWorld::GetInstance().SyncIsHeroExists(&info);

	if (!info.bExists)
	{
		//	可以进行登录
		LOG(INFO) << "Player[" << req.stHeader.szName << "] login ok! index[" << _dwIndex << "] conn code[" << GetConnCode(_dwIndex) << "]";
	}
	else
	{
		//	踢出之前的玩家
		LOG(ERROR) << "Player[" << req.stHeader.szName << "] login failed! same uid relogin, index[" << _dwIndex << "]";
		ForceCloseConnection(info.dwConnID);
		// NOTE: Previous player is kicked, but current player is kicked too
		return false;
	}

	//	继续进行一次判断，看此用户是否已经登录完毕了
	if (g_pxHeros[_dwIndex] != NULL)
	{
		HeroObject* pExistHero = g_pxHeros[_dwIndex];
		LOG(ERROR) << "Player[" << req.stHeader.szName << "] already logined.";
		return false;
	}

	// Below is old version massive code, it is ugly
	if (req.xData.size() == 0)
	{
		// Hero data is empty, it represent the hero is a new player, so we should
		// initialize the player with born data

		//	进行登录操作
		HeroObject* pObj = new HeroObject(_dwIndex);
		pObj->SetMapID(0);
		pObj->GetUserData()->wCoordX = 20;
		pObj->GetUserData()->wCoordY = 16;
		pObj->GetUserData()->bJob = req.stHeader.bJob;
		ItemAttrib* pAttrib = &pObj->GetUserData()->stAttrib;
		pAttrib->level = 1;
		strcpy(pAttrib->name, req.stHeader.szName);
		pAttrib->HP = GetHeroBaseAttribHP(pAttrib->level, pObj->GetUserData()->bJob);
		pAttrib->maxHP = pAttrib->HP;
		pAttrib->MP = GetHeroBaseAttribMP(pAttrib->level, pObj->GetUserData()->bJob);
		pAttrib->maxMP = pAttrib->MP;
		pAttrib->EXPR = 0;
		pAttrib->maxEXPR = GetHeroBaseAttribExpr(pAttrib->level);
		pAttrib->sex = req.stHeader.bSex;

		if (CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
		{
			pObj->SetLSIndex(_dwLSIndex);
			pObj->SetUID(_dwUID);
			pObj->SetDonateMoney(xLoginInfoParser.GetDonateMoney());
			pObj->SetDonateLeft(xLoginInfoParser.GetDonateLeft());

			GiftItemIDVector& refGiftItems = pObj->GetGiftItemIDs();
			if (xLoginInfoParser.GetGiftCount() != 0)
			{
				refGiftItems.resize(xLoginInfoParser.GetGiftCount());

				for (int i = 0; i < xLoginInfoParser.GetGiftCount(); ++i)
				{
					refGiftItems[i] = xLoginInfoParser.GetGiftID(i);
				}
			}
		}

		if (0 != GameWorld::GetInstance().SyncOnHeroConnected(pObj, true)) {
			//	delete the hero
			SAFE_DELETE(pObj);
			g_pxHeros[_dwIndex] = NULL;
		}
		else {
			//	record the player in the global table
			if (_dwIndex <= MAX_CONNECTIONS)
			{
				g_pxHeros[_dwIndex] = pObj;
			}

			//	Set mapped key
			++m_dwUserNumber;
			UpdateServerState();

			AddInformation("玩家[%s]登入游戏",
				req.stHeader.szName);
		}
	}
	else
	{
		const char* pData = &req.xData[0];
		DWORD dwDataLen = req.xData.size();

		static char* s_pBuf = new char[MAX_SAVEDATA_SIZE];
		GlobalAllocRecord::GetInstance()->RecordArray(s_pBuf);
		uLongf buflen = MAX_SAVEDATA_SIZE;
		uLongf srclen = dwDataLen;
		int nRet = uncompress((Bytef*)s_pBuf, &buflen, (const Bytef*)pData, srclen);
		if (nRet == Z_OK)
		{
			if (g_xMainBuffer.GetLength() > buflen)
			{
				LOG(ERROR) << "Buffer overflow.Too large hum data size:" << buflen;
				return false;
			}

			g_xMainBuffer.Reset();
			g_xMainBuffer.Write(s_pBuf, buflen);
			//delete[] pBuf;

			//	Version
			HeroObject* pObj = new HeroObject(_dwIndex);
			UserData* pUserData = pObj->GetUserData();
			pUserData->bJob = req.stHeader.bJob;
			pUserData->stAttrib.sex = req.stHeader.bSex;
			strcpy(pUserData->stAttrib.name, req.stHeader.szName);

			USHORT uVersion = 0;
			g_xMainBuffer >> uVersion;

			const char s_pszArchive[] = "Archive version:";
			const char s_pszInvalid[] = "Invalid archive";
			const char s_pszUserLogin[] = "User login:";
			const char s_pszUnsupportVersion[] = "Unsupport version:";
			char szText[MAX_PATH];

			sprintf(szText, "%s[%d]", s_pszArchive, uVersion);
			//AddInfomation("存档版本[%d]", uVersion);
			AddInformation(szText);
			pObj->SetVersion(uVersion);

			if (CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
			{
				pObj->SetLSIndex(_dwLSIndex);
				pObj->SetUID(_dwUID);
				pObj->SetDonateMoney(xLoginInfoParser.GetDonateMoney());
				pObj->SetDonateLeft(xLoginInfoParser.GetDonateLeft());

				GiftItemIDVector& refGiftItems = pObj->GetGiftItemIDs();
				if (xLoginInfoParser.GetGiftCount() != 0)
				{
					refGiftItems.resize(xLoginInfoParser.GetGiftCount());

					for (int i = 0; i < xLoginInfoParser.GetGiftCount(); ++i)
					{
						refGiftItems[i] = xLoginInfoParser.GetGiftID(i);
					}
				}
			}

			if (uVersion == BACKMIR_VERSION209 ||
				uVersion == BACKMIR_VERSION208)
			{
				if (LoadHumData208(pObj, g_xMainBuffer))
				{
					//	Now Ok
					if (GameWorld::GetInstance().GetThreadRunMode()) {
						DelayedProcess dp;
						dp.uOp = DP_USERLOGIN;
						dp.uParam0 = (unsigned int)pObj;
						dp.uParam1 = 1;
						GameWorld::GetInstance().AddDelayedProcess(&dp);

						//	record the player in the global table
						if (_dwIndex <= MAX_CONNECTIONS)
						{
							g_pxHeros[_dwIndex] = pObj;
						}

						//	Set mapped key
						++m_dwUserNumber;
						UpdateServerState();

						/*AddInfomation("玩家[%s]登入游戏",
						req.stHeader.szName);*/
						sprintf(szText, "%s[%s]", s_pszUserLogin, req.stHeader.szName);
						AddInformation(szText);
					}
					else {
						if (0 != GameWorld::GetInstance().SyncOnHeroConnected(pObj, false)) {
							SAFE_DELETE(pObj);
							g_pxHeros[_dwIndex] = NULL;
						}
						else {
							//	record the player in the global table
							if (_dwIndex <= MAX_CONNECTIONS)
							{
								g_pxHeros[_dwIndex] = pObj;
							}

							//	Set mapped key
							++m_dwUserNumber;
							UpdateServerState();

							/*AddInfomation("玩家[%s]登入游戏",
							req.stHeader.szName);*/
							sprintf(szText, "%s[%s]", s_pszUserLogin, req.stHeader.szName);
							AddInformation(szText);
						}
					}
				}
				else
				{
					//AddInfomation("不支持的存档[%d]或非法的存档", uVersion);
					sprintf(szText, "%s[%d]", s_pszInvalid, uVersion);
					//AddInfomation("不支持的存档[%d]或非法的存档", uVersion);
					AddInformation(szText);

					delete pObj;
#ifdef _DEBUG
					LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
#endif
					return false;
				}
			}
			else if (uVersion == BACKMIR_VERSION210)
			{
				if (LoadHumData210(pObj, g_xMainBuffer))
				{
					//	Now Ok
					if (GameWorld::GetInstance().GetThreadRunMode()) {
						DelayedProcess dp;
						dp.uOp = DP_USERLOGIN;
						dp.uParam0 = (unsigned int)pObj;
						dp.uParam1 = 1;
						GameWorld::GetInstance().AddDelayedProcess(&dp);

						//	record the player in the global table
						if (_dwIndex <= MAX_CONNECTIONS)
						{
							g_pxHeros[_dwIndex] = pObj;
						}

						//	Set mapped key
						++m_dwUserNumber;
						UpdateServerState();

						/*AddInfomation("玩家[%s]登入游戏",
						req.stHeader.szName);*/
						sprintf(szText, "%s[%s]", s_pszUserLogin, req.stHeader.szName);
						AddInformation(szText);
					}
					else {
						if (0 != GameWorld::GetInstance().SyncOnHeroConnected(pObj, false)) {
							SAFE_DELETE(pObj);
							g_pxHeros[_dwIndex] = NULL;
						}
						else {
							//	record the player in the global table
							if (_dwIndex <= MAX_CONNECTIONS)
							{
								g_pxHeros[_dwIndex] = pObj;
							}

							//	Set mapped key
							++m_dwUserNumber;
							UpdateServerState();

							/*AddInfomation("玩家[%s]登入游戏",
							req.stHeader.szName);*/
							sprintf(szText, "%s[%s]", s_pszUserLogin, req.stHeader.szName);
							AddInformation(szText);
						}
					}
				}
				else
				{
					//AddInfomation("不支持的存档[%d]或非法的存档", uVersion);
					sprintf(szText, "%s[%d]", s_pszInvalid, uVersion);
					//AddInfomation("不支持的存档[%d]或非法的存档", uVersion);
					AddInformation(szText);

					delete pObj;
#ifdef _DEBUG
					LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
#endif
					return false;
				}
			}
			else
			{
#ifdef _DEBUG
				LOG(WARNING) << "不支持的存档版本[" << uVersion << "]";
#else
				LOG(WARNING) << uVersion;
#endif
				SAFE_DELETE(pObj);
			}
		}
		else
		{
#ifdef _DEBUG
			LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
#endif
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData110(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData111(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData112(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
}

bool CMainServer::LoadHumData113(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
}

bool CMainServer::LoadHumData114(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
}

bool CMainServer::LoadHumData115(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
}

bool CMainServer::LoadHumData116(HeroObject* _pHero, ByteBuffer& _xBuf)
{
#ifdef _DEBUG
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel >= 1)
			{
				if (_pHero->AddUserMagic(wID))
				{
					--bLevel;
					for (int j = 0; j < bLevel; ++j)
					{
						_pHero->UpgradeUserMagic(wID);
					}
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//return true;
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#else
	return false;
#endif
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData117(HeroObject* _pHero, ByteBuffer& _xBuf)
{
#ifdef _DEBUG
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel > 3)
			{
				return false;
			}
			if (bLevel >= 1)
			{
				if (_pHero->AddUserMagic(wID))
				{
					--bLevel;
					for (int j = 0; j < bLevel; ++j)
					{
						_pHero->UpgradeUserMagic(wID);
					}
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	A dword reserve
	DWORD dwReserve = 0;
	_xBuf >> dwReserve;

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif

#else
	return false;
#endif
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData200(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
}

bool CMainServer::LoadHumData201(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	const MagicInfo* pMagicInfo = NULL;
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel > 3)
			{
				return false;
			}
			if (bLevel >= 1)
			{
				if (wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if (pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if (_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for (int j = 0; j < bLevel; ++j)
							{
								_pHero->UpgradeUserMagic(wID);
							}
						}
					}
#ifdef _DEBUG
#else
					else
					{
						return false;
					}
#endif
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if (_xBuf.GetLength() == 8)
	{
		DWORD dwReserve = 0;
		_xBuf >> dwReserve;
		_xBuf >> dwReserve;
	}
	else
	{
		return false;
	}

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData202(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	const MagicInfo* pMagicInfo = NULL;
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel > 3)
			{
				return false;
			}
			if (bLevel >= 1)
			{
				if (wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if (pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if (_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for (int j = 0; j < bLevel; ++j)
							{
								_pHero->UpgradeUserMagic(wID);
							}
						}
					}
#ifdef _DEBUG
#else
					else
					{
						return false;
					}
#endif
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if (_xBuf.GetLength() == 8)
	{
		DWORD dwReserve = 0;
		_xBuf >> dwReserve;
		_xBuf >> dwReserve;
	}
	else
	{
		return false;
	}

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif
}

bool CMainServer::LoadHumData203(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	const MagicInfo* pMagicInfo = NULL;
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel > 3)
			{
				return false;
			}
			if (bLevel >= 1)
			{
				if (wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if (pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if (_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for (int j = 0; j < bLevel; ++j)
							{
								_pHero->UpgradeUserMagic(wID);
							}
						}
					}
#ifdef _DEBUG
#else
					else
					{
						return false;
					}
#endif
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if (_xBuf.GetLength() == 8)
	{
		DWORD dwReserve = 0;
		_xBuf >> dwReserve;
		_xBuf >> dwReserve;
	}
	else
	{
		return false;
	}

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData204(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	const MagicInfo* pMagicInfo = NULL;
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel > 3)
			{
				return false;
			}
			if (bLevel >= 1)
			{
				if (wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if (pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if (_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for (int j = 0; j < bLevel; ++j)
							{
								_pHero->UpgradeUserMagic(wID);
							}
						}
					}
#ifdef _DEBUG
#else
					else
					{
						return false;
					}
#endif
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if (_xBuf.GetLength() == 8)
	{
		DWORD dwReserve = 0;
		_xBuf >> dwReserve;
		_xBuf >> dwReserve;
	}
	else
	{
		return false;
	}

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData205(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	const MagicInfo* pMagicInfo = NULL;
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel > 3)
			{
				return false;
			}
			if (bLevel >= 1)
			{
				if (wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if (pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if (_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for (int j = 0; j < bLevel; ++j)
							{
								_pHero->UpgradeUserMagic(wID);
							}
						}
					}
#ifdef _DEBUG
#else
					else
					{
						return false;
					}
#endif
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if (_xBuf.GetLength() == 8)
	{
		DWORD dwReserve = 0;
		_xBuf >> dwReserve;
		_xBuf >> dwReserve;
	}
	else
	{
		return false;
	}

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData206(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	const MagicInfo* pMagicInfo = NULL;
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel > 3)
			{
				return false;
			}
			if (bLevel >= 1)
			{
				if (wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if (pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if (_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for (int j = 0; j < bLevel; ++j)
							{
								_pHero->UpgradeUserMagic(wID);
							}
						}
					}
#ifdef _DEBUG
#else
					else
					{
						return false;
					}
#endif
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if (_xBuf.GetLength() == 8)
	{
		DWORD dwReserve = 0;
		_xBuf >> dwReserve;
		_xBuf >> dwReserve;
	}
	else
	{
		return false;
	}

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData207(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	const MagicInfo* pMagicInfo = NULL;
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel > 3)
			{
				return false;
			}
			if (bLevel >= 1)
			{
				if (wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if (pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if (_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for (int j = 0; j < bLevel; ++j)
							{
								_pHero->UpgradeUserMagic(wID);
							}
						}
					}
#ifdef _DEBUG
#else
					else
					{
						return false;
					}
#endif
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if (_xBuf.GetLength() == 8)
	{
		DWORD dwReserve = 0;
		_xBuf >> dwReserve;
		_xBuf >> dwReserve;
	}
	else
	{
		return false;
	}

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData208(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	const MagicInfo* pMagicInfo = NULL;
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel > 7)
			{
				return false;
			}
			if (bLevel >= 1)
			{
				if (wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if (pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if (_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for (int j = 0; j < bLevel; ++j)
							{
								_pHero->UpgradeUserMagic(wID);
							}
						}
					}
#ifdef _DEBUG
#else
					else
					{
						return false;
					}
#endif
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if (_xBuf.GetLength() == 8)
	{
		DWORD dwReserve = 0;
		_xBuf >> dwReserve;
		_xBuf >> dwReserve;
	}
	else
	{
		return false;
	}

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData210(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	UserData* pUserData = _pHero->GetUserData();

	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		WORD wLastCity = 0;
		_xBuf >> wLastCity;
		_pHero->SetCityMap(wLastCity);
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = GetHeroBaseAttribHP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = GetHeroBaseAttribMP(pUserData->stAttrib.level, pUserData->bJob);
		if (pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = GetHeroBaseAttribExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if (_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if (_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	ItemAttrib item;
	if (bBag > 0)
	{
		for (int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}
	if (bBody > 0)
	{
		BYTE bPos = 0;
		for (int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}

	const MagicInfo* pMagicInfo = NULL;
	if (bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for (int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if (bLevel > 7)
			{
				return false;
			}
			if (bLevel >= 1)
			{
				if (wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if (pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if (_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for (int j = 0; j < bLevel; ++j)
							{
								_pHero->UpgradeUserMagic(wID);
							}
						}
					}
#ifdef _DEBUG
#else
					else
					{
						return false;
					}
#endif
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if (_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if (bStorage > 0)
	{
		ItemAttrib item;

		for (int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if (item.id == 97)
			{
				if (item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if (_xBuf.GetLength() >= 8)
	{
		DWORD dwReserve = 0;
		_xBuf >> dwReserve;
		_xBuf >> dwReserve;
	}
	else
	{
		return false;
	}

	//	save extend fields
	if (!_pHero->ReadSaveExtendField(_xBuf))
	{
		return false;
	}

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif
}