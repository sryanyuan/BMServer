#include "../CMainServer/CMainServer.h"
#include "ObjectEngine.h"
#include "GameWorld.h"
#include "GameSceneManager.h"
#include "MonsterObject.h"
#include "DBThread.h"
#include "ObjectValid.h"
#include "TeammateControl.h"
#include "OlShopManager.h"
#include <math.h>
#include <zlib.h>
#include <algorithm>
#include "../../CommonModule/HideAttribHelper.h"
#include "../../CommonModule/StoveManager.h"
#include "ObjectValid.h"
#include "../../CommonModule/PotentialAttribHelper.h"

//////////////////////////////////////////////////////////////////////////
bool SortAttribID(const ItemAttrib* _pLeft, const ItemAttrib* _pRight)
{
	ItemAttrib* pLeft = const_cast<ItemAttrib*>(_pLeft);
	ItemAttrib* pRight = const_cast<ItemAttrib*>(_pRight);
	return GETITEMATB(pLeft, ID) < GETITEMATB(pRight, ID);
}
//////////////////////////////////////////////////////////////////////////

/************************************************************************/
/* virtual void ProcessPacket(PacketHeader* _pPkt)
/************************************************************************/
void HeroObject::ProcessPacket(PacketHeader* _pPkt)
{
	BEGIN_HANDLE_PACKET(_pPkt)
		HANDLE_PACKET(PKG_GAME_SERVERSTATUS_REQ,				PkgPlayerSayReq)
		HANDLE_PACKET(PKG_GAME_USER_ACTION_REQ,					PkgUserActionReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_SAY_REQ,					PkgPlayerSayReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_CHANGEEQUIPMENT_REQ,		PkgPlayerChangeEquipReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_DROPITEM_REQ,				PkgPlayerDropItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_PICKUPITEM_REQ,			PkgPlayerPickUpItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_UNDRESSITEM_REQ,			PkgPlayerUndressItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_DRESSITEM_REQ,			PkgPlayerDressItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_USEITEM_REQ,				PkgPlayerUseItemReq)
		HANDLE_PACKET(PKG_GAME_LOADED,							PkgGameLoadedAck)
		HANDLE_PACKET(PKG_GAME_PLAYER_CLICKNPC_REQ,				PkgPlayerClickNPCReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_ATTACK_REQ,				PkgPlayerAttackReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_SYNCASSIST_REQ,			PkgPlayerSyncAssistReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_SHOPOP_REQ,				PkgPlayerShopOpReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_USERDATA_REQ,				PkgPlayerUserDataReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_MONSINFO_REQ,				PkgPlayerMonsInfoReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_REVIVE_REQ,				PkgPlayerReviveReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_SLAVE_STOP_REQ,			PkgPlayerSlaveStopReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_CUBEITEMS_REQ,			PkgPlayerCubeItemsReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_CALLSLAVE_REQ,			PkgPlayerCallSlaveReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_KILLSLAVE_REQ,			PkgPlayerKillSlaveReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_SPEOPERATE_REQ,			PkgPlayerSpeOperateReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_MERGYCOSTITEM_REQ,		PkgPlayerMergyCostItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_NETDELAY_REQ,				PkgPlayerNetDelayReq)
		HANDLE_PACKET(PKG_GAME_GM_NOTIFICATION_REQ,				PkgGmNotificationReq)
		HANDLE_PACKET(PKG_SYSTEM_NOTIFY_REQ,					PkgSystemNotifyReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_DECOMPOSE_REQ,			PkgPlayerDecomposeReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_FORGEITEM_REQ,			PkgPlayerForgeItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_SPLITITEM_REQ,			PkgPlayerSplitItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_OFFSELLITEM_REQ,			PkgPlayerOffSellItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_OFFBUYITEM_REQ,			PkgPlayerOffBuyItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_OFFGETLIST_REQ,			PkgPlayerOffGetListReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_OFFCHECKSOLD_REQ,			PkgPlayerOffCheckSoldReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_PRIVATECHAT_REQ,			PkgPlayerPrivateChatReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_OFFTAKEBACK_REQ,			PkgPlayerOffTakeBackReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_IDENTIFYITEM_REQ,			PkgPlayerIdentifyItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_UNBINDITEM_REQ,			PkgPlayerUnbindItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_SERVERDELAY_ACK,			PkgPlayerServerDelayAck)
		HANDLE_PACKET(PKG_GAME_PLAYER_RANKLIST_REQ,				PkgPlayerRankListReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_GETOLSHOPLIST_REQ,		PkgPlayerGetOlShopListReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_BUYOLSHOPITEM_REQ,		PkgPlayerBuyOlShopItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_SMELTMATERIALS_REQ,		PkgPlayerSmeltMaterialsReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_HAND_MAKE_ITEM_REQ,		PkgPlayerHandMakeItemReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_OPEN_POTENTIAL_REQ,		PkgPlayerOpenPotentialReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_CHARGE_REQ,				PkgPlayerChargeReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_WORLD_SAY_REQ,			PkgPlayerWorldSayReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_LOGINEXTDATA_REQ,			PkgPlayerLoginExtDataReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_DIFFICULTYLEVEL_REQ,		PkgPlayerDifficultyLevelReq)
		HANDLE_PACKET(PKG_GAME_PLAYER_QUITSELCHR_REQ,			PkgPlayerQuitSelChrReq)
	END_PROCESS_PACKET()

	SetLastRecvDataTime(GetTickCount());
}

/************************************************************************/
/* void DoPacket(const PkgUserActionReq& req)
/************************************************************************/
void HeroObject::DoPacket(const PkgUserActionReq& req)
{
	//char szMsg[MAX_PATH];
	DWORD dwCurrentTick = GetTickCount();
	GameScene* pScene = GetLocateScene();
	WORD wPosX = m_stData.wCoordX;
	WORD wPosY = m_stData.wCoordY;
	WORD wDestPosX = LOWORD(req.uParam1);
	WORD wDestPosY = HIWORD(req.uParam1);
	int nOftX = 0;
	int nOftY = 0;
	int nDrt = -1;

	if(m_stData.eGameState == OS_DEAD)
	{
		return;
	}
	if(GetServerNetDelay() == 0xffffffff)
	{
		return;
	}

	bool bIsStone = TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_STONE);

	if(req.uAction == ACTION_WALK)
	{
		//m_stData.wCoordX = LOWORD(req.uParam1);
		//m_stData.wCoordY = HIWORD(req.uParam1);
		//m_stData.eGameState = OS_WALK;
		///*m_stData.*/m_dwLastWalkTime = dwCurrentTick;
		//g_xConsole.CPrint("Walk interval:%d", dwCurrentTick - m_dwLastWalkTime);

		if(dwCurrentTick - m_dwLastWalkTime < 250)
		{
			++m_dwTimeOut;
			ForceGotoValidPosition();
			return;
		}
		if(bIsStone)
		{
			ForceGotoValidPosition();
			return;
		}

		nOftX = wDestPosX - wPosX;
		nOftY = wDestPosY - wPosY;
		for(int i = 0; i < 8; ++i)
		{
			if(g_nMoveOft[i * 2] == nOftX &&
				g_nMoveOft[i * 2 + 1] == nOftY)
			{
				nDrt = i;
				break;
			}
		}

		if(nDrt != -1)
		{
			if(!WalkOneStep(nDrt))
			{
				//m_stData.eGameState = OS_STAND;
				//m_stData.wCoordX = wPosX;
				//m_stData.wCoordY = wPosY;
				//PkgPlayerForceActionAck ack;
				//ack.bType = FORCE_POSITION;
				//ack.dwData = MAKELONG(wPosX, wPosY);
				//g_xThreadBuffer.Reset();
				//g_xThreadBuffer << ack;
				//SendPlayerBuffer(g_xThreadBuffer);
				//LOG(INFO) << "Can't move";
				DEBUG_BREAK;
				ForceGotoValidPosition();
			}
			else
			{
				//	Reset hide mode
				if(IsHide())
				{
					ResetEffStatus(MMASK_HIDE);

					PkgPlayerSetEffectAck ack;
					ack.uTargetId = GetID();
					ack.dwMgcID = MMASK_HIDE;
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ack;
					GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
				}
				if(GetLocateScene()->IsInCity(m_stData.wCoordX, m_stData.wCoordY))
				{
					m_dwLastCityMap = m_stData.wMapID;
				}
				CheckDoorEvent();
			}
		}
		else
		{
			/*
			m_stData.eGameState = OS_STAND;
						m_stData.wCoordX = wPosX;
						m_stData.wCoordY = wPosY;
						PkgPlayerForceActionAck ack;
						ack.bType = FORCE_POSITION;
						ack.dwData = MAKELONG(wPosX, wPosY);
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ack;
						SendPlayerBuffer(g_xThreadBuffer);
						LOG(INFO) << "Can't move";*/
			DEBUG_BREAK;
			ForceGotoValidPosition();
		}
	}
	else if(req.uAction == ACTION_RUN)
	{
		//m_stData.wCoordX = LOWORD(req.uParam1);
		//m_stData.wCoordY = HIWORD(req.uParam1);
		//m_stData.eGameState = OS_RUN;
		///*m_stData.*/m_dwLastRunTime = dwCurrentTick;
		//g_xConsole.CPrint("Run interval:%d", dwCurrentTick - m_dwLastRunTime);

		if(dwCurrentTick - m_dwLastRunTime < 250)
		{
			++m_dwTimeOut;
			ForceGotoValidPosition();
			return;
		}

		if(bIsStone)
		{
			ForceGotoValidPosition();
			return;
		}

		nOftX = wDestPosX - wPosX;
		nOftY = wDestPosY - wPosY;
		for(int i = 0; i < 8; ++i)
		{
			if(g_nMoveOft[i * 2] * 2 == nOftX &&
				g_nMoveOft[i * 2 + 1] * 2 == nOftY)
			{
				nDrt = i;
				break;
			}
		}

		if(nDrt != -1)
		{
			if(!RunOneStep(nDrt))
			{
				/*
				m_stData.eGameState = OS_STAND;
								m_stData.wCoordX = wPosX;
								m_stData.wCoordY = wPosY;
								PkgPlayerForceActionAck ack;
								ack.bType = FORCE_POSITION;
								ack.dwData = MAKELONG(wPosX, wPosY);
								g_xThreadBuffer.Reset();
								g_xThreadBuffer << ack;
								SendPlayerBuffer(g_xThreadBuffer);
								LOG(INFO) << "Can't run";*/
				DEBUG_BREAK;
				ForceGotoValidPosition();
				
			}
			else
			{
				if(IsHide())
				{
					ResetEffStatus(MMASK_HIDE);

					PkgPlayerSetEffectAck ack;
					ack.uTargetId = GetID();
					ack.dwMgcID = MMASK_HIDE;
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ack;
					GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
				}

				if(GetLocateScene()->IsInCity(m_stData.wCoordX, m_stData.wCoordY))
				{
					m_dwLastCityMap = m_stData.wMapID;
				}
				//CheckDoorEvent();
			}
		}
		else
		{
			/*
			m_stData.eGameState = OS_STAND;
						m_stData.wCoordX = wPosX;
						m_stData.wCoordY = wPosY;
						PkgPlayerForceActionAck ack;
						ack.bType = FORCE_POSITION;
						ack.dwData = MAKELONG(wPosX, wPosY);
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ack;
						SendPlayerBuffer(g_xThreadBuffer);
						LOG(INFO) << "Can't run";*/
			DEBUG_BREAK;
			ForceGotoValidPosition();
		}
	}
	else if(req.uAction == ACTION_TURN)
	{
		//m_stData.wCoordX = LOWORD(req.uParam0);
		//m_stData.wCoordY = HIWORD(req.uParam0);
		//m_stData.nDrt = (int)req.uParam1;
		TurnTo((int)req.uParam1);
	}
	else if(req.uAction == ACTION_ATTACK)
	{
		WORD wNowX = LOWORD(req.uParam0);
		WORD wNowY = HIWORD(req.uParam0);
		if(wNowX != GetValidPositionX() ||
			wNowY != GetValidPositionY())
		{
			ForceGotoValidPosition();
			return;
		}

		int nAttackInterval = GetHeroAttackInterval();
		//nAttackInterval -= int(GetServerNetDelay());
		nAttackInterval -= 250;
		if(nAttackInterval < 0)
		{
			nAttackInterval = 0;
		}

		//g_xConsole.CPrint("Attack interval:%d, need interval:%d", dwCurrentTick - m_dwLastAttackTime, nAttackInterval);

		if(dwCurrentTick - m_dwLastAttackTime < nAttackInterval)
		{
			LOG(WARNING) << "Attack timeout:" << GetName() << " last attack time:" << m_dwLastAttackTime << " now time:" << dwCurrentTick << " interval:" << (GetTickCount() - m_dwLastAttackTime) << " need interval:" << nAttackInterval;
			++m_dwTimeOut;
			return;
		}
		if(bIsStone)
		{
			return;
		}
		MonsterObject* pMonster = NULL;
		HeroObject* pHero = NULL;
		MapCellInfo* pCell = NULL;
		int nDestX = LOWORD(req.uParam0);
		int nDestY = HIWORD(req.uParam0);
		if(req.uParam1 > 7)
		{
			return;
		}
	
		int nAttackDelayTime = 350 - GetServerNetDelay();
		if(nAttackDelayTime < 0)
		{
			nAttackDelayTime = 0;
		}

		nDestX += g_nMoveOft[req.uParam1 * 2];
		nDestY += g_nMoveOft[req.uParam1 * 2 + 1];
		
		GameObjectList xObjList;
		if(0 != GetLocateScene()->GetMappedObjects(nDestX, nDestY, 0, 0, xObjList, MAPPEDOBJECT_ALL_ALIVE))
		{
			GameObjectList::iterator it = xObjList.begin();
			for(it;
				it != xObjList.end();
				++it)
			{
				GameObject* pAttacked = *it;

				if(pAttacked->GetType() == SOT_MONSTER)
				{
					pMonster = (MonsterObject*)pAttacked;
				}
				else if(pAttacked->GetType() == SOT_HERO)
				{
					pHero = (HeroObject*)pAttacked;
				}
			}
		}

		m_stData.wCoordX = LOWORD(req.uParam0);
		m_stData.wCoordY = HIWORD(req.uParam0);
		m_stData.nDrt = req.uParam1;
		/*m_stData.*/m_dwLastAttackTime = dwCurrentTick;

		int nAtkRand = m_xNormalAttackRand.Rand(0, 99);
		if(req.uParam2 != nAtkRand)
		{
			SyncRandSeedNormalAtk();
		}

		PkgObjectActionNot not;
		not.uAction = req.uAction;
		not.uParam0 = req.uParam0;
		not.uParam1 = req.uParam1;
		not.uParam2 = 0;
		not.uParam3 = req.uParam3;
		not.uTargetId = GetID();


		WORD wDamage = 0;

		if(pMonster)
		{
			//SetSlaveTarget(pMonster);
		}

		//	normal attack
		/*int nAC = 0;
		if(pMonster)
		{
			nAC = pMonster->GetRandomAbility(AT_AC);
		}
		int nDC = GetRandomAbility(AT_DC);*/


		if(m_stData.bJob != 0)
		{
			if(pMonster)
			{
				if(0 != pMonster->ReceiveDamage(this, false, 0, nAttackDelayTime))
				{
					SetSlaveTarget(pMonster);
				}
			}
			if(pHero)
			{
				if(0 != pHero->ReceiveDamage(this, false, 0, nAttackDelayTime))
				{
					SetSlaveTarget(pHero);
				}
			}
		}
		else
		{
			DoSpeHit(pMonster, pHero, (DWORD*)&not.uParam2);
		}

		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		pScene->BroadcastPacket(&g_xThreadBuffer);
	}
	else if(req.uAction == ACTION_STRUCK)
	{
		m_stData.wCoordX = LOWORD(req.uParam0);
		m_stData.wCoordY = HIWORD(req.uParam0);
		if(req.uParam1 <= 7)
		{
			m_stData.nDrt = req.uParam1;
		}
		/*m_stData.*/m_dwLastStruckTime = dwCurrentTick;
		PkgUserActionReq& wrtReq = const_cast<PkgUserActionReq&>(req);
		wrtReq.uParam2 = MAKELONG(GetObject_HP(), GetObject_MaxHP());

		PkgPlayerUpdateAttribNtf ntf;
		ntf.uTargetId = GetID();
		ntf.bType = UPDATE_HP;
		ntf.dwParam = GetObject_HP();
		
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ntf;
		SendPlayerBuffer(g_xThreadBuffer);

		PkgObjectActionNot not;
		not.uAction = req.uAction;
		not.uParam0 = req.uParam0;
		not.uParam1 = req.uParam1;
		not.uParam2 = req.uParam2;
		not.uParam3 = req.uParam3;
		not.uTargetId = GetID();

		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		pScene->BroadcastPacket(&g_xThreadBuffer, GetID());
	}
	else if(req.uAction == ACTION_SPELL)
	{
		if(bIsStone)
		{
			return;
		}
		WORD wNowX = LOWORD(req.uParam0);
		WORD wNowY = HIWORD(req.uParam0);
		if(wNowX != GetValidPositionX() ||
			wNowY != GetValidPositionY())
		{
			ForceGotoValidPosition();
			return;
		}

		DoSpell(req);
	}

	/*PkgObjectActionNot not;
	not.uAction = req.uAction;
	not.uParam0 = req.uParam0;
	not.uParam1 = req.uParam1;
	not.uParam2 = req.uParam2;
	not.uParam3 = req.uParam3;
	not.uTargetId = GetID();

	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer, GetID());*/
}

void HeroObject::DoPacket(const PkgPlayerChangeEquipReq& req)
{
	ItemAttrib* pItem = const_cast<ItemAttrib*>(&req.stItem);
	if(GETITEMATB(pItem, Type) == ITEM_CLOTH ||
		GETITEMATB(pItem, Type) == ITEM_WEAPON)
	{
		PkgPlayerChangeEquipNot not;
		memcpy(&not.stItem, &req.stItem, sizeof(ItemAttrib));
		not.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer, GetID());
	}
}

void HeroObject::DoPacket(const PkgPlayerSayReq& req)
{
	if(req.uType == 2)
	{
#ifdef _DEBUG
		std::string xMsg = "[GM]";
		xMsg += req.xWords;
		SendSystemMessage(xMsg.c_str());
#endif
	}
	else
	{
		/*PkgChatNot not;
		not.xMsg += req.xWords;
		not.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer, GetID());*/
		PkgPlayerSayNot not;
		not.xMsg = req.xWords;
		not.uColor = m_stExtAttrib.uChatColor;
		not.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer, GetID());
	}
}

void HeroObject::DoPacket(const PkgPlayerDropItemReq& req)
{
	//	find if the item exists
	ItemAttrib* pDropedItem = NULL;
	pDropedItem = GetItemByTag(req.dwId);
	if(pDropedItem != NULL)
	{
		/*
		PkgPlayerDropItemNot not;
				not.uTargetId = GetID();
				not.dwID = pDropedItem->id;
				not.dwTag = pDropedItem->tag;
				not.wCoordX = m_stData.wCoordX;
				not.wCoordY = m_stData.wCoordY;
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << not;
				GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer);*/
		
		/*
		GroundItem item;
				item.wPosX = m_stData.wCoordX;
				item.wPosY = m_stData.wCoordY;
				memcpy(&item.stAttrib, pDropedItem, sizeof(ItemAttrib));
				item.wID = GetTickCount();*/
		//if(pDropedItem->atkPois > 0)
		if(TEST_FLAG_BOOL(GETITEMATB(pDropedItem, AtkPois), POIS_MASK_BIND))
		{
			//	destory
			PkgPlayerLostItemAck ack;
			ack.uTargetId = GetID();
			ack.dwTag = pDropedItem->tag;
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ack;
			SendBuffer(GetUserIndex(), &g_xThreadBuffer);
			memset(pDropedItem, 0, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(pDropedItem);
		}
		else
		{
			/*
			PkgPlayerDropItemNot not;
						not.uTargetId = GetID();
						not.dwID = pDropedItem->id;
						not.dwTag = pDropedItem->tag;
						not.wCoordX = m_stData.wCoordX;
						not.wCoordY = m_stData.wCoordY;
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << not;
						GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer);*/
			
			if(GETITEMATB(pDropedItem, Type) == ITEM_COST)
			{
				int nNumber = GETITEMATB(pDropedItem, AtkSpeed);

				/*if(nNumber >= 1 &&
					req.uTargetId >= 1 &&
					req.uTargetId <= nNumber)
				{
					GroundItem item;
					item.wPosX = m_stData.wCoordX;
					item.wPosY = m_stData.wCoordY;
					memcpy(&item.stAttrib, pDropedItem, sizeof(ItemAttrib));
					item.wID = GetTickCount();

					//--nNumber;
					nNumber -= req.uTargetId;

					PkgPlayerUpdateCostNtf ntf;
					ntf.uTargetId = GetID();
					ntf.dwTag = pDropedItem->tag;
					ntf.nNumber = nNumber;
					SendPacket(ntf);

					if(nNumber == 0)
					{
						memset(pDropedItem, 0, sizeof(ItemAttrib));
						ObjectValid::EncryptAttrib(pDropedItem);
					}
					else
					{
						SETITEMATB(pDropedItem, AtkSpeed, nNumber);
					}

					//	Cost item need new tag
					item.stAttrib.tag = GameWorld::GetInstance().GenerateItemTag();
					GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->InsertItem(&item);
				}*/

				if(nNumber >= 1)
				{
					GroundItem item;
					item.wPosX = m_stData.wCoordX;
					item.wPosY = m_stData.wCoordY;
					memcpy(&item.stAttrib, pDropedItem, sizeof(ItemAttrib));
					item.wID = GetTickCount();

					--nNumber;
					PkgPlayerUpdateCostNtf ntf;
					ntf.uTargetId = GetID();
					ntf.dwTag = pDropedItem->tag;
					ntf.nNumber = nNumber;
					SendPacket(ntf);

					if(nNumber == 0)
					{
						memset(pDropedItem, 0, sizeof(ItemAttrib));
						ObjectValid::EncryptAttrib(pDropedItem);
					}
					else
					{
						SETITEMATB(pDropedItem, AtkSpeed, nNumber);
					}

					//	Cost item need new tag
					item.stAttrib.tag = GameWorld::GetInstance().GenerateItemTag();
					GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->InsertItem(&item);
				}
			}
			else
			{
				GroundItem item;
				item.wPosX = m_stData.wCoordX;
				item.wPosY = m_stData.wCoordY;
				memcpy(&item.stAttrib, pDropedItem, sizeof(ItemAttrib));
				item.wID = GetTickCount();

				PkgPlayerLostItemAck ack;
				ack.uTargetId = GetID();
				ack.dwTag = pDropedItem->tag;
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ack;
				SendBuffer(GetUserIndex(), &g_xThreadBuffer);

				GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->InsertItem(&item);
				memset(pDropedItem, 0, sizeof(ItemAttrib));
				ObjectValid::EncryptAttrib(pDropedItem);
			}
		}
		
		
		//GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->InsertItem(&item);
	}
}

void HeroObject::DoPacket(const PkgPlayerPickUpItemReq& req)
{
	//	find if the tag exists
	GroundItem* pItem = GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->GetItem(req.dwTag);
	if(pItem)
	{
		int nMoney = 0;
		if(GETITEMATB(&pItem->stAttrib, ID) >= 112 &&
			GETITEMATB(&pItem->stAttrib, ID) <= 116)
		{
			if(pItem->wPosX == m_stData.wCoordX &&
				pItem->wPosY == m_stData.wCoordY)
			{
				if(GETITEMATB(&pItem->stAttrib, ID) == 112)
				{
					nMoney = rand() % 50 + 20;
				}
				else if(GETITEMATB(&pItem->stAttrib, ID) == 113)
				{
					nMoney = 70 + rand() % 50;
				}
				else if(GETITEMATB(&pItem->stAttrib, ID) == 114)
				{
					nMoney = 120 + rand() % 100;
				}
				else if(GETITEMATB(&pItem->stAttrib, ID) == 115)
				{
					nMoney = 220 + rand() % 400;
				}
				else if(GETITEMATB(&pItem->stAttrib, ID) == 116)
				{
					nMoney = 620 + rand() % 1200;
				}

				AddMoney(nMoney);
				PkgPlayerUpdateAttribNtf uantf;
				uantf.uTargetId = GetID();
				uantf.bType = UPDATE_MONEY;
				uantf.dwParam = GetMoney();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << uantf;
				SendPlayerBuffer(g_xThreadBuffer);

				//	clean the ground item
				//PkgPlayerClearItemNtf cntf;
				PkgSystemClearGroundItemNtf cntf;
				cntf.dwTag = pItem->stAttrib.tag;
				cntf.uTargetId = GetID();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << cntf;
				GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer);
				GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->RemoveItem(req.dwTag);
			}
		}
		else if(pItem->wPosX == m_stData.wCoordX &&
			pItem->wPosY == m_stData.wCoordY)
		{
			ItemAttrib* pBagItem = NULL;
			if(GETITEMATB(&pItem->stAttrib, Type) == ITEM_COST)
			{
				//	1. cost items
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					//	search the same item that already exists
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_COST &&
						GETITEMATB(&m_xBag[i], ID) == GETITEMATB(&pItem->stAttrib, ID) &&
						!TEST_FLAG_BOOL(GETITEMATB(&m_xBag[i], AtkPois), POIS_MASK_BIND))
					{
						if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX)
						{
							pBagItem = &m_xBag[i];
							break;
						}
					}
				}

				if(pBagItem == NULL)
				{
					//	search empty item
					for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
					{
						if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
							m_xBag[i].tag != ITEMTAG_INQUERY)
						{
							pBagItem = &m_xBag[i];
							break;
						}
					}
				}

				if(NULL != pBagItem)
				{
					//	Can save 20 items
					if(GETITEMATB(pBagItem, Type) == ITEM_NO)
					{
						memcpy(pBagItem, &pItem->stAttrib, sizeof(ItemAttrib));
						SETITEMATB(pBagItem, AtkSpeed, 1);

						// notify player
						PkgPlayerGainItemNtf ntf;
						ntf.stItem = *pBagItem;
						ObjectValid::DecryptAttrib(&ntf.stItem);
						ntf.uTargetId = GetID();
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendBuffer(GetUserIndex(), &g_xThreadBuffer);

						//	clean the ground item
						//PkgPlayerClearItemNtf cntf;
						PkgSystemClearGroundItemNtf cntf;
						cntf.dwTag = pItem->stAttrib.tag;
						cntf.uTargetId = GetID();
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << cntf;
						GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer);
						GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->RemoveItem(req.dwTag);
					}
					else if(GETITEMATB(pBagItem, Type) == ITEM_COST)
					{
						int nSum = GETITEMATB(pBagItem, AtkSpeed) + 1;
						if(nSum <= GRID_MAX)
						{
							SETITEMATB(pBagItem, AtkSpeed, nSum);

							PkgPlayerUpdateCostNtf ntf;
							ntf.uTargetId = GetID();
							ntf.nNumber = nSum;
							ntf.dwTag = pBagItem->tag;
							g_xThreadBuffer.Reset();
							g_xThreadBuffer << ntf;
							SendPlayerBuffer(g_xThreadBuffer);
						}

						//	clean the ground item
						//PkgPlayerClearItemNtf cntf;
						PkgSystemClearGroundItemNtf cntf;
						cntf.dwTag = pItem->stAttrib.tag;
						cntf.uTargetId = GetID();
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << cntf;
						GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer);
						GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->RemoveItem(req.dwTag);
					}
				}
			}
			else
			{
				//	2.other items
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
						m_xBag[i].tag != ITEMTAG_INQUERY)
					{
						pBagItem = &m_xBag[i];
						break;
					}
				}

				if(NULL != pBagItem)
				{
					if(GETITEMATB(pBagItem, Type) == ITEM_NO)
					{
						memcpy(pBagItem, &pItem->stAttrib, sizeof(ItemAttrib));

						// notify player
						PkgPlayerGainItemNtf ntf;
						ntf.stItem = *pBagItem;
						ObjectValid::DecryptAttrib(&ntf.stItem);
						ntf.uTargetId = GetID();
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						SendBuffer(GetUserIndex(), &g_xThreadBuffer);

						//	clean the ground item
						//PkgPlayerClearItemNtf cntf;
						PkgSystemClearGroundItemNtf cntf;
						cntf.dwTag = pItem->stAttrib.tag;
						cntf.uTargetId = GetID();
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << cntf;
						GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer);
						GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->RemoveItem(req.dwTag);
					}
				}
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerUndressItemReq& req)
{
	if(GETITEMATB(&m_stEquip[req.bPos], Type) != ITEM_NO &&
		m_stEquip[req.bPos].tag == req.dwTag)
	{
		PkgPlayerUndressItemAck ack;
		ack.bPos = req.bPos;
		ack.dwTag = req.dwTag;
		ack.uTargetId = GetID();

		ItemAttrib* pItem = &m_stEquip[req.bPos];
		if(AddBagItem(pItem))
		{
			ZeroMemory(pItem, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(pItem);
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ack;
			GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer);
			RefleshAttrib();
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerDressItemReq& req)
{
	//	First check if the destinate rect have item
	if(req.bPos >= (BYTE)PLAYER_ITEM_TOTAL)
	{
		return;
	}
	
	ItemAttrib* pTargetItem = &m_stEquip[(PLAYER_ITEM_TYPE)req.bPos];
	ItemAttrib* pWantDressItem = GetItemByTag(req.dwTag);

	if(pWantDressItem == NULL)
	{
		return;
	}
	if(!IsEquipItem(GETITEMATB(pWantDressItem, Type)))
	{
		return;
	}
	if(GETITEMATB(pWantDressItem, Type) == ITEM_WEAPON)
	{
		if(req.bPos != PLAYER_ITEM_WEAPON)
		{
			return;
		}
		else if(GetHeroWanLi() < GETITEMATB(pWantDressItem, Weight))
		{
			return;
		}
	}
	else if(GETITEMATB(pWantDressItem, Type) == ITEM_CLOTH)
	{
		if(req.bPos != PLAYER_ITEM_CLOTH)
		{
			return;
		}
	}
	else if(GETITEMATB(pWantDressItem, Type) == ITEM_NECKLACE)
	{
		if(req.bPos != PLAYER_ITEM_NECKLACE)
		{
			return;
		}
	}
	else if(GETITEMATB(pWantDressItem, Type) == ITEM_MEDAL)
	{
		if(req.bPos != PLAYER_ITEM_MEDAL)
		{
			return;
		}
	}
	else if(GETITEMATB(pWantDressItem, Type) == ITEM_HELMET)
	{
		if(req.bPos != PLAYER_ITEM_HELMET)
		{
			return;
		}
	}
	else if(GETITEMATB(pWantDressItem, Type) == ITEM_RING)
	{
		if(req.bPos != PLAYER_ITEM_RING1 &&
			req.bPos != PLAYER_ITEM_RING2)
		{
			return;
		}
	}
	else if(GETITEMATB(pWantDressItem, Type) == ITEM_BRACELAT)
	{
		if(req.bPos != PLAYER_ITEM_BRACELAT1 &&
			req.bPos != PLAYER_ITEM_BRACELAT2)
		{
			return;
		}
	}
	else if(GETITEMATB(pWantDressItem, Type) == ITEM_CHARM)
	{
		if(req.bPos != PLAYER_ITEM_CHARM)
		{
			return;
		}
	}
	else if(GETITEMATB(pWantDressItem, Type) == ITEM_BELT)
	{
		if(req.bPos != PLAYER_ITEM_BELT)
		{
			return;
		}
	}
	else if(GETITEMATB(pWantDressItem, Type) == ITEM_SHOE)
	{
		if(req.bPos != PLAYER_ITEM_SHOE)
		{
			return;
		}
	}
	else if(GETITEMATB(pWantDressItem, Type) == ITEM_GEM)
	{
		if(req.bPos != PLAYER_ITEM_GEM)
		{
			return;
		}
	}

	BYTE bRet = 0;

	PkgPlayerDressItemAck ack;
	bRet = CheckItemCanDress(pWantDressItem);
	if(0 != bRet)
	{
		ack.bRet = bRet;
		ack.uTargetId = GetID();
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		//SendBuffer(GetUserIndex(), &g_xThreadBuffer);
		GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer);
	}
	else
	{
		ack.bRet = bRet;
		ack.uTargetId = GetID();
		ack.bPos = req.bPos;
		ack.dwTag = req.dwTag;
		ack.dwTex = GETITEMATB(pWantDressItem, Tex);
		ack.uUserId = GETITEMATB(pWantDressItem, ID);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		//SendBuffer(GetUserIndex(), &g_xThreadBuffer);
		GameSceneManager::GetInstance()->GetScene(m_stData.wMapID)->BroadcastPacket(&g_xThreadBuffer);

		if(GETITEMATB(pTargetItem, Type) == ITEM_NO)
		{
			//	Have no item
			memcpy(pTargetItem, pWantDressItem, sizeof(ItemAttrib));
			memset(pWantDressItem, 0, sizeof(ItemAttrib));
			ObjectValid::EncryptAttrib(pWantDressItem);
		}
		else
		{
			ItemAttrib item;
			memcpy(&item, pTargetItem, sizeof(ItemAttrib));
			memcpy(pTargetItem, pWantDressItem, sizeof(ItemAttrib));
			memcpy(pWantDressItem, &item, sizeof(ItemAttrib));
		}
		RefleshAttrib();

		OnPlayerDressdItem(req.dwTag);
	}
}

void HeroObject::DoPacket(const PkgPlayerUseItemReq& req)
{
	ItemAttrib* pItem = GetItemByTag(req.dwTag);
	GameScene* pScene = GameSceneManager::GetInstance()->GetScene(m_stData.wMapID);
	if(pScene == NULL)
	{
		LOG(WARNING) << "Player[" << GetID() << "] dosen't have scene!";
		return;
	}
	if(m_stData.eGameState == OS_DEAD)
	{
		return;
	}

	if(pItem)
	{
		if(IsEquipItem(GETITEMATB(pItem, Type)))
		{
			BYTE bRet = 0;
			PkgPlayerDressItemAck ack;
			bRet = CheckItemCanDress(pItem);

			if(bRet != 0)
			{
				ack.bRet = bRet;
				ack.uTargetId = GetID();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ack;
				//SendBuffer(GetUserIndex(), &g_xThreadBuffer);
				pScene->BroadcastPacket(&g_xThreadBuffer);
			}
			else
			{
				ack.bPos = 0xFF;
				switch(GETITEMATB(pItem, Type))
				{
				case ITEM_WEAPON:
					{
						ack.bPos = (BYTE)PLAYER_ITEM_WEAPON;
					}break;
				case ITEM_CLOTH:
					{
						ack.bPos = (BYTE)PLAYER_ITEM_CLOTH;
					}break;
				case ITEM_NECKLACE:
					{
						ack.bPos = (BYTE)PLAYER_ITEM_NECKLACE;
					}break;
				case ITEM_MEDAL:
					{
						ack.bPos = (BYTE)PLAYER_ITEM_MEDAL;
					}break;
				case ITEM_HELMET:
					{
						ack.bPos = (BYTE)PLAYER_ITEM_HELMET;
					}break;
				case ITEM_BELT:
					{
						ack.bPos = PLAYER_ITEM_BELT;
					}break;
				case ITEM_SHOE:
					{
						ack.bPos = PLAYER_ITEM_SHOE;
					}break;
				case ITEM_CHARM:
					{
						ack.bPos = PLAYER_ITEM_CHARM;
					}break;
				case ITEM_GEM:
					{
						ack.bPos = PLAYER_ITEM_GEM;
					}break;
				case ITEM_RING:
					{
						if(GETITEMATB(&m_stEquip[PLAYER_ITEM_RING1], Type) == ITEM_NO)
						{
							ack.bPos = (BYTE)PLAYER_ITEM_RING1;
						}
						else if(GETITEMATB(&m_stEquip[PLAYER_ITEM_RING2], Type) == ITEM_NO)
						{
							ack.bPos = (BYTE)PLAYER_ITEM_RING2;
						}
						else
						{
							ack.bPos = (BYTE)PLAYER_ITEM_RING1;
						}
					}break;
				case ITEM_BRACELAT:
					{
						if(GETITEMATB(&m_stEquip[PLAYER_ITEM_BRACELAT1], Type) == ITEM_NO)
						{
							ack.bPos = (BYTE)PLAYER_ITEM_BRACELAT1;
						}
						else if(GETITEMATB(&m_stEquip[PLAYER_ITEM_BRACELAT2], Type) == ITEM_NO)
						{
							ack.bPos = PLAYER_ITEM_BRACELAT2;
						}
						else
						{
							ack.bPos = PLAYER_ITEM_BRACELAT1;
						}
					}break;
				}

				if(ack.bPos != 0xFF)
				{
					ack.bRet = bRet;
					ack.dwTag = pItem->tag;
					ack.dwTex = GETITEMATB(pItem, Tex);
					ack.uTargetId = GetID();
					ack.uUserId = GETITEMATB(pItem, ID);
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ack;
					//SendBuffer(GetUserIndex(), &g_xThreadBuffer);
					pScene->BroadcastPacket(&g_xThreadBuffer);

					if(GETITEMATB(&m_stEquip[ack.bPos], Type) != ITEM_NO)
					{
						//	Change
						ItemAttrib item;
						memcpy(&item, &m_stEquip[ack.bPos], sizeof(ItemAttrib));
						memcpy(&m_stEquip[ack.bPos], pItem, sizeof(ItemAttrib));
						memcpy(pItem, &item, sizeof(ItemAttrib));
					}
					else
					{
						memcpy(&m_stEquip[ack.bPos], pItem, sizeof(ItemAttrib));
						memset(pItem, 0, sizeof(ItemAttrib));
						ObjectValid::EncryptAttrib(pItem);
					}

					RefleshAttrib();
					OnPlayerDressdItem(req.dwTag);
				}
			}
		}
		else if(IsCostItem(GETITEMATB(pItem, Type)))
		{
			if(UseCostItem(pItem))
			{
				//memset(pItem, 0, sizeof(ItemAttrib));
			}
		}
		else if(GETITEMATB(pItem, Type) == ITEM_BALE)
		{
			if(UseBaleItem(pItem))
			{
				//memset(pItem, 0, sizeof(ItemAttrib));
			}
		}
		else if(GETITEMATB(pItem, Type) == ITEM_BOOK)
		{
			if(ReadBook(pItem))
			{
				//memset(pItem, 0, sizeof(ItemAttrib));
			}
		}
	}
}

void HeroObject::DoPacket(const PkgGameLoadedAck& ack)
{
	m_bClientLoaded = true;
	//	Then load the scene?
	GameScene* pScene = GameSceneManager::GetInstance()->GetScene(GetMapID());
	if(pScene)
	{
		pScene->GetSceneData(this);
		if(IsNewPlayer())
		{
			PkgPlayerShowHelpAck ack;
			ack.uTargetId = GetID();
			ack.nHelpID = 2;
			SendPacket(ack);
		}

		PkgPlayerQuickMsgNtf ppqmn;
		ppqmn.uTargetId = GetID();
		ppqmn.nParam = MAKELONG(GameWorld::GetInstance().GetDropMultiple(), GameWorld::GetInstance().GetExprMultiple());
		ppqmn.nMsgID = QUICKMSG_MULTI;
		SendPacket(ppqmn);
		ppqmn.nMsgID = QMSG_DIFFICULTYLEVEL;
		ppqmn.nParam = GameWorld::GetInstance().GetDifficultyLevel();
		SendPacket(ppqmn);
		
		PkgPlayerVersionVerifyNtf ppvvn;
		ppvvn.uTargetId = GetID();
		ppvvn.xVersion = BACKMIR_CURVERSION;
		SendPacket(ppvvn);

		//	Quest info
		PkgPlayerQuestDataNtf ppqdn;
		ppqdn.uTargetId = GetID();
#define MAX_DATA_SIZE 10240
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << m_xQuest;
		static char* s_pData = new char[MAX_DATA_SIZE];
		uLongf srcsize = g_xThreadBuffer.GetLength();
		uLongf cmpsize = MAX_DATA_SIZE;

		int nRet = compress((Bytef*)s_pData, &cmpsize, (const Bytef*)g_xThreadBuffer.GetBuffer(), srcsize);
		if(nRet == Z_OK)
		{
			//	OK
			ppqdn.xData.resize(cmpsize);
			/*for(int i = 0; i < cmpsize; ++i)
			{
				ppqdn.xData[i] = pData[i];
			}*/
			memcpy((char*)(&ppqdn.xData[0]), s_pData, cmpsize);
			SendPacket(ppqdn);
		}
		else
		{
			LOG(ERROR) << "Compress error!";
		}
		//SAFE_DELETE_ARRAY(pData);
		char szMsg[MAX_PATH] = {0};

		//	根据捐赠金额来发送系统公告
		if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN &&
			GetDonateMoney() != 0)
		{
			time_t currentTime = 0;
			time(&currentTime);
			bool bNotifyWorld = false;

			std::map<int, int>::iterator fndIter = g_xShowDonateTimeMap.find(GetUID());
			if(fndIter == g_xShowDonateTimeMap.end())
			{
				//	没有进行过通知
				g_xShowDonateTimeMap.insert(std::make_pair(GetUID(), currentTime));
				bNotifyWorld = true;
			}
			else
			{
				int nLastNotifyTime = fndIter->second;
#ifdef _DEBUG
#else
				if(currentTime - nLastNotifyTime > 30 * 60)
#endif
				{
					//	进行通知
					fndIter->second = currentTime;
					bNotifyWorld = true;
				}
			}

			if(bNotifyWorld)
			{
				char szName[20] = {0};
				ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);

				sprintf(szMsg, "尊敬的游戏捐助玩家[%s] LV[%d]进入游戏，感谢您对BackMIR的支持", szName, GetDonateLevel());
				GameSceneManager::GetInstance()->SendSystemMessageAllScene(szMsg);
			}
		}

		//	同步一些数据
		//	生成新的随机数种子
		m_xNormalAttackRand.SetSeed((long)time(NULL));
		SyncRandSeedNormalAtk();

		//	同步一些数据
		SyncDonateLeft();
		SendPlayerDataTo(this);

		//	礼花
		if(GameWorld::GetInstance().GetExpFireworkTime() != 0)
		{
			sprintf(szMsg, "玩家[%s]燃放了五彩礼花，您享受0.5倍经验加成", GameWorld::GetInstance().GetExpFireworkUserName().c_str());
			SendSystemMessage(szMsg);
		}
		if(GameWorld::GetInstance().GetBurstFireworkTime() != 0)
		{
			sprintf(szMsg, "玩家[%s]燃放了炫紫礼花，您享受0.5倍爆率加成", GameWorld::GetInstance().GetBurstFireworkUserName().c_str());
			SendSystemMessage(szMsg);
		}
		if(GameWorld::GetInstance().GetMagicDropFireworkTime() != 0)
		{
			sprintf(szMsg, "玩家[%s]燃放了魔法礼花，您享受魔法装备加成+8", GameWorld::GetInstance().GetMagicDropFireworkUserName().c_str());
			SendSystemMessage(szMsg);
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerSyncAssistReq& req)
{
	if(req.bNum >= 0 &&
		req.bNum <= HERO_ASSISTBAG_SIZE_CUR)
	{
		m_nAssistItemSum = HERO_ASSISTBAG_SIZE_CUR - req.bNum;
	}
}

void HeroObject::DoPacket(const PkgPlayerShopOpReq& req)
{
	if(req.bOp == SHOP_BUY)
	{
		//	check bag empty
		if(/*GetBagEmptySum() > 0 &&*/
			req.dwData < ITEM_ID_MAX)
		{
			GameObject* pObj = GetLocateScene()->GetNPCByHandleID(req.uTargetId);
			if(NULL != pObj)
			{
				if(pObj->GetType() == SOT_NPC)
				{
					NPCObject* pNPC = static_cast<NPCObject*>(pObj);
					if(pNPC->IsItemExist(req.dwData))
					{
						int nTotalPrice = g_nItemPrice[req.dwData];
						ItemAttrib item;

						if(GetRecordInItemTable(req.dwData, &item))
						{
							if(item.type == ITEM_COST)
							{
								if(req.bNumber == 0)
								{
									//
								}
								else
								{
									//nTotalPrice *= GRID_MAX;
									nTotalPrice *= req.bNumber;
								}
								

								if(GetMoney() >= nTotalPrice)
								{
									/*if(req.bNumber == 0)
									{
										if(AddItem(req.dwData))
										{
											int nLeftMoney = GetMoney() - nTotalPrice;
											SetMoney(nLeftMoney);

											PkgPlayerUpdateAttribNtf uantf;
											uantf.uTargetId = GetID();
											//uantf.dwParam = MAKELONG(UPDATE_MONEY, 0);
											uantf.bType = UPDATE_MONEY;
											uantf.dwParam = GetMoney();
											g_xThreadBuffer.Reset();
											g_xThreadBuffer << uantf;
											SendPlayerBuffer(g_xThreadBuffer);
										}
									}
									else*/
									{
										/*for(int i = 0; i < GRID_MAX; ++i)
										{
											AddItem(req.dwData);
										}*/
										AddBatchItem(req.dwData, req.bNumber);

										int nLeftMoney = GetMoney() - nTotalPrice;
										SetMoney(nLeftMoney);

										PkgPlayerUpdateAttribNtf uantf;
										uantf.uTargetId = GetID();
										//uantf.dwParam = MAKELONG(UPDATE_MONEY, 0);
										uantf.bType = UPDATE_MONEY;
										uantf.dwParam = GetMoney();
										g_xThreadBuffer.Reset();
										g_xThreadBuffer << uantf;
										SendPlayerBuffer(g_xThreadBuffer);
									}
								}
							}
							else
							{
								if(GetMoney() >= g_nItemPrice[req.dwData])
								{
									if(AddItem(req.dwData))
									{
										int nLeftMoney = GetMoney() - g_nItemPrice[req.dwData];
										SetMoney(nLeftMoney);

										PkgPlayerUpdateAttribNtf uantf;
										uantf.uTargetId = GetID();
										//uantf.dwParam = MAKELONG(UPDATE_MONEY, 0);
										uantf.bType = UPDATE_MONEY;
										uantf.dwParam = GetMoney();
										g_xThreadBuffer.Reset();
										g_xThreadBuffer << uantf;
										SendPlayerBuffer(g_xThreadBuffer);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if(req.bOp == SHOP_SELL)
	{
		if(req.dwData == 0)
		{
			return;
		}

		ItemAttrib* pItem = GetItemByTag(req.dwData);
		if(pItem)
		{
			if(TEST_FLAG_BOOL(GETITEMATB(pItem, Expr), EXPR_MASK_NOSELL))
			{
				//SendSystemMessage("任务物品无法卖出");
				SendQuickMessage(QMSG_CANNOTSELLQUEST);
			}
			else
			{
				if(GETITEMATB(pItem, Type) == ITEM_COST)
				{
					int nItemID = GETITEMATB(pItem, ID);
					if(nItemID < 0 ||
						nItemID >= sizeof(g_nItemPrice) / sizeof(g_nItemPrice[0]))
					{
						return;
					}
					AddMoney(g_nItemPrice[GETITEMATB(pItem, ID)] / SELL_ITEM_MULTI * GETITEMATB(pItem, AtkSpeed));
				}
				else
				{
					int nItemID = GETITEMATB(pItem, ID);
					if(nItemID < 0 ||
						nItemID >= sizeof(g_nItemPrice) / sizeof(g_nItemPrice[0]))
					{
						return;
					}

					int nSellMoney = g_nItemPrice[GETITEMATB(pItem, ID)] / SELL_ITEM_MULTI;
					if(HeroObject::IsEquipItem(GETITEMATB(pItem, Type)))
					{
						WORD wLevel = GETITEMATB(pItem, Level);

						if(wLevel != 0)
						{
							BYTE bLow = LOBYTE(wLevel);
							BYTE bHigh = HIBYTE(wLevel);

							bHigh &= 0x7A;
							bool bZero = false;

							if((bHigh & 0x40) == 0)
							{
								bZero = ((bHigh & 0x10) != 0 ? false : true);
								if(bZero)
								{
									bHigh |= 0x10;
								}
								else
								{
									bHigh &= (~0x10);
								}
							}

							if((bHigh & 0x02) == 0)
							{
								bZero = ((bHigh & 0x08) != 0 ? false : true);
								if(bZero)
								{
									bHigh |= 0x08;
								}
								else
								{
									bHigh &= (~0x08);
								}
							}

							static int s_nValueTable[] =
							{
								9, 2, 1, 3, 4, 7, 8, 5
							};
							static BYTE s_btMaskTable[] =
							{
								0x80, 0x40, 0x20, 0x10,
								0x08, 0x04, 0x02, 0x01
							};

							int nValue = 0;
							for(int i = 0; i < 8; ++i)
							{
								if((bHigh & s_btMaskTable[i]) != 0)
								{
									nValue += s_nValueTable[i];
								}
							}

							nValue = (int)bLow - nValue;
							if(nValue > 0 &&
								nValue <= 8)
							{
								//	recalc price
								nSellMoney *= (float)(1.0f + (float)((float)nValue / 4));
							}
						}
					}
					AddMoney(nSellMoney);
				}
				
				PkgPlayerUpdateAttribNtf uantf;
				uantf.uTargetId = GetID();
				//uantf.dwParam = MAKELONG(UPDATE_MONEY, 0);
				uantf.bType = UPDATE_MONEY;
				uantf.dwParam = GetMoney();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << uantf;
				SendPlayerBuffer(g_xThreadBuffer);

				PkgPlayerClearBagNtf cbntf;
				cbntf.uTargetId = GetID();
				cbntf.dwTag = req.dwData;
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << cbntf;
				SendPlayerBuffer(g_xThreadBuffer);
				ZeroMemory(pItem, sizeof(ItemAttrib));
				ObjectValid::EncryptAttrib(pItem);
			}
		}
	}
	else if(req.bOp == SHOP_STORE)
	{
		ItemAttrib* pItem = GetItemByTag(req.dwData);
		if(pItem)
		{
			if(AddStoreItem(pItem))
			{
				/*PkgPlayerClearBagNtf cbntf;
				cbntf.uTargetId = GetID();
				cbntf.dwTag = req.dwData;
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << cbntf;
				SendPlayerBuffer(g_xThreadBuffer);*/
				PkgPlayerStoreSyncAck ack;
				ack.bType = STORESYNC_ADDSTORE;
				ack.dwTag = pItem->tag;
				ack.uTargetId = GetID();
				SendPacket(ack);

				ZeroMemory(pItem, sizeof(ItemAttrib));
				ObjectValid::EncryptAttrib(pItem);
			}
		}
	}
	else if(req.bOp == SHOP_BIGSTORE_STORE)
	{
		ItemAttrib* pItem = GetItemByTag(req.dwData);
		if(pItem)
		{
			if(AddBigStoreItem(pItem))
			{
				/*PkgPlayerClearBagNtf cbntf;
				cbntf.uTargetId = GetID();
				cbntf.dwTag = req.dwData;
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << cbntf;
				SendPlayerBuffer(g_xThreadBuffer);*/
				PkgPlayerStoreSyncAck ack;
				ack.bType = STORESYNC_BIG_ADDSTORE;
				ack.dwTag = pItem->tag;
				ack.uTargetId = GetID();
				SendPacket(ack);

				ZeroMemory(pItem, sizeof(ItemAttrib));
				ObjectValid::EncryptAttrib(pItem);
			}
		}
	}
	else if(req.bOp == SHOP_FETCH)
	{
		ItemAttrib* pItem = GetStoreItem(req.dwData);
		if(pItem)
		{
			int nEmptyBag = GetBagEmptySum();
			if(nEmptyBag > 0)
			{
				int nIdx = -1;
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
						m_xBag[i].tag != ITEMTAG_INQUERY)
					{
						nIdx = i;
						break;
					}
				}

				if(nIdx != -1)
				{
					ItemAttrib* pBagItem = &m_xBag[nIdx];
					memcpy(pBagItem, pItem, sizeof(ItemAttrib));
					//SETITEMATB(pItem, Type, ITEM_NO);
					//pItem->tag = 0;
					ZeroMemory(pItem, sizeof(ItemAttrib));
					ObjectValid::EncryptAttrib(pItem);

					//	Now ok
					/*PkgPlayerGainItemNtf ntf;
					ntf.stItem = *pBagItem;
					ntf.uTargetId = GetID();
					SendPacket(ntf);
*/

					//	Clear Store Dialog
					PkgPlayerStoreSyncAck ack;
					ack.bType = STORESYNC_REMOVESTORE;
					ack.dwTag = pBagItem->tag;
					ack.uTargetId = GetID();
					SendPacket(ack);
				}
			}
		}
	}
	else if(req.bOp == SHOP_BIGSTORE_FETCH)
	{
		ItemAttrib* pItem = GetBigStoreItem(req.dwData);
		if(pItem)
		{
			int nEmptyBag = GetBagEmptySum();
			if(nEmptyBag > 0)
			{
				int nIdx = -1;
				for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
				{
					if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
						m_xBag[i].tag != ITEMTAG_INQUERY)
					{
						nIdx = i;
						break;
					}
				}

				if(nIdx != -1)
				{
					ItemAttrib* pBagItem = &m_xBag[nIdx];
					memcpy(pBagItem, pItem, sizeof(ItemAttrib));
					//SETITEMATB(pItem, Type, ITEM_NO);
					//pItem->tag = 0;
					ZeroMemory(pItem, sizeof(ItemAttrib));
					ObjectValid::EncryptAttrib(pItem);

					//	Now ok
					/*PkgPlayerGainItemNtf ntf;
					ntf.stItem = *pBagItem;
					ntf.uTargetId = GetID();
					SendPacket(ntf);
*/

					//	Clear Store Dialog
					PkgPlayerStoreSyncAck ack;
					ack.bType = STORESYNC_BIG_REMOVESTORE;
					ack.dwTag = pBagItem->tag;
					ack.uTargetId = GetID();
					SendPacket(ack);
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::DoPacket(const PkgPlayerUserDataReq& req)
{
	SendHumDataV2(true, CMainServer::GetInstance()->GetServerMode() == GM_LOGIN ? true : false);
}

void HeroObject::DoPacket(const PkgPlayerReviveReq& req)
{
	if(m_stData.eGameState == OS_DEAD)
	{
		PkgPlayerReviveNot not;
		PkgPlayerUpdateAttribNtf ntf;
		ntf.uTargetId = GetID();
		not.uTargetId = GetID();

		if(req.bMode == 1)
		{
			GameScene* pHome = GetHomeScene();
			if(pHome)
			{
				SetObject_HP(GetObject_MaxHP() / 2);
				m_pValid->SetHP(GetObject_HP());
				m_stData.eGameState = OS_STAND;
				if(GetLocateScene()->GetMapPkType() != kMapPkType_All)
				{
					SetObject_Expr(GetObject_Expr() * 0.9995f);
				}

				not.uHp = GetObject_HP();
				not.bMode = 1;
				SendPacket(not);

				ntf.bType = UPDATE_EXP;
				ntf.dwParam = GetObject_Expr();
				SendPacket(ntf);

				if(pHome->GetMapID() != GetLocateScene()->GetMapID())
				{
					FlyToMap(pHome->GetCityCenterX(), pHome->GetCityCenterY(), pHome->GetMapID());
				}
				else
				{
					FlyTo(pHome->GetCityCenterX(), pHome->GetCityCenterY());
				}
			}
		}
		else if(req.bMode == 2)
		{
			if(GetTickCount() - m_dwLastReviveTime > INTERVAL_REVIVE)
			{
				SetObject_HP(GetObject_MaxHP() / 2);
				m_pValid->SetHP(GetObject_HP());
				m_stData.eGameState = OS_STAND;
				SetObject_Expr(GetObject_Expr() / 2);

				not.uHp = GetObject_HP();
				not.bMode = 2;
				SendPacket(not);

				ntf.bType = UPDATE_EXP;
				ntf.dwParam = GetObject_Expr();
				SendPacket(ntf);

				int nNowMoney = GetMoney() * 0.9f;
				SetMoney(nNowMoney);
				ntf.bType = UPDATE_MONEY;
				ntf.dwParam = GetMoney();
				SendPacket(ntf);

				m_dwLastReviveTime = GetTickCount();
			}
			else
			{
				PkgPlayerShowDlgAck ack;
				ack.bType = DLG_REVIVE;
				ack.uTargetId = GetID();
				//SendPacket(ack);
				//SendSystemMessage("原地复活间隔为 15 分钟");
				SendQuickMessage(QMSG_REVIVEINTERVAL, 15);
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerMonsInfoReq& req)
{
	GameScene* pScene = GetLocateScene();
	if(pScene)
	{
		GameObject* pObj = pScene->GetNPCByHandleID(req.uTargetId);
		if(pObj)
		{
			PkgNewNPCNot stNpcNot;
			stNpcNot.uHandlerID = pObj->GetID();
			stNpcNot.uMonsID = pObj->GetObject_ID();
			stNpcNot.uPosition = MAKELONG(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY);
			stNpcNot.uParam = pObj->GetObject_HP();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << stNpcNot;
			SendPlayerBuffer(g_xThreadBuffer);
		}
		else
		{
			pObj = pScene->GetPlayerWithoutLock(req.uTargetId);
			if(pObj)
			{
				PkgNewPlayerNot not;
				not.bNew = true;
				HeroObject* pHero = (HeroObject*)pObj;
				not.dwLook1 = MAKELONG(GETITEMATB(pHero->GetEquip(PLAYER_ITEM_CLOTH), ID), GETITEMATB(pHero->GetEquip(PLAYER_ITEM_WEAPON), ID));
				memcpy(&not.stData, pObj->GetUserData(), sizeof(UserData));
				int nSkillLevel = pHero->GetMagicLevel(MEFF_SHIELD);
				if(0 != nSkillLevel)
				{
					not.xSkillInfo.push_back(MEFF_SHIELD);
					not.xSkillInfo.push_back(nSkillLevel);
				}
				not.uHandlerID = pObj->GetID();
				g_xThreadBuffer << not;
				SendPlayerBuffer(g_xThreadBuffer);
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerCubeItemsReq& req)
{
	int nItemSum = req.xTags.size();
	if(nItemSum > 0 &&
		nItemSum <= 8)
	{
		//ItemAttrib* pItems[8] = {NULL};
		std::list<const ItemAttrib*> xList;
		std::list<int>::const_iterator begiter = req.xTags.begin();
		for(begiter;
			begiter != req.xTags.end();
			++begiter)
		{
			const ItemAttrib* pItem = GetItemByTag(*begiter);
			if(NULL == pItem)
			{
				return;
			}
			else
			{
				xList.push_back(pItem);
			}
		}
		if(xList.empty())
		{
			return;
		}
		//std::sort(xList.begin(), xList.end(), SortAttribID);
		xList.sort(SortAttribID);

		//	预先处理混沌石
		/*if(xList.size() == 3 &&
			nHunDunStoneTag != 0 &&
			nHunDunStoneCount == 1)
		{
			//	有混沌石 检查剩余装备是否符合要求
			int nEquipId = 0;
			int nEquipCount = 0;
			int nEquipTags[2] = {0};

			std::list<const ItemAttrib*>::const_iterator tagbegiter = xList.begin();
			for(tagbegiter;
				tagbegiter != xList.end();
				++tagbegiter)
			{
				ItemAttrib* pItem = *tagbegiter;
				int nCheckItemId = GETITEMATB(pItem, ID);
				
				if(nCheckItemId != nHunDunStoneId)
				{
					if(0 == nEquipId)
					{
						nEquipId = nCheckItemId;
						nEquipTags[nEquipCount++] = pItem->tag;
					}
					else if(nEquipId == nCheckItemId)
					{
						
					}
					else
					{
						SendQuickMessage(QMSG_INVALIDHUNDUNITEMS);
						return;
					}
				}
			}
		}*/

		lua_State* pLuaState = GameWorld::GetInstance().GetLuaState();
		lua_getglobal(pLuaState, "CubeItems");
		tolua_pushusertype(pLuaState, this, "HeroObject");
		lua_pushinteger(pLuaState, nItemSum);
		std::list<const ItemAttrib*>::const_iterator tagbegiter = xList.begin();
		for(tagbegiter;
			tagbegiter != xList.end();
			++tagbegiter)
		{
			lua_pushinteger(pLuaState, (*tagbegiter)->tag);
		}
		if(nItemSum < 8)
		{
			nItemSum = 8 - nItemSum;
			for(int i = 0; i < nItemSum; ++i)
			{
				lua_pushinteger(pLuaState, 0);
			}
		}
		int nRet = lua_pcall(pLuaState, 10, 0, 0);
		if(nRet != 0)
		{
			LOG(ERROR) << lua_tostring(pLuaState, -1);
			lua_pop(pLuaState, 1);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void HeroObject::DoPacket(const PkgPlayerClickNPCReq& req)
{
	GameScene* pScene = GetLocateScene();
	if(!pScene)
	{
		return;
	}
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << req;

	GameObject* pNPC = pScene->GetNPCByHandleID(req.uTargetId);
	if(pNPC)
	{
		if (GameWorld::GetInstance().GetThreadRunMode()) {
			if(!pNPC->AddProcess(g_xThreadBuffer))
			{
				// !!!
				char szName[20] = {0};
				ObjectValid::GetItemName(&pNPC->GetUserData()->stAttrib, szName);
				LOG(ERROR) << szName << " fatal error operation!!!ClickNPCReq";
				LOG(ERROR) << "Content:" << g_xThreadBuffer.ToHexString();
				CMainServer::GetInstance()->GetEngine()->CompulsiveDisconnectUser(GetUserIndex());
			}
		} else {
			pNPC->DispatchPacket(g_xThreadBuffer);
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerAttackReq& req)
{
	return;
	GameScene* pScene = GetLocateScene();
	if(!pScene)
	{
		LOG(ERROR) << "In no scene!";
		return;
	}

	g_xThreadBuffer.Reset();
	g_xThreadBuffer << req;

	MonsterObject* pMons = (MonsterObject*)pScene->GetNPCByHandleID(req.uTargetId);
	if(pMons)
	{
		pMons->AddAttackProcess(g_xThreadBuffer);
	}
}

void HeroObject::DoPacket(const PkgPlayerSlaveStopReq& req)
{
	OBJECT_STATE os = OS_STOP;
	bool bStop = false;
	bool bCast = false;
	bool bFirst = true;

	for(int i = 0; i < MAX_SLAVE_SUM; ++i)
	{
		if(m_pSlaves[i])
		{
			bCast = true;
			if(bFirst)
			{
				os = m_pSlaves[i]->GetUserData()->eGameState;
				if(os == OS_STOP)
				{
					m_pSlaves[i]->GetUserData()->eGameState = OS_STAND;
					os = OS_STAND;
				}
				else
				{
					bStop = true;
					m_pSlaves[i]->GetUserData()->eGameState = OS_STOP;
					os = OS_STOP;
				}
				bFirst = false;
			}
			else
			{
				m_pSlaves[i]->GetUserData()->eGameState = os;
			}

			m_pSlaves[i]->SetLastAttackTime(GetTickCount());
		}
	}

	if(bCast)
	{
		if(bStop)
		{
			SendSystemMessage("下属 休息");
		}
		else
		{
			SendSystemMessage("下属 攻击");
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerCallSlaveReq& req)
{
	int nCurX = GetUserData()->wCoordX;
	int nCurY = GetUserData()->wCoordY;
	int nCallX = 0;
	int nCallY = 0;
	MonsterObject* pMonster = NULL;

	for(int i = 0; i < MAX_SLAVE_SUM; ++i)
	{
		if(m_pSlaves[i])
		{
			if(m_pSlaves[i]->GetType() == SOT_MONSTER)
			{
				pMonster = static_cast<MonsterObject*>(m_pSlaves[i]);
				if(pMonster->GetLocateScene()->GetMapID() == GetMapID())
				{
					for(int i = 0; i < 8; ++i)
					{
						nCallX = nCurX + g_nMoveOft[i * 2];
						nCallY = nCurY + g_nMoveOft[i * 2 + 1];

						if(GetLocateScene()->CanThrough(nCallX, nCallY))
						{
							pMonster->SetTarget(NULL);
							pMonster->FlyTo(nCallX, nCallY);
							break;
						}
					}
				}
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerKillSlaveReq& req)
{
	KillAllSlave();
}

void HeroObject::DoPacket(const PkgPlayerMergyCostItemReq& req)
{
	ItemAttrib* pDestItem = GetItemByTag(req.dwDestTag);
	ItemAttrib* pSrcItem = GetItemByTag(req.dwSrcTag);

	if(NULL == pDestItem ||
		NULL == pSrcItem)
	{
		return;
	}

	ItemAttrib item = *pDestItem;
	ObjectValid::DecryptAttrib(&item);
	item = *pSrcItem;
	ObjectValid::DecryptAttrib(&item);

	if(NULL != pDestItem &&
		NULL != pSrcItem)
	{
		if(GETITEMATB(pDestItem, Type) == ITEM_COST &&
			GETITEMATB(pSrcItem, Type) == ITEM_COST &&
			GETITEMATB(pDestItem, ID) == GETITEMATB(pSrcItem, ID))
		{
			if(!TEST_FLAG_BOOL(GETITEMATB(pDestItem, AtkPois), POIS_MASK_BIND) &&
				!TEST_FLAG_BOOL(GETITEMATB(pSrcItem, AtkPois), POIS_MASK_BIND))
			{
				int nMergySum = GETITEMATB(pDestItem, AtkSpeed) + GETITEMATB(pSrcItem, AtkSpeed);
				if(nMergySum <= GRID_MAX)
				{
					SETITEMATB(pDestItem, AtkSpeed, nMergySum);

					PkgPlayerUpdateCostNtf ppucn;
					ppucn.dwTag = pDestItem->tag;
					ppucn.nNumber = nMergySum;
					ppucn.uTargetId = GetID();
					SendPacket(ppucn);

					PkgPlayerLostItemAck pplia;
					pplia.uTargetId = GetID();
					pplia.dwTag = pSrcItem->tag;
					SendPacket(pplia);

					ZeroMemory(pSrcItem, sizeof(ItemAttrib));
					ObjectValid::EncryptAttrib(pSrcItem);
				}
				else
				{
					if(GETITEMATB(pDestItem, AtkSpeed) != GRID_MAX &&
						GETITEMATB(pSrcItem, AtkSpeed) != GRID_MAX)
					{
						SETITEMATB(pDestItem, AtkSpeed, GRID_MAX);
						PkgPlayerUpdateCostNtf ppucn;
						ppucn.dwTag = pDestItem->tag;
						ppucn.nNumber = nMergySum;
						ppucn.uTargetId = GetID();
						SendPacket(ppucn);

						SETITEMATB(pSrcItem, AtkSpeed, nMergySum - GRID_MAX);
						ppucn.dwTag = pSrcItem->tag;
						ppucn.nNumber = nMergySum - GRID_MAX;
						ppucn.uTargetId = GetID();
						SendPacket(ppucn);
					}
				}
			}
			else
			{
				int nMergySum = GETITEMATB(pDestItem, AtkSpeed) + GETITEMATB(pSrcItem, AtkSpeed);
				if(nMergySum <= GRID_MAX)
				{
					SETITEMATB(pDestItem, AtkSpeed, nMergySum);

					int nValue = GETITEMATB(pDestItem, AtkPois);
					SET_FLAG(nValue, POIS_MASK_BIND);
					SETITEMATB(pDestItem, AtkPois, nValue);

					PkgPlayerUpdateCostNtf ppucn;
					ppucn.dwTag = pDestItem->tag;
					ppucn.nNumber = nMergySum;
					ppucn.uTargetId = GetID();
					ppucn.bBind = true;
					SendPacket(ppucn);

					PkgPlayerLostItemAck pplia;
					pplia.uTargetId = GetID();
					pplia.dwTag = pSrcItem->tag;
					SendPacket(pplia);

					ZeroMemory(pSrcItem, sizeof(ItemAttrib));
					ObjectValid::EncryptAttrib(pSrcItem);
				}
				else
				{
					if(GETITEMATB(pDestItem, AtkSpeed) != GRID_MAX &&
						GETITEMATB(pSrcItem, AtkSpeed) != GRID_MAX)
					{
						int nValue = GETITEMATB(pDestItem, AtkPois);
						SET_FLAG(nValue, POIS_MASK_BIND);
						SETITEMATB(pDestItem, AtkPois, nValue);
						SETITEMATB(pSrcItem, AtkPois, nValue);

						SETITEMATB(pDestItem, AtkSpeed, GRID_MAX);
						PkgPlayerUpdateCostNtf ppucn;
						ppucn.dwTag = pDestItem->tag;
						ppucn.nNumber = GRID_MAX;
						ppucn.uTargetId = GetID();
						ppucn.bBind = true;
						SendPacket(ppucn);

						SETITEMATB(pSrcItem, AtkSpeed, nMergySum - GRID_MAX);
						ppucn.dwTag = pSrcItem->tag;
						ppucn.nNumber = nMergySum - GRID_MAX;
						ppucn.uTargetId = GetID();
						ppucn.bBind = true;
						SendPacket(ppucn);
					}
				}
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerSpeOperateReq& req)
{
	ItemAttrib* pItem = NULL;
	bool bCanUse = false;

	if(req.dwOp == CMD_OP_MOVE)
	{
		/*pItem = GetEquip(PLAYER_ITEM_RING1);
		if(GETITEMATB(pItem, ID) == 172)
		{
			bCanUse = true;
		}
		if(!bCanUse)
		{
			pItem = GetEquip(PLAYER_ITEM_RING2);
			if(GETITEMATB(pItem, ID) == 172)
			{
				bCanUse = true;
			}
		}*/

		bCanUse = false;

		if(m_xStates.GetMovePosAddition() > 0)
		{
			bCanUse = true;
		}

		if(bCanUse)
		{
			//if(GetLocateScene()->IsAutoReset())
			if(!GetLocateScene()->CanUseMove())
			{
				//SendSystemMessage("该地图无法传送");
				SendQuickMessage(QMSG_CANNOTTRANSFER);
				return;
			}
			if(GetTickCount() - m_dwLastUseMoveRingTime < 15 * 1000)
			{
				//SendSystemMessage("间隔:8秒");
				SendQuickMessage(QMSG_TRANSFERINTERVAL, 15);
			}
			else
			{
				int nX = 0;
				int nY = 0;
				nX = LOWORD(req.dwParam);
				nY = HIWORD(req.dwParam);
				if(nX > 0 &&
					nX < 999 &&
					nY > 0 &&
					nY < 999)
				{
					if(GetLocateScene()->CanThrough(nX, nY))
					{
						FlyTo(nX, nY);
						m_dwLastUseMoveRingTime = GetTickCount();
					}
					else
					{
						SendSystemMessage("无法到达目的地");
					}
				}
			}
		}
		else
		{
			SendSystemMessage("非法指令");
		}
	}
	else if(req.dwOp == CMD_OP_GMOVE)
	{
		if(IsGmHide())
		{
			int nX = 0;
			int nY = 0;
			nX = LOWORD(req.dwParam);
			nY = HIWORD(req.dwParam);
			if(nX > 0 &&
				nX < 999 &&
				nY > 0 &&
				nY < 999)
			{
				if(GetLocateScene()->CanThrough(nX, nY))
				{
					FlyTo(nX, nY);
					//m_dwLastUseMoveRingTime = GetTickCount();
				}
				else
				{
					SendSystemMessage("无法到达目的地");
				}
			}
		}
	}
	else if(req.dwOp == CMD_OP_FLY)
	{
//#ifdef _DEBUG
		if(IsGmHide())
		{
			DWORD dwPos = 0;
			GameScene* pNextScene = GameSceneManager::GetInstance()->GetScene(req.dwParam);
			if(NULL != pNextScene)
			{
				if(pNextScene->GetRandomPosition(&dwPos))
				{
					FlyToMap(LOWORD(dwPos), HIWORD(dwPos), req.dwParam);
				}
			}
		}
//#endif
	}
	else if(req.dwOp == CMD_OP_MAKE)
	{
//#ifdef _DEBUG
		if(IsGmHide())
		{
			AddItem(req.dwParam);
		}
//#endif
	}
	else if (req.dwOp == CMD_OP_MAKESUPERITEM)
	{
		if (IsGmHide())
		{
			AddSuperItem(LOWORD(req.dwParam), HIWORD(req.dwParam));
		}
	}
	else if(req.dwOp == CMD_OP_GIVE)
	{
		if(IsGmHide())
		{
			WORD wPlayerID = LOWORD(req.dwParam);
			WORD wItemID = HIWORD(req.dwParam);

			HeroObject* pPlayer = static_cast<HeroObject*>(GetLocateScene()->GetPlayer(wPlayerID));
			if(pPlayer)
			{
				
				pPlayer->AddItem(wItemID);
			}
		}
	}
	else if(req.dwOp == CMD_OP_LEVELUP)
	{
//#ifdef _DEBUG
		if(IsGmHide())
		{
			int nUpLevel = req.dwParam;
			if(nUpLevel > MAX_LEVEL)
			{
				nUpLevel = MAX_LEVEL;
			}

			SetObject_Level(nUpLevel);
			PkgPlayerUpdateAttribNtf ntf;
			ntf.bType = UPDATE_LEVEL;
			ntf.uTargetId = GetID();
			ntf.dwParam = GetObject_Level();
			SendPacket(ntf);
		}
//#endif
	}
	else if(req.dwOp == CMD_OP_MONSTER)
	{
//#ifdef _DEBUG
		if(IsGmHide())
		{
			int nMonsNum = HIWORD(req.dwParam);
			int nMonsID = LOWORD(req.dwParam);
			if(nMonsNum == 0)	{nMonsNum = 1;}

			for(int i = 0; i < nMonsNum; ++i)
			{
				GetLocateScene()->CreateMonster(nMonsID, GetUserData()->wCoordX, GetUserData()->wCoordY);	
			}
		}
		//#endif
	}
	else if(req.dwOp == CMD_OP_EMONSTER)
	{
		if(IsGmHide())
		{
			GetLocateScene()->CreateEliteMonster(req.dwParam, GetUserData()->wCoordX, GetUserData()->wCoordY);
		}
	}
	else if(req.dwOp == CMD_OP_LMONSTER)
	{
		if(IsGmHide())
		{
			GetLocateScene()->CreateLeaderMonster(req.dwParam, GetUserData()->wCoordX, GetUserData()->wCoordY);
		}
	}
	else if(req.dwOp == CMD_OP_GET)
	{
//#ifdef _DEBUG
		if(IsGmHide())
		{
			int nItemId = LOWORD(req.dwParam);
			int nCount = HIWORD(req.dwParam);
			if(0 == nCount)
			{
				nCount = 1;
			}
			for(int i = 0; i < nCount; ++i)
			{
				AddItem_GM(nItemId);
			}
		}
//#endif
	}
	else if(req.dwOp == CMD_OP_EXPRMULTI)
	{
		if(IsGmHide())
		{
			if(req.dwParam <= 10 &&
				req.dwParam >= 0)
			{
				GameWorld::GetInstance().SetExprMultiple(req.dwParam);
			}
		}
	}
	else if(req.dwOp == CMD_OP_DROPMULTI)
	{
		if(IsGmHide())
		{
			if(req.dwParam <= 10 &&
				req.dwParam >= 0)
			{
				GameWorld::GetInstance().SetDropMultiple(req.dwParam);
			}
		}
	}
	else if(req.dwOp == CMD_OP_NORESET)
	{
		if(IsGmHide())
		{
			if(req.dwParam == 0)
			{
				GameWorld::GetInstance().EnableAutoReset();
			}
			else
			{
				GameWorld::GetInstance().DisableAutoReset();
			}
		}
	}
	else if(req.dwOp == CMD_OP_ADDOLSHOPITEM)
	{
		if(IsGmHide())
		{
			if(req.dwParam != 0)
			{
				OlShopManager::GetInstance()->AddShopItem(req.dwParam);
			}
		}
	}
	else if(req.dwOp == CMD_OP_DELOLSHOPITEM)
	{
		if(IsGmHide())
		{
			if(req.dwParam != 0)
			{
				OlShopManager::GetInstance()->RemoveShopItem(req.dwParam);
			}
		}
	}
	else if(req.dwOp == CMD_OP_SETQUEST)
	{
		if(IsGmHide())
		{
			int nQuestId = LOWORD(req.dwParam);
			int nQuestStep = HIWORD(req.dwParam);

			m_xQuest.SetQuestStep(nQuestId, nQuestStep);
			SyncQuestData(nQuestId);
		}
	}
	else if(req.dwOp == CMD_OP_SCHEDULEACTIVE)
	{
		if(IsGmHide())
		{
			LuaEvent_WorldScheduleActive lw;
			lw.nScheduleId = req.dwParam;
			GameWorld::GetInstance().GetLuaEngine()->DispatchEvent(kLuaEvent_WorldScheduleActive, &lw);
		}
	}
	else if(req.dwOp == CMD_OP_GMHIDE)
	{
		//	Nothing
		if(req.dwParam != 55837413)
		{
			m_bGmHide = false;
			return;
		}
		m_bGmHide = true;

		GetLocateScene()->EraseTarget(this);
	}
	else if(req.dwOp == CMD_OP_CREATETEAM)
	{
		if(GetTeamID() == 0)
		{
			//	No team
			GameTeam* pTeam = GameTeamManager::GetInstance()->CreateTeam();
			if(pTeam)
			{
				pTeam->Insert(this);
				SendSystemMessage("队伍已建立");
			}
		}
	}
	else if(req.dwOp == CMD_OP_JOINTEAM)
	{
		if(GetTeamID() == 0)
		{
			GameTeam* pTeam = GameTeamManager::GetInstance()->GetTeam(GetTeamID());

			if(pTeam)
			{
				if(!pTeam->IsFull())
				{

				}
			}
		}
	}
	else if(req.dwOp == CMD_OP_ADDTEAMMATE)
	{
		if(GetTeamID() != 0)
		{
			GameTeam* pTeam = GameTeamManager::GetInstance()->GetTeam(GetTeamID());

			if(pTeam)
			{
				if(!pTeam->IsFull())
				{
					if(pTeam->GetTeamLeader() == this)
					{
						HeroObject* pOtherPlayer = static_cast<HeroObject*>(GetLocateScene()->GetPlayerWithoutLock(req.dwParam));
						if(NULL != pOtherPlayer)
						{
							if(!pTeam->IsInTeam(pOtherPlayer) &&
								pOtherPlayer->GetTeamID() == 0)
							{
								if(pTeam->Insert(pOtherPlayer))
								{
									std::string xMsg = "您已加入 ";
									char szNameBuf[20];
									ObjectValid::GetItemName(&GetUserData()->stAttrib, szNameBuf);
									xMsg += szNameBuf;
									xMsg += " 的队伍";
									pOtherPlayer->SendSystemMessage(xMsg.c_str());

									xMsg.clear();
									ObjectValid::GetItemName(&pOtherPlayer->GetUserData()->stAttrib, szNameBuf);
									xMsg = szNameBuf;
									xMsg += " 已加入队伍";
									pTeam->SendTeamMessage(xMsg.c_str());
								}
							}
							else
							{
								SendSystemMessage("已在队伍中");
							}
						}
						else
						{
							SendSystemMessage("对方不存在或不在同一地图");
						}
					}
					else
					{
						SendSystemMessage("您不是队长");
					}
				}
				else
				{
					SendSystemMessage("队伍已满");
				}
			}
		}
		else
		{
			SendSystemMessage("您还没有创建队伍,输入指令@createteam创建队伍");
		}
	}
	else if(req.dwOp == CMD_OP_KICKTEAMMATE)
	{
		if(GetTeamID() != 0)
		{
			GameTeam* pTeam = GameTeamManager::GetInstance()->GetTeam(GetTeamID());

			if(pTeam->GetTeamLeader() == this)
			{
				HeroObject* pHero = static_cast<HeroObject*>(GetLocateScene()->GetPlayerWithoutLock(req.dwParam));

				if(pHero)
				{
					if(pTeam->IsInTeam(pHero))
					{
						pTeam->Remove(pHero);
						pHero->SendSystemMessage("您已被踢出队伍");

						std::string xMsg;
						char szNameBuf[20];
						ObjectValid::GetItemName(&pHero->GetUserData()->stAttrib, szNameBuf);
						xMsg = szNameBuf;
						xMsg += " 已被踢出队伍";

						pTeam->SendTeamMessage(xMsg.c_str());

						if(pTeam->GetCount() == 0)
						{
							GameTeamManager::GetInstance()->RemoveTeam(pTeam->GetTeamID());
						}
					}
				}
				else
				{
					SendSystemMessage("对方不存在或不在同一地图");
				}
			}
		}
		else
		{
			SendSystemMessage("您还没有加入队伍");
		}
	}
	else if(req.dwOp == CMD_OP_QUITTEAM)
	{
		if(GetTeamID() != 0)
		{
			GameTeam* pTeam = GameTeamManager::GetInstance()->GetTeam(GetTeamID());

			if(pTeam)
			{
				if(pTeam->IsInTeam(this))
				{
					pTeam->Remove(this);
					SendSystemMessage("您已退出队伍");

					char szName[20];
					ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
					std::string xMsg = szName;
					xMsg += "已退出队伍";
					pTeam->SendTeamMessage(xMsg.c_str());

					if(pTeam->GetCount() == 0)
					{
						GameTeamManager::GetInstance()->RemoveTeam(pTeam->GetTeamID());
					}
				}
			}
		}
	}
	else if(req.dwOp == CMD_OP_VIEWEQUIP)
	{
		HeroObject* pHero = static_cast<HeroObject*>(GetLocateScene()->GetPlayerWithoutLock(req.dwParam));
		if(pHero &&
			pHero != this)
		{
			//	View equipments
			PkgPlayerOtherPlayerEquipNtf ppopen;
			ppopen.uTargetId = GetID();
#define MAX_VIEWEQUIPDATA_SIZE 1024
			char szName[20];
			ObjectValid::GetItemName(&pHero->GetUserData()->stAttrib, szName);
			std::string xName = szName;

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << xName;

			BYTE bSex = pHero->GetHeroSex();
			g_xThreadBuffer << bSex;

			ItemAttrib item;
			for(int i = 0; i < PLAYER_ITEM_TOTAL; ++i)
			{
				item = *pHero->GetEquip((PLAYER_ITEM_TYPE)i);
				ObjectValid::DecryptAttrib(&item);
				if(item.id == 521)
				{
					item.type = 0;
				}
				g_xThreadBuffer << item;
			}

			char* pData = new char[MAX_VIEWEQUIPDATA_SIZE];
			uLongf srcsize = g_xThreadBuffer.GetLength();
			uLongf cmpsize = MAX_VIEWEQUIPDATA_SIZE;

			int nRet = compress((Bytef*)pData, &cmpsize, (const Bytef*)g_xThreadBuffer.GetBuffer(), srcsize);
			if(nRet == Z_OK)
			{
				//	OK
				ppopen.xData.resize(cmpsize);
				for(int i = 0; i < cmpsize; ++i)
				{
					ppopen.xData[i] = pData[i];
				}
				SendPacket(ppopen);
			}
			else
			{
				LOG(ERROR) << "Compress error!";
			}
			SAFE_DELETE_ARRAY(pData);
		}
	}
	else if(req.dwOp == CMD_OP_CHANGEATTACKMODE)
	{
		int nAttackMode = m_ePkType + 1;
		if(nAttackMode == kHeroPkType_Total)
		{
			nAttackMode = kHeroPkType_None;
		}
		m_ePkType = (HeroPkType)nAttackMode;

		PkgPlayerQuickMsgNtf ntf;
		ntf.uTargetId = GetID();
		ntf.nMsgID = QMSG_ATTACKMODE;
		ntf.nParam = m_ePkType;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ntf;
		SendPacket(ntf);
	}
}

void HeroObject::DoPacket(const PkgPlayerNetDelayReq& req)
{
	PkgPlayerNetDelayAck ack;
	ack.uTargetId = GetID();
	ack.dwSendSequence = req.dwSendSequence;
	SendPacket(ack);

	++m_cAutoSaveCounter;

	if(m_cAutoSaveCounter == 0xff)
	{
		m_cAutoSaveCounter = 0;
	}

	if(m_cAutoSaveCounter % 20 == 0)
	{
		//	auto save
		if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN &&
			!CMainServer::GetInstance()->GetAppException())
		{
#ifdef _DEBUG
			LOG(INFO) << "auto save hero [" << GetUID() << "] data";
#endif
			SendHumDataV2(false, true);
		}
	}
}

void HeroObject::DoPacket(const PkgGmNotificationReq& req)
{
	if(!IsGmHide())
	{
		return;
	}

	/*PkgGmNotificationNot not;
	not.xMsg = req.xMsg;

	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;

	GameSceneManager::GetInstance()->BroadcastPacketAllScene(&g_xThreadBuffer);*/
	PkgSystemNotifyNot nreq;
	nreq.xMsg = req.xMsg;
	nreq.dwTimes = 1;
	nreq.dwColor = 0xff7CFC00;
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << nreq;
	GameSceneManager::GetInstance()->BroadcastPacketAllScene(&g_xThreadBuffer);
}

void HeroObject::DoPacket(const PkgSystemNotifyReq& req)
{
	if(!IsGmHide())
	{
		return;
	}

	PkgSystemNotifyNot nreq;
	nreq.xMsg = req.xMsg;
	nreq.dwTimes = req.dwTimes;
	nreq.dwColor = req.dwColor;
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << nreq;
	GameSceneManager::GetInstance()->BroadcastPacketAllScene(&g_xThreadBuffer);
}

void HeroObject::DoPacket(const PkgPlayerDecomposeReq& req)
{
	ItemAttrib* pItem = GetItemByTag(req.dwItemTag);

	if(NULL != pItem)
	{
		ItemAttrib oriItem;

		if(GetRecordInItemTable(GETITEMATB(pItem, ID), &oriItem))
		{
			int nItemLevel = GetItemGrade(oriItem.id);

			if(0 != nItemLevel)
			{
				int nTotalPrice = 0;

				int nACUp = 0;
				int nMACUp = 0;
				int nDCUp = 0;
				int nMCUp = 0;
				int nSCUp = 0;

				nACUp = GETITEMATB(pItem, MaxAC) - oriItem.maxAC;
				nMACUp = GETITEMATB(pItem, MaxMAC) - oriItem.maxMAC;
				nDCUp = GETITEMATB(pItem, MaxDC) - oriItem.maxDC;
				nMCUp = GETITEMATB(pItem, MaxMC) - oriItem.maxMC;
				nSCUp = GETITEMATB(pItem, MaxSC) - oriItem.maxSC;

				int nDCGetStone = 0;
				int nMCGetStone = 0;
				int nSCGetStone = 0;

				//	Now just decompose DC MC SC
				if(nDCUp > 0)
				{
					nDCGetStone = nDCUp;
				}

				if(nMCUp > 0)
				{
					nMCGetStone = nMCUp;
				}

				if(nSCUp > 0)
				{
					nSCGetStone = nSCUp;
				}

				if(nDCGetStone > 0)
				{
					nTotalPrice += ((nItemLevel - 1) * 1000 * nDCGetStone);
				}

				if(nMCGetStone > 0)
				{
					nTotalPrice += ((nItemLevel - 1) * 1000 * nMCGetStone);
				}

				if(nSCGetStone > 0)
				{
					nTotalPrice += ((nItemLevel - 1) * 1000 * nSCGetStone);
				}

				/*static const int s_nDCStoneTable[6] = {685,686,687,688,689,691};
				static const int s_nMCStoneTable[6] = {692,693,694,695,696,697};
				static const int s_nSCStoneTable[6] = {698,699,700,701,702,703};*/

				int nChekcedID = 0;

				if(nTotalPrice > 0)
				{
					if(GetMoney() >= nTotalPrice)
					{
						//	Check if have enough room
						bool bCanDecompose = true;
						int nEmptyRoom = GetBagEmptySum();

						if(nDCUp > 0)
						{
							nChekcedID = g_nDCStoneTable[nItemLevel - 2];
							size_t uSize = m_xBag.size();
							int nLeft = nDCUp;

							for(int i = 0; i < uSize; ++i)
							{
								if(GETITEMATB(&m_xBag[i], ID) == nChekcedID)
								{
									if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX)
									{
										nLeft -= (GRID_MAX - GETITEMATB(&m_xBag[i], AtkSpeed));
									}
								}

								if(nLeft <= 0)
								{
									break;
								}
							}

							if(nLeft > 0)
							{
								--nEmptyRoom;
							}
						}

						if(nSCUp > 0)
						{
							nChekcedID = g_nSCStoneTable[nItemLevel - 2];
							size_t uSize = m_xBag.size();
							int nLeft = nSCUp;

							for(int i = 0; i < uSize; ++i)
							{
								if(GETITEMATB(&m_xBag[i], ID) == nChekcedID)
								{
									if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX)
									{
										nLeft -= (GRID_MAX - GETITEMATB(&m_xBag[i], AtkSpeed));
									}
								}

								if(nLeft <= 0)
								{
									break;
								}
							}

							if(nLeft > 0)
							{
								--nEmptyRoom;
							}
						}

						if(nMCUp > 0)
						{
							nChekcedID = g_nMCStoneTable[nItemLevel - 2];
							size_t uSize = m_xBag.size();
							int nLeft = nMCUp;

							for(int i = 0; i < uSize; ++i)
							{
								if(GETITEMATB(&m_xBag[i], ID) == nChekcedID)
								{
									if(GETITEMATB(&m_xBag[i], AtkSpeed) < GRID_MAX)
									{
										nLeft -= (GRID_MAX - GETITEMATB(&m_xBag[i], AtkSpeed));
									}
								}

								if(nLeft <= 0)
								{
									break;
								}
							}

							if(nLeft > 0)
							{
								--nEmptyRoom;
							}
						}

						if(nEmptyRoom >= 0)
						{
							MinusMoney(nTotalPrice);

							PkgPlayerUpdateAttribNtf ppuan;
							ppuan.uTargetId = GetID();
							ppuan.bType = UPDATE_MONEY;
							ppuan.dwParam = GetMoney();
							SendPacket(ppuan);

							PkgPlayerClearBagNtf ppcbn;
							ppcbn.uTargetId = GetID();
							ppcbn.dwTag = pItem->tag;
							g_xThreadBuffer.Reset();
							g_xThreadBuffer << ppcbn;
							SendPacket(ppcbn);

							ZeroMemory(pItem, sizeof(ItemAttrib));
							ObjectValid::EncryptAttrib(pItem);

							//	Add Item
							for(int i = 0; i < nDCGetStone; ++i)
							{
								AddItem(g_nDCStoneTable[nItemLevel - 2]);
							}
							for(int i = 0; i < nMCGetStone; ++i)
							{
								AddItem(g_nMCStoneTable[nItemLevel - 2]);
							}
							for(int i = 0; i < nSCGetStone; ++i)
							{
								AddItem(g_nSCStoneTable[nItemLevel - 2]);
							}
						}
					}
				}
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerForgeItemReq& req)
{
	if(req.xItems.empty())
	{
		return;
	}

	bool bUpgradeItem = false;
	ItemAttrib* pStone = NULL;
	ItemAttrib* pEquip = NULL;

	if(req.xItems.size() == 1)
	{
		pStone = GetItemByTag(req.xItems.front());
	}
	else if(req.xItems.size() == 2)
	{
		pStone = GetItemByTag(req.xItems.front());
		pEquip = GetItemByTag(req.xItems.back());
	}

	/*static const int s_nDCStoneTable[6] = {685,686,687,688,689,691};
	static const int s_nMCStoneTable[6] = {692,693,694,695,696,697};
	static const int s_nSCStoneTable[6] = {698,699,700,701,702,703};*/

	if(pStone &&
		!pEquip)
	{
		//	Upgrade stone
		if(GETITEMATB(pStone, Type) == ITEM_COST &&
			GETITEMATB(pStone, Curse) == 3 &&
			GETITEMATB(pStone, Lucky) == 1 &&
			GETITEMATB(pStone, AtkSpeed) == GRID_MAX)
		{
			if(GETITEMATB(pStone, Accuracy) <= (UPGRADESTONE_MAX_LEVEL - 1))
			{
				int nNextValue = GETITEMATB(pStone, Accuracy);
				int nCostMoney = nNextValue * 10000;

				if(GetMoney() >= nCostMoney)
				{
					MinusMoney(nCostMoney);
					PkgPlayerUpdateAttribNtf ppuan;
					ppuan.uTargetId = GetID();
					ppuan.dwParam = GetMoney();
					SendPacket(ppuan);

					PkgPlayerClearBagNtf ppcbn;
					ppcbn.uTargetId = GetID();
					ppcbn.dwTag = pStone->tag;
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ppcbn;
					SendPacket(ppcbn);

					if(GETITEMATB(pStone, Hide) == 1)
					{
						//	DC
						//for(int i = 0; i < GRID_MAX; ++i)
						{
							AddItem(g_nDCStoneTable[nNextValue]);
						}
					}
					else if(GETITEMATB(pStone, Hide) == 2)
					{
						//	MC
						//for(int i = 0; i < GRID_MAX; ++i)
						{
							AddItem(g_nMCStoneTable[nNextValue]);
						}
					}
					else if(GETITEMATB(pStone, Hide) == 3)
					{
						//	SC
						//for(int i = 0; i < GRID_MAX; ++i)
						{
							AddItem(g_nSCStoneTable[nNextValue]);
						}
					}

					ZeroMemory(pStone, sizeof(ItemAttrib));
					ObjectValid::EncryptAttrib(pStone);
				}
			}
		}
	}
	else if(pStone &&
		pEquip)
	{
		if(GETITEMATB(pStone, Type) == ITEM_COST &&
			GETITEMATB(pStone, Curse) == 3 &&
			GETITEMATB(pStone, Lucky) == 1 &&
			IsEquipItem(GETITEMATB(pEquip, Type)))
		{
			//	Upgrade equipment
			ItemAttrib stCheckItem;
			ItemAttrib* pCheckItem = (ItemAttrib*)pEquip;
			memcpy(&stCheckItem, pCheckItem, sizeof(ItemAttrib));
			//	Decrypt
			ObjectValid::DecryptAttrib(&stCheckItem);
			pCheckItem = &stCheckItem;

			bUpgradeItem = false;
			int nUpgradeValue = 0;
			BYTE bLow = LOBYTE(pCheckItem->level);
			BYTE bHigh = HIBYTE(pCheckItem->level);

			{
				if(pCheckItem->level == 0)
				{
					nUpgradeValue = 0;
				}
				else
				{
					BYTE bKey = GetItemMakeMask(bHigh);
					int nValue = GetMakeMaskValue(bKey);
					nValue = (int)bLow - nValue;
					nUpgradeValue = nValue;
				}

				if(nUpgradeValue < 5 &&
					nUpgradeValue >= 0)
				{
					int nItemGrade = GetItemGrade(pCheckItem->id);
					if(0 == nItemGrade)
					{
						return;
					}
					//int nCostMoney = (nUpgradeValue + 1) * 20000;
					int nCostMoney = (nUpgradeValue + 1) * nItemGrade * 2500;

					if(GetMoney() >= nCostMoney)
					{
						MinusMoney(nCostMoney);

						PkgPlayerUpdateAttribNtf ppuan;
						ppuan.uTargetId = GetID();
						ppuan.dwParam = GetMoney();
						SendPacket(ppuan);

						int nStoneGrade = GETITEMATB(pStone, Accuracy);

						if(nStoneGrade >= nItemGrade)
						{
							int nLeft = GETITEMATB(pStone, AtkSpeed);
							if(nLeft >= 1)
							{
								--nLeft;
								SETITEMATB(pStone, AtkSpeed, nLeft);

								if(GETITEMATB(pStone, Hide) == 1)
								{
									//	DC
									GameWorld::GetInstance().UpgradeAttrib(&stCheckItem, ATTRIB_DC, 1);
								}
								else if(GETITEMATB(pStone, Hide) == 2)
								{
									//	MC
									GameWorld::GetInstance().UpgradeAttrib(&stCheckItem, ATTRIB_MC, 1);
								}
								else if(GETITEMATB(pStone, Hide) == 3)
								{
									//	SC
									GameWorld::GetInstance().UpgradeAttrib(&stCheckItem, ATTRIB_SC, 1);
								}
							}

							PkgPlayerUpdateCostNtf ppucn;
							ppucn.uTargetId = GetID();
							ppucn.dwTag = pStone->tag;
							ppucn.nNumber = nLeft;
							SendPacket(ppucn);

							if(nLeft == 0)
							{
								ZeroMemory(pStone, sizeof(ItemAttrib));
								ObjectValid::EncryptAttrib(pStone);
							}

							//	Bind the item
							SET_FLAG(stCheckItem.atkPois, POIS_MASK_BIND);

							PkgPlayerUpdateItemNtf ppuin;
							ppuin.uTargetId = GetID();
							ppuin.stItem = stCheckItem;
							SendPacket(ppuin);

							ObjectValid::EncryptAttrib(&stCheckItem);
							memcpy(pEquip, &stCheckItem, sizeof(ItemAttrib));
						}
					}
				}
			}
		}
		else if(GETITEMATB(pStone, Type) == ITEM_COST &&
			GETITEMATB(pStone, Curse) == 3 &&
			GETITEMATB(pStone, Lucky) == 2 &&
			IsEquipItem(GETITEMATB(pEquip, Type)))
		{
			//	Upgrade equipment
			ItemAttrib stCheckItem;
			ItemAttrib* pCheckItem = (ItemAttrib*)pEquip;
			memcpy(&stCheckItem, pCheckItem, sizeof(ItemAttrib));
			//	Decrypt
			ObjectValid::DecryptAttrib(&stCheckItem);
			pCheckItem = &stCheckItem;

			bUpgradeItem = false;
			int nUpgradeValue = 0;
			int nPreHP = pCheckItem->HP;
			int nPreMaxMP = pCheckItem->maxMP;
			int nPreMP = pCheckItem->MP;
			/*BYTE bLow = LOBYTE(pCheckItem->level);
			BYTE bHigh = HIBYTE(pCheckItem->level);

			if(pCheckItem->level == 0)
			{
				nUpgradeValue = 0;
			}
			else
			{
				BYTE bKey = GetItemMakeMask(bHigh);
				int nValue = GetMakeMaskValue(bKey);
				nValue = (int)bLow - nValue;
				nUpgradeValue = nValue;
			}*/
			nUpgradeValue = GetItemUpgrade(pCheckItem->level);

			if(nUpgradeValue > 0 &&
				nUpgradeValue <= 8)
			{
				int nCostMoney = nUpgradeValue * 15000;
				if(GetMoney() >= nCostMoney)
				{
					MinusMoney(nCostMoney);

					PkgPlayerUpdateAttribNtf ppuan;
					ppuan.uTargetId = GetID();
					ppuan.dwParam = GetMoney();
					SendPacket(ppuan);

					int nLeft = GETITEMATB(pStone, AtkSpeed);
					if(nLeft > 0)
					{
						--nLeft;
						SETITEMATB(pStone, AtkSpeed, nLeft);

						int nTag = pCheckItem->tag;
						//Go back to original attribute
						if(GetRecordInItemTable(pCheckItem->id, pCheckItem))
						{
							GameWorld::GetInstance().UpgradeItemsWithAddition(pCheckItem, nUpgradeValue);
							SET_FLAG(pCheckItem->atkPois, POIS_MASK_BIND);
							pCheckItem->tag = nTag;
							pCheckItem->HP = nPreHP;
							pCheckItem->maxMP = nPreMaxMP;
							pCheckItem->MP = nPreMP;

							PkgPlayerUpdateCostNtf ppucn;
							ppucn.uTargetId = GetID();
							ppucn.dwTag = pStone->tag;
							ppucn.nNumber = nLeft;
							SendPacket(ppucn);

							if(0 == nLeft)
							{
								ZeroMemory(pStone, sizeof(ItemAttrib));
								ObjectValid::EncryptAttrib(pStone);
							}

							PkgPlayerUpdateItemNtf ppuin;
							ppuin.uTargetId = GetID();
							ppuin.stItem = stCheckItem;
							SendPacket(ppuin);

							ObjectValid::EncryptAttrib(&stCheckItem);
							memcpy(pEquip, &stCheckItem, sizeof(ItemAttrib));
						}
					}
				}
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerSplitItemReq& req)
{
	ItemAttrib* pSpliteItem = NULL;
	pSpliteItem = GetItemByTag(req.dwTag);

	if(NULL == pSpliteItem)
	{
		return;
	}

	if(GETITEMATB(pSpliteItem, Type) != ITEM_COST)
	{
		return;
	}

	if(GETITEMATB(pSpliteItem, AtkSpeed) <= 1)
	{
		return;
	}

	if(req.cSum <= 0 ||
		req.cSum >= GRID_MAX)
	{
		return;
	}

	int nItemSum = GETITEMATB(pSpliteItem, AtkSpeed);
	if(req.cSum >= nItemSum)
	{
		return;
	}

	int nEmptyBag = GetBagEmptySum();
	if(0 == nEmptyBag)
	{
		return;
	}

	int nNumber = nItemSum - req.cSum;

	PkgPlayerUpdateCostNtf ntf;
	ntf.uTargetId = GetID();
	ntf.dwTag = pSpliteItem->tag;
	ntf.nNumber = nNumber;
	SendPacket(ntf);

	SETITEMATB(pSpliteItem, AtkSpeed, nNumber);

	//	new item
	//	find empty bag
	bool bNeedBind = TEST_FLAG_BOOL(GETITEMATB(pSpliteItem, AtkPois), POIS_MASK_BIND);
	ItemAttrib* pBagItem = NULL;

	if(pBagItem == NULL)
	{
		//	search empty item
		for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
		{
			if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
				m_xBag[i].tag != ITEMTAG_INQUERY)
			{
				pBagItem = &m_xBag[i];
				break;
			}
		}
	}

	int nAttribID = GETITEMATB(pSpliteItem, ID);
	ItemAttrib destItem;
	if(!GetRecordInItemTable(nAttribID, &destItem))
	{
		return;
	}
	if(destItem.type != ITEM_COST)
	{
		return;
	}

	if(NULL != pBagItem)
	{
		if(GETITEMATB(pBagItem, Type) == ITEM_NO)
		{
			memcpy(pBagItem, &destItem, sizeof(ItemAttrib));
			pBagItem->tag = GameWorld::GetInstance().GenerateItemTag();
			pBagItem->atkSpeed = req.cSum;

			if(bNeedBind)
			{
				SET_FLAG(pBagItem->atkPois, POIS_MASK_BIND);
			}
			
			ObjectValid::EncryptAttrib(pBagItem);

			PkgPlayerGainItemNtf ntf;
			ntf.uTargetId = GetID();
			ntf.stItem = *pBagItem;
			ObjectValid::DecryptAttrib(&ntf.stItem);
			SendPacket(ntf);
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerOffGetListReq& req)
{
	if(CMainServer::GetInstance()->GetServerMode() != GM_LOGIN)
	{
		return;
	}
	if(!GameWorld::GetInstancePtr()->IsEnableOfflineSell())
	{
		return;
	}

	if(GetTickCount() - m_dwLastReqOffSellItems < 10 * 1000)
	{
		return;
	}
	m_dwLastReqOffSellItems = GetTickCount();

	SellItemVector xSellItems;
	OfflineSellSystem::GetInstance()->UpdateExpireItems();
	OfflineSellSystem::GetInstance()->GetAllSellItems(xSellItems);

	if(!xSellItems.empty())
	{
		std::reverse(xSellItems.begin(), xSellItems.end());
		int nMaxPage = (xSellItems.size() + 3) / 4;

		for(int i = 0; i < nMaxPage; ++i)
		{
			PkgPlayerOffSellListNtf ntf;
			ntf.uTargetId = GetID();
			ntf.bPage = i;

			for(int j = 0; j < 4; ++j)
			{
				int nItemIndex = i * 4 + j;
				if(nItemIndex >= xSellItems.size())
				{
					break;
				}

				ntf.xItems.push_back(xSellItems[nItemIndex]);
			}

			SendPacket(ntf);
		}
	}
	else
	{
		PkgPlayerOffSellListNtf ntf;
		ntf.uTargetId = GetID();
		ntf.bPage = 0;
		SendPacket(ntf);
	}
}

void HeroObject::DoPacket(const PkgPlayerOffBuyItemReq& req)
{
	if(CMainServer::GetInstance()->GetServerMode() != GM_LOGIN)
	{
		return;
	}
	if(!GameWorld::GetInstancePtr()->IsEnableOfflineSell())
	{
		return;
	}

	const SellItem* pSellItem = OfflineSellSystem::GetInstance()->GetSellItem(req.dwItemID);
	if(NULL == pSellItem)
	{
		return;
	}

	if(pSellItem->nUID != GetUID())
	{
		if(GetMoney() < pSellItem->nMoney)
		{
			return;
		}
	}

	//	add item
	ItemAttrib* pBagItem = NULL;

	for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
	{
		if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
			m_xBag[i].tag != ITEMTAG_INQUERY)
		{
			pBagItem = &m_xBag[i];
			break;
		}
	}

	if(NULL != pBagItem)
	{
		if(GETITEMATB(pBagItem, Type) == ITEM_NO)
		{
			//	not encrypt
			memcpy(pBagItem, &pSellItem->stAttrib, sizeof(ItemAttrib));
			int nPois = GETITEMATB(pBagItem, AtkPois);
			SET_FLAG(nPois, POIS_MASK_BIND);
			SETITEMATB(pBagItem, AtkPois, nPois);

			//	encrypt
			ObjectValid::EncryptAttrib(pBagItem);

			// notify player
			PkgPlayerGainItemNtf ntf;
			ntf.stItem = *pBagItem;
			ObjectValid::DecryptAttrib(&ntf.stItem);
			ntf.uTargetId = GetID();
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ntf;
			SendBuffer(GetUserIndex(), &g_xThreadBuffer);

			if(pSellItem->nUID != GetUID())
			{
				//	不是同一个玩家 收金钱 放入已售物品队列
				MinusMoney(pSellItem->nMoney);

				char szName[20] = {0};
				if(IsEncrypt())
				{
					ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
				}
				else
				{
					strcpy(szName, GetUserData()->stAttrib.name);
				}

				OfflineSellSystem::GetInstance()->AddSoldItem(pSellItem->nUID, pSellItem->nMoney, pSellItem->nGold, ntf.stItem.name, szName);
			}
			OfflineSellSystem::GetInstance()->RemoveSellItem(pSellItem->nItemID);
		}
	}
	else
	{
		PkgPlayerQuickMsgNtf qm;
		qm.uTargetId = GetID();
		qm.nMsgID = QMSG_NOBAGROOM;
		SendPacket(qm);
	}
}

void HeroObject::DoPacket(const PkgPlayerOffCheckSoldReq& req)
{
	if(CMainServer::GetInstance()->GetServerMode() != GM_LOGIN)
	{
		return;
	}

	if(!GameWorld::GetInstancePtr()->IsEnableOfflineSell())
	{
		return;
	}

	SoldItemList xSoldItems;
	OfflineSellSystem::GetInstance()->QuerySoldItem(GetUID(), xSoldItems);

	if(xSoldItems.empty())
	{
		PkgPlayerQuickMsgNtf qm;
		qm.uTargetId = GetID();
		qm.nMsgID = QMSG_OFFNOTSOLD;
		SendPacket(qm);
	}
	else
	{
		SoldItemList::const_iterator begIter = xSoldItems.begin();
		SoldItemList::const_iterator endIter = xSoldItems.end();

		for(begIter;
			begIter != endIter;
			++begIter)
		{
			PkgPlayerOffSoldItemAck ack;
			ack.uTargetId = GetID();
			ack.xItemName = begIter->szItemName;
			ack.xBuyerName = begIter->szBuyerName;
			ack.dwMoney = begIter->nMoney;
			ack.dwGold = begIter->nGold;
			SendPacket(ack);

			AddMoney(ack.dwMoney);

			OfflineSellSystem::GetInstance()->RemoveSoldItem(begIter->nItemID);
		}
	}


	//	回收过期物品
	ExpireItemList xExpireItems;
	OfflineSellSystem::GetInstance()->QueryExpireItem(GetUID(), xExpireItems);

	for(ExpireItemList::iterator begIter = xExpireItems.begin();
		begIter != xExpireItems.end();
		++begIter)
	{
		ExpireItem& refItem = *begIter;

		if(refItem.nUID == GetUID())
		{
			//	add item
			ItemAttrib* pBagItem = NULL;

			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
					m_xBag[i].tag != ITEMTAG_INQUERY)
				{
					pBagItem = &m_xBag[i];
					break;
				}
			}

			if(NULL == pBagItem)
			{
				PkgPlayerQuickMsgNtf qm;
				qm.uTargetId = GetID();
				qm.nMsgID = QMSG_NOBAGROOM;
				SendPacket(qm);

				return;
			}

			if(GETITEMATB(pBagItem, Type) == ITEM_NO)
			{
				//	not encrypt
				memcpy(pBagItem, &refItem.stAttrib, sizeof(ItemAttrib));
				int nPois = GETITEMATB(pBagItem, AtkPois);
				SET_FLAG(nPois, POIS_MASK_BIND);
				SETITEMATB(pBagItem, AtkPois, nPois);

				//	encrypt
				ObjectValid::EncryptAttrib(pBagItem);

				// notify player
				PkgPlayerGainItemNtf ntf;
				ntf.stItem = *pBagItem;
				ObjectValid::DecryptAttrib(&ntf.stItem);
				ntf.uTargetId = GetID();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ntf;
				SendBuffer(GetUserIndex(), &g_xThreadBuffer);

				OfflineSellSystem::GetInstance()->RemoveExpireItem(refItem.nItemID);
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerOffSellItemReq& req)
{
	if(CMainServer::GetInstance()->GetServerMode() != GM_LOGIN)
	{
		return;
	}

	if(!GameWorld::GetInstancePtr()->IsEnableOfflineSell())
	{
		return;
	}

	if(GetMoney() < 1000)
	{
		return;
	}

	if(req.dwTag == 0)
	{
		return;
	}

	ItemAttrib* pItem = GetItemByTag(req.dwTag);
	if(NULL == pItem)
	{
		PkgPlayerQuickMsgNtf qm;
		qm.uTargetId = GetID();
		qm.nMsgID = 1;
		return;
	}

	if(ITEM_NO == GETITEMATB(pItem, Type))
	{
		return;
	}

	int nBind = GETITEMATB(pItem, AtkPois);
	if(TEST_FLAG_BOOL(nBind, POIS_MASK_BIND))
	{
		return;
	}

	int nSellMoney = req.dwMoney;
	if(req.dwMoney > MAX_MONEY)
	{
		nSellMoney = MAX_MONEY;
	}

	ItemAttrib item;
	item = *pItem;
	ObjectValid::DecryptAttrib(&item);

	char szName[20] = {0};
	if(IsEncrypt())
	{
		ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
	}
	else
	{
		strcpy(szName, GetUserData()->stAttrib.name);
	}
	if(OfflineSellSystem::GetInstance()->AddSellItem(GetUID(), nSellMoney, req.dwGold, szName, &item))
	{
		PkgPlayerQuickMsgNtf qm;
		qm.uTargetId = GetID();
		qm.nMsgID = QMSG_OFFSELLOK;
		qm.nParam = 1;
		SendPacket(qm);

		//	destory
		PkgPlayerLostItemAck ack;
		ack.uTargetId = GetID();
		ack.dwTag = pItem->tag;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		SendBuffer(GetUserIndex(), &g_xThreadBuffer);
		memset(pItem, 0, sizeof(ItemAttrib));
		ObjectValid::EncryptAttrib(pItem);

		MinusMoney(1000);
	}
	else
	{
		PkgPlayerQuickMsgNtf qm;
		qm.uTargetId = GetID();
		qm.nMsgID = QMSG_OFFSELLOK;
		qm.nParam = 0;
		SendPacket(qm);
	}
}

void HeroObject::DoPacket(const PkgPlayerPrivateChatReq& req)
{
	if(!req.xPeerName.empty())
	{
		UserInfoList::const_iterator begIter = g_xUserInfoList.begin();
		UserInfoList::const_iterator endIter = g_xUserInfoList.end();

		int nHandlerID = 0;

		for(begIter;
			begIter != endIter;
			++begIter)
		{
			if(begIter->xName == req.xPeerName)
			{
				nHandlerID = begIter->uHandlerID;
				break;
			}
		}

		//HeroObject* pDestHero = (HeroObject*)GameSceneManager::GetInstance()->GetPlayerByName(req.xPeerName.c_str());

		if(nHandlerID &&
			nHandlerID != GetID())
		{
			HeroObject* pHero = (HeroObject*)GameSceneManager::GetInstance()->GetPlayer(nHandlerID);

			if(NULL != pHero)
			{
				char szName[20];
				ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);

				PkgPlayerPrivateChatNtf ntf;
				ntf.xSender = szName;
				ntf.xMsg = req.xMsg;
				ntf.uTargetId = nHandlerID;
				pHero->SendPacket(ntf);
			}
			else
			{
				PkgPlayerPrivateChatAck ack;
				ack.bOnline = false;
				ack.xPeerName = req.xPeerName;
				ack.uTargetId = GetID();
				SendPacket(ack);
			}
		}
		else
		{
			if(nHandlerID != GetID())
			{
				PkgPlayerPrivateChatAck ack;
				ack.bOnline = false;
				ack.xPeerName = req.xPeerName;
				ack.uTargetId = GetID();
				SendPacket(ack);
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerOffTakeBackReq& req)
{
	if(CMainServer::GetInstance()->GetServerMode() != GM_LOGIN)
	{
		return;
	}
	if(!GameWorld::GetInstancePtr()->IsEnableOfflineSell())
	{
		return;
	}

	if(GetTickCount() - m_dwLastReqOffTakeBack < 8 * 1000)
	{
		return;
	}

	m_dwLastReqOffTakeBack = GetTickCount();

	SellItemVector xSellItems;
	OfflineSellSystem::GetInstance()->GetAllSellItems(xSellItems);

	for(int i = 0; i < xSellItems.size(); ++i)
	{
		SellItem& refItem = xSellItems[i];

		if(refItem.nUID == GetUID())
		{
			//	add item
			ItemAttrib* pBagItem = NULL;

			for(int i = 0; i < HERO_BAG_SIZE_CUR; ++i)
			{
				if(GETITEMATB(&m_xBag[i], Type) == ITEM_NO &&
					m_xBag[i].tag != ITEMTAG_INQUERY)
				{
					pBagItem = &m_xBag[i];
					break;
				}
			}

			if(NULL == pBagItem)
			{
				PkgPlayerQuickMsgNtf qm;
				qm.uTargetId = GetID();
				qm.nMsgID = QMSG_NOBAGROOM;
				SendPacket(qm);

				return;
			}

			if(GETITEMATB(pBagItem, Type) == ITEM_NO)
			{
				//	not encrypt
				memcpy(pBagItem, &refItem.stAttrib, sizeof(ItemAttrib));
				int nPois = GETITEMATB(pBagItem, AtkPois);
				SET_FLAG(nPois, POIS_MASK_BIND);
				SETITEMATB(pBagItem, AtkPois, nPois);

				//	encrypt
				ObjectValid::EncryptAttrib(pBagItem);

				// notify player
				PkgPlayerGainItemNtf ntf;
				ntf.stItem = *pBagItem;
				ObjectValid::DecryptAttrib(&ntf.stItem);
				ntf.uTargetId = GetID();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ntf;
				SendBuffer(GetUserIndex(), &g_xThreadBuffer);

				OfflineSellSystem::GetInstance()->RemoveSellItem(refItem.nItemID);
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerIdentifyItemReq &req)
{
	ItemAttrib* pItem = GetItemByTag(req.dwTag);
	if(NULL == pItem)
	{
		return;
	}

	//	是否是装备
	int nItemType = GETITEMATB(pItem, Type);
	if(!IsEquipItem(nItemType))
	{
		return;
	}

	//	符咒无法辨识
	if(nItemType == ITEM_CHARM)
	{
		return;
	}

	//	获得装备的阶级
	int nEquipLevel = GetItemGrade(GETITEMATB(pItem, ID));
	if (nEquipLevel >= 1 &&
		nEquipLevel <= 3)
	{
		IdentifyLowLevelEquip(pItem);
		return;
	}

	UINT uHideAttribCode = GETITEMATB(pItem, MaxMP);
	if(0 == uHideAttribCode)
	{
		return;
	}

	int nActiveAttribSum = HideAttribHelper::GetActiveAttribCount(uHideAttribCode);
	int nAllAttribSum = HideAttribHelper::GetAllAttribCount(uHideAttribCode);

	if(nActiveAttribSum >= nAllAttribSum)
	{
		PkgPlayerQuickMsgNtf ntf;
		ntf.uTargetId = GetID();
		ntf.nMsgID = QMSG_IDENTIFYFULL;
		SendPacket(ntf);
		return;
	}

	//	检查身上的辨识卷轴是否足够
	int nNeedItem = nActiveAttribSum + 1;

	if(CountItem(1123) < nNeedItem)
	{
		PkgPlayerQuickMsgNtf ntf;
		ntf.uTargetId = GetID();
		ntf.nMsgID = QMSG_NOIDENTIFYSCROLL;
		SendPacket(ntf);
		return;
	}

	//	扣钱
	int nCostMoney = 0;
	nCostMoney = (1 + GetItemUpgrade(GETITEMATB(pItem, Level))) * 10000;

	if(GetMoney() < nCostMoney)
	{
		PkgPlayerQuickMsgNtf ntf;
		ntf.uTargetId = GetID();
		ntf.nMsgID = QMSG_NOENOUGHMONEY;
		SendPacket(ntf);
		return;
	}

	MinusMoney(nCostMoney);
	ClearItem(1123, nNeedItem);
	
	nActiveAttribSum++;
	HideAttribHelper::SetActiveAttribCount(uHideAttribCode, nActiveAttribSum);
	SETITEMATB(pItem, MaxMP, uHideAttribCode);

	int nValue = GETITEMATB(pItem, AtkPois);
	SET_FLAG(nValue, POIS_MASK_BIND);
	SETITEMATB(pItem, AtkPois, nValue);

	PkgPlayerQuickMsgNtf ntf;
	ntf.uTargetId = GetID();
	ntf.nMsgID = QMSG_IDENTIFYOK;
	SendPacket(ntf);

	SyncItemAttrib(req.dwTag);
}

void HeroObject::DoPacket(const PkgPlayerUnbindItemReq &req)
{
	ItemAttrib* pItem = GetItemByTag(req.dwTag);
	if(NULL == pItem)
	{
		return;
	}

	//	是否是装备
	if(!IsEquipItem(pItem->type))
	{
		return;
	}
	if(CountItem(1118) <= 0)
	{
		PkgPlayerQuickMsgNtf ntf;
		ntf.uTargetId = GetID();
		ntf.nMsgID = QMSG_UNBINDFAILED;
		SendPacket(ntf);
		return;
	}

	if(GETITEMATB(pItem, ID) == 66)
	{

	}
}

void HeroObject::DoPacket(const PkgPlayerServerDelayAck &ack)
{
	if(ack.nSeq != m_nServerNetDelaySeq)
	{
		return;
	}

	DWORD dwCurrentTick = GetTickCount();
	if(ack.dwTimeStamp > dwCurrentTick)
	{
		return;
	}

	m_dwServerNetDelaySec = (dwCurrentTick - ack.dwTimeStamp) / 2;
}

void HeroObject::DoPacket(const PkgPlayerRankListReq& req)
{
	PkgPlayerRankListNot not;
	not.uTargetId = GetID();
	const char* pRankListData = GameWorld::GetInstance().GetRankListData();

	if(NULL != pRankListData)
	{
		not.xData = pRankListData;
	}

	SendPacket(not);
}

void HeroObject::DoPacket(const PkgPlayerGetOlShopListReq &req)
{
	PkgPlayerGetOlShopListAck ack;
	OlShopManager::GetInstance()->GetShopItems(ack.xItemsId);
	ack.uTargetId = GetID();
	SendPacket(ack);

	SyncDonateLeft();
}

void HeroObject::DoPacket(const PkgPlayerBuyOlShopItemReq &req)
{
	if(GetBagEmptySum() < 1)
	{
		SendQuickMessage(QMSG_NOBAGROOM);
		return;
	}

	if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
	{
		g_xConsole.CPrint("Player[%d] request to buy olshop item[%d]", GetUID(), req.nItemId);

		if(OlShopManager::GetInstance()->IsWaitForQuery(GetUID()))
		{
			LOG(WARNING) << "Player[" << GetUID() << "] waiting for ol shop query";
			return;
		}

		//	get item attrib
		ItemAttrib item;
		if(!GetRecordInItemTable(req.nItemId, &item))
		{
			return;
		}

		OlShopManager::GetInstance()->SetWaitForQuery(GetUID(), true);

		PkgLoginCheckBuyShopItemReq lreq;
		lreq.nItemId = req.nItemId;
		lreq.nCost = item.price;
		lreq.nGsId = GetID();
		lreq.nUid = GetUID();
		lreq.nQueryId = OlShopManager::GetInstance()->GetQueryID(GetUID());
		
		DWORD dwLsIndex = CMainServer::GetInstance()->GetLSConnIndex();
		if(0 != dwLsIndex)
		{
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << lreq;
			SendBufferToServer(dwLsIndex, &g_xThreadBuffer);
		}
	}
	else
	{
		if(req.nItemId >= sizeof(g_nItemPrice) / sizeof(g_nItemPrice[0]))
		{
			return;
		}

		int nTotalPrice = g_nItemPrice[req.nItemId] * 10000;
		ItemAttrib item;

		if(GetRecordInItemTable(req.nItemId, &item))
		{
			if(GetMoney() >= nTotalPrice)
			{
				AddItem(req.nItemId);

				int nLeftMoney = GetMoney() - nTotalPrice;
				SetMoney(nLeftMoney);

				PkgPlayerUpdateAttribNtf uantf;
				uantf.uTargetId = GetID();
				//uantf.dwParam = MAKELONG(UPDATE_MONEY, 0);
				uantf.bType = UPDATE_MONEY;
				uantf.dwParam = GetMoney();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << uantf;
				SendPlayerBuffer(g_xThreadBuffer);
			}
		}
	}
}

void HeroObject::DoPacket(const PkgLoginCheckBuyShopItemAck& ack)
{
	if(0 != ack.cRet)
	{
		if(GetBagEmptySum() < 1)
		{
			SendQuickMessage(QMSG_NOBAGROOM);
			return;
		}

		//	购买成功
		ItemAttrib item;
		if(GetRecordInItemTable(ack.nItemId, &item))
		{
			PkgLoginConsumeDonateReq req;
			req.nCost = item.price;
			req.nGsId = GetID();
			req.nItemId = item.id;
			req.nUid = GetUID();

			char szName[20] = {0};
			if(IsEncrypt())
			{
				ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
			}
			else
			{
				strcpy(szName, GetUserData()->stAttrib.name);
			}
			req.xName = szName;

			DWORD dwLsIndex = CMainServer::GetInstance()->GetLSConnIndex();
			if(0 != dwLsIndex)
			{
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << req;
				SendBufferToServer(dwLsIndex, &g_xThreadBuffer);
			}

			//	给玩家添加道具
			if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
			{
				//AddItemNoBind(item.id);
				AddItem(item.id);
			}
			else
			{
				AddItem(item.id);
			}
		}
	}
	else
	{
		PkgPlayerBuyOlShopItemAck back;
		back.nItemId = ack.nItemId;
		back.nRet = -1;
		back.uTargetId = GetID();
		SendPacket(back);
	}
}

void HeroObject::DoPacket(const PkgLoginConsumeDonateAck& ack)
{
	if(0 == ack.cRet)
	{
		return;
	}

	//	更新玩家的贡献点
	GameObjectList xHeroList;
	GameSceneManager::GetInstance()->GetPlayerByUid(ack.nUid, xHeroList);
	if(!xHeroList.empty())
	{
		for(GameObjectList::const_iterator begIter = xHeroList.begin();
			begIter != xHeroList.end();
			++begIter)
		{
			GameObject* pObj = *begIter;
			if(pObj->GetType() == SOT_HERO)
			{
				HeroObject* pHero = (HeroObject*)pObj;
				pHero->SetDonateLeft(ack.nDonateLeft);
				pHero->SyncDonateLeft();
			}
		}
	}
}

void HeroObject::DoPacket(const PkgPlayerSmeltMaterialsReq& req)
{
	int nItemsSize = req.xItemsTag.size();
	if(req.xItemsTag.size() == 0 ||
		req.xItemsTag.size() > 5)
	{
		return;
	}

	ItemAttrib* pItems[5] = {0};

	for(int i = 0; i < req.xItemsTag.size(); ++i)
	{
		pItems[i] = GetItemByTag(req.xItemsTag[i]);
		if(NULL == pItems[i])
		{
			return;
		}
	}

	//	判断是否重复
	for(int i = 0; i < nItemsSize; ++i)
	{
		for(int j = 0; j < nItemsSize; ++j)
		{
			if(i == j)
			{
				continue;
			}

			if(req.xItemsTag[i] == req.xItemsTag[j])
			{
				LOG(ERROR) << "Can't smelt materials with same tag";
				return;
			}
		}
	}

	int nPrevId = 0;

	for(int i = 0; i < req.xItemsTag.size(); ++i)
	{
		//	判断是否全部为同一个ID 并且是材料
		if(0 == nPrevId)
		{
			nPrevId = GETITEMATB(pItems[i], ID);
		}
		else
		{
			if(GETITEMATB(pItems[i], ID) != nPrevId)
			{
				return;
			}
		}
	}

	//	判断是否是材料
	if(GETITEMATB(pItems[0], Type) != ITEM_OTHER)
	{
		return;
	}
	int nOtherType = GETITEMATB(pItems[0], Curse);
	if(nOtherType % 2 != 1)
	{
		return;
	}

	//	材料制作后的Id
	int nDestItemId = GETITEMATB(pItems[0], MaxDC);
	if(0 == nDestItemId)
	{
		LOG(ERROR) << "Can't smelt material.Id:" << nPrevId;
		return;
	}

	//	进行合成
	const LifeSkillInfo* pSkill = NULL;
	pSkill = StoveManager::GetInstance()->GetLifeSkillInfo(LifeSkillType((nOtherType - 1) / 2));
	if(NULL == pSkill)
	{
		return;
	}

	//	判断生活技能的等级
	int nSmeltNeedLevel = GETITEMATB(pItems[0], Hide);
	int nCurrentLevel = 0;
	int nMaterialType = GETITEMATB(pItems[0], Curse);

	if(nMaterialType == kMaterial_Ore)
	{
		nCurrentLevel = m_stExtAttrib.uSmeltOreLevel;
	}
	else if(nMaterialType == kMaterial_Wood)
	{
		nCurrentLevel = m_stExtAttrib.uSmeltWoodLevel;
	}
	else if(nMaterialType == kMaterial_Cloth)
	{
		nCurrentLevel = m_stExtAttrib.uSmeltClothLevel;
	}
	else if(nMaterialType == kMaterial_Gem)
	{
		nCurrentLevel = m_stExtAttrib.uSmeltGemLevel;
	}
	else
	{
		return;
	}

	if(nCurrentLevel < 0)
	{
		return;
	}

	if(nCurrentLevel < nSmeltNeedLevel)
	{
		return;
	}

	//	金币
	int nSmeltNeedMoney = 0;
	for(int i = 0; i < 5; ++i)
	{
		if(NULL == pItems[i])
		{
			continue;
		}

		nSmeltNeedMoney += GETITEMATB(pItems[i], Hide) * 3000;
	}
	if(GetMoney() < nSmeltNeedMoney)
	{
		return;
	}

	MinusMoney(nSmeltNeedMoney);

	//	满足条件了 可以合成了
	int nLevelOffset = nCurrentLevel - nSmeltNeedLevel;
	static float s_ay_fSmeltPercent[] =
	{
		0.4f,
		0.6f,
		0.8f
	};
	float fPercent = 1.0f;
	if(nLevelOffset < sizeof(s_ay_fSmeltPercent) / sizeof(s_ay_fSmeltPercent[0]))
	{
		fPercent = s_ay_fSmeltPercent[nLevelOffset];
	}

	int nTotalQuality = 0;
	for(int i = 0; i < 5; ++i)
	{
		ItemAttrib* pItem = pItems[i];
		if(NULL == pItem)
		{
			continue;
		}

		if(GETITEMATB(pItem, Type) == ITEM_NO)
		{
			continue;
		}

		nTotalQuality += GETITEMATB(pItem, Lucky);

		RemoveItem(pItem->tag);
		pItems[i] = NULL;
	}
	nTotalQuality *= fPercent;

	//	add item
	int nDestItemTag = AddItemReturnTag(nDestItemId);
	if(0 != nDestItemTag)
	{
		ItemAttrib* pDestItem = GetItemByTag(nDestItemTag);
		if(NULL != pDestItem)
		{
			SETITEMATB(pDestItem, Lucky, nTotalQuality);

			PkgPlayerUpdateItemNtf ppuin;
			ppuin.uTargetId = GetID();
			ppuin.stItem = *pDestItem;
			ObjectValid::DecryptAttrib(&ppuin.stItem);
			SendPacket(ppuin);
		}
	}

	//	增加对应的经验
	unsigned short* pExp = NULL;
	unsigned char* pLevel = NULL;
	if(nMaterialType == kMaterial_Ore)
	{
		pExp = &m_stExtAttrib.uSmeltOreExp;
		pLevel = &m_stExtAttrib.uSmeltOreLevel;
	}
	else if(nMaterialType == kMaterial_Wood)
	{
		pExp = &m_stExtAttrib.uSmeltWoodExp;
		pLevel = &m_stExtAttrib.uSmeltWoodLevel;
	}
	else if(nMaterialType == kMaterial_Cloth)
	{
		pExp = &m_stExtAttrib.uSmeltClothExp;
		pLevel = &m_stExtAttrib.uSmeltClothLevel;
	}
	else if(nMaterialType == kMaterial_Gem)
	{
		pExp = &m_stExtAttrib.uSmeltGemExp;
		pLevel = &m_stExtAttrib.uSmeltGemLevel;
	}

	if(NULL != pExp &&
		NULL != pLevel)
	{
		int nAddExp = nSmeltNeedLevel;
#ifdef _DEBUG
		nAddExp *= 50;
#endif
		if(*pLevel >= pSkill->nMaxLevel)
		{
			//	nothing
			*pExp = 0;
		}
		else
		{
			*pExp += nAddExp;

			if(*pExp > pSkill->uExp[*pLevel - 1])
			{
				++(*pLevel);
				*pExp = 0;
			}
		}

		PkgPlayerExtendAttribNot pean;
		pean.uTargetId = GetID();
		GetHeroExtendAttrib(pean.xAttrib);
		SendPacket(pean);
	}

	//	ok
	PkgPlayerSmeltMaterialsRsp rsp;
	rsp.uTargetId = GetID();
	rsp.nCode = 0;
	SendPacket(rsp);
}

void HeroObject::DoPacket(const PkgPlayerHandMakeItemReq &req)
{
	const MakeEquipInfo* pInfo = StoveManager::GetInstance()->GetMakeEquipInfo(req.nItemId);
	if(NULL == pInfo)
	{
		return;
	}

	//	查看材料是否够
	for(int i = 0; i < pInfo->nMaterialCount; ++i)
	{
		int nMaterialId = pInfo->nMaterialsId[i];
		int nCount = CountItem(nMaterialId);

		if(nCount < pInfo->nMaterialsCount[i])
		{
			return;
		}
	}

	//	金币是否够
	if(GetMoney() < pInfo->nNeedMoney)
	{
		return;
	}

	//	技能是否够
	if(m_stExtAttrib.uMakeEquipLevel < pInfo->nNeedLevel)
	{
		return;
	}

	int nTotalQuality = 0;
	int nCurrentQuality = 0;
	int nMaterialItemCount = 0;

	//	清除物品
	for(int i = 0; i < pInfo->nMaterialCount; ++i)
	{
		int nMaterialId = pInfo->nMaterialsId[i];
		int nMaterialCount = pInfo->nMaterialsCount[i];

		for(int j = 0; j < nMaterialCount; ++j)
		{
			ItemAttrib* pItemRemove = Lua_GetItemByAttribID(nMaterialId);

			if(NULL == pItemRemove)
			{
				LOG(ERROR) << "Can't get material:" << nMaterialId;
				return;
			}

			if(GETITEMATB(pItemRemove, Type) == ITEM_OTHER &&
				GETITEMATB(pItemRemove, Curse) >= 1 &&
				GETITEMATB(pItemRemove, Curse) <= 8)
			{
				nTotalQuality += 50;
				nCurrentQuality += GETITEMATB(pItemRemove, Lucky);
				++nMaterialItemCount;
			}

			if(!RemoveItem(pItemRemove->tag))
			{
				LOG(ERROR) << "Can't remove item";
				return;
			}
		}
	}
	MinusMoney(pInfo->nNeedMoney);

	//	添加装备
	int nTag = 0;
	if(CMainServer::GetInstance()->GetServerMode() == GM_NORMAL)
	{
		nTag = AddItemReturnTag(req.nItemId);
	}
	else
	{
		nTag = AddItemNoBindReturnTag(req.nItemId);
	}

	//	根据材料决定是否要升级该装备
	ItemAttrib* pItem = GetItemByTag(nTag);
	if(NULL == pItem)
	{
		return;
	}

	/*int nRandom = rand() % (nTotalQuality * 3);
	if(nCurrentQuality >= nRandom)
	{
		ObjectValid::DecryptAttrib(pItem);
		lua_State* pState = GameWorld::GetInstance().GetLuaState();
		lua_getglobal(pState, "upgradeItem");
		if(lua_isfunction(pState, -1))
		{
			tolua_pushusertype(pState, pItem, "ItemAttrib");
			lua_pushnumber(pState, 500);
			lua_pushnumber(pState, 500);

			if(0 != lua_pcall(pState, 3, 0, 0))
			{
				LOG(ERROR) << "Lua error : " << lua_tostring(pState, -1);
				lua_pop(pState, 1);
			}
		}
		PkgPlayerUpdateItemNtf ppuin;
		ppuin.uTargetId = GetID();
		ppuin.stItem = *pItem;
		SendPacket(ppuin);

		ObjectValid::EncryptAttrib(pItem);
	}*/
	//	根据品质确定装备的星级
	int nUpgradeNumber = 0;
	if(nCurrentQuality > nMaterialItemCount * 50)
	{
		//	每一块70品质以上
		nUpgradeNumber = 5;
	}
	else if(nCurrentQuality > nMaterialItemCount * 40)
	{
		nUpgradeNumber = 4;
	}
	else if(nCurrentQuality > nMaterialItemCount * 25)
	{
		nUpgradeNumber = 3;
	}
	else if(nCurrentQuality > nMaterialItemCount * 20)
	{
		nUpgradeNumber = 2;
	}
	else if(nCurrentQuality > nMaterialItemCount * 15)
	{
		nUpgradeNumber = 1;
	}

	if(0 != nUpgradeNumber)
	{
		ObjectValid::DecryptAttrib(pItem);
		lua_State* pState = GameWorld::GetInstance().GetLuaState();
		lua_getglobal(pState, "mustUpgradeItem");
		if(lua_isfunction(pState, -1))
		{
			tolua_pushusertype(pState, pItem, "ItemAttrib");
			lua_pushnumber(pState, nUpgradeNumber);

			if(0 != lua_pcall(pState, 2, 0, 0))
			{
				LOG(ERROR) << "Lua error : " << lua_tostring(pState, -1);
				lua_pop(pState, 1);
			}
			else
			{
				PkgPlayerUpdateItemNtf ppuin;
				ppuin.uTargetId = GetID();
				ppuin.stItem = *pItem;
				SendPacket(ppuin);
			}
		}

		ObjectValid::EncryptAttrib(pItem);
	}

	//	升级经验
	const LifeSkillInfo* pSkill = StoveManager::GetInstance()->GetLifeSkillInfo(kLifeSkill_SmeltEquip);
	int nAddExp = pInfo->nExp;
#ifdef _DEBUG
	nAddExp *= 10;
#endif
	if(m_stExtAttrib.uMakeEquipLevel >= pSkill->nMaxLevel)
	{
		//	nothing
		m_stExtAttrib.uMakeEquipExp = 0;
	}
	else
	{
		m_stExtAttrib.uMakeEquipExp += nAddExp;

		if(m_stExtAttrib.uMakeEquipExp > pSkill->uExp[m_stExtAttrib.uMakeEquipLevel - 1])
		{
			++(m_stExtAttrib.uMakeEquipLevel);
			m_stExtAttrib.uMakeEquipExp = 0;
		}
	}

	PkgPlayerExtendAttribNot pean;
	pean.uTargetId = GetID();
	GetHeroExtendAttrib(pean.xAttrib);
	SendPacket(pean);
}

void HeroObject::DoPacket(const PkgPlayerOpenPotentialReq& req)
{
	ItemAttrib* pItem = GetItemByTag(req.nItemId);
	ItemAttrib* pStone = GetItemByTag(req.nStoneId);

	if(NULL == pItem ||
		NULL == pStone)
	{
		return;
	}

	//	装备是否合法
	if(!IsEquipItem(GETITEMATB(pItem, Type)))
	{
		return;
	}

	DWORD dwMPDesc = GETITEMATB(pItem, MP);
	if(dwMPDesc == 0)
	{
		return;
	}

	//	价格
	int nMoney = PotentialAttribHelper::GetPotentialOffset(dwMPDesc) * 50000;
	if(GetMoney() < nMoney)
	{
		return;
	}

	//	升级次数
	int nPotentialTime = PotentialAttribHelper::GetPotentialTime(dwMPDesc);
	if(nPotentialTime >= 5)
	{
		return;
	}

	//	确定最小的升级等级
	int nMinIndex = PotentialAttribHelper::GetPotentialOffset(dwMPDesc);
	if(GETITEMATB(pStone, Hide) < nMinIndex)
	{
		nMinIndex = GETITEMATB(pStone, Hide);
	}

	//	清除石头
	ClearItem(GETITEMATB(pStone, ID), 1);
	MinusMoney(nMoney);

	//	修改装备潜能索引
	const StoveAttribVector& refAttribs = StoveManager::GetInstance()->GetStoveAttribVector();
	std::vector<const StoveAttribInfo*> xAvailabelAttribs;
	xAvailabelAttribs.resize(refAttribs.size(), NULL);
	int nAvailabelIndex = 0;

	for(int i = 0; i < refAttribs.size(); ++i)
	{
		const StoveAttribInfo& refInfo = refAttribs[i];

		if((((1 << GETITEMATB(pItem, Type)) & refInfo.dwActiveItemType) != 0) &&
			nMinIndex >= refInfo.nLevel)
		{
			xAvailabelAttribs[nAvailabelIndex++] = &refInfo;
		}
	}

	int nActiveIndex = rand() % nAvailabelIndex;
	const StoveAttribInfo* pActiveAttrib = xAvailabelAttribs[nActiveIndex];
	if(NULL != pActiveAttrib)
	{
		PotentialAttribHelper::SetPotentialIndex(dwMPDesc, pActiveAttrib->nAttribId + 1);
		PotentialAttribHelper::SetPotentialTime(dwMPDesc, nPotentialTime + 1);
		SETITEMATB(pItem, MP, dwMPDesc);

		//	绑定装备
		DWORD dwAtkPois = GETITEMATB(pItem, AtkPois);
		SET_FLAG(dwAtkPois, POIS_MASK_BIND);
		SETITEMATB(pItem, AtkPois, dwAtkPois);

		PkgPlayerUpdateItemNtf ppuin;
		ppuin.uTargetId = GetID();
		ppuin.stItem = *pItem;
		ObjectValid::DecryptAttrib(&ppuin.stItem);
		SendPacket(ppuin);

		PkgPlayerOpenPotentialRsp rsp;
		rsp.uTargetId = GetID();
		SendPacket(rsp);
	}
}

void HeroObject::DoPacket(const PkgPlayerChargeReq& req)
{
	PkgPlayerChargeRsp rsp;
	rsp.uTargetId = GetID();

	if(req.xOrderId.length() != 28 ||
		req.xOrderId.length() != 32)
	{
		rsp.nResult = -1;
		SendPacket(rsp);
		return;
	}

	//	提交给登录服务器
}

void HeroObject::DoPacket(const PkgPlayerWorldSayReq& req)
{
	int nItemCount = CountItem(1664);
	if(nItemCount <= 0)
	{
		SendQuickMessage(QMSG_NOENOUGHITEM, 1664);
		return;
	}

	ClearItem(1664, 1);

	PkgSystemWorldSayNot not;
	not.xName = GetName();
	not.xMsg = req.xMsg;
	not.uReserved = req.uReserved;
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << not;
	GameSceneManager::GetInstance()->BroadcastPacketAllScene(&g_xThreadBuffer);
}

void HeroObject::DoPacket(const PkgPlayerLoginExtDataReq &req)
{
	g_xConsole.CPrint("Received hum %s ext data %d length %d", GetName(), req.cIndex, req.xData.size());

	if (0 == req.cIndex)
	{
		m_bBigStoreReceived = true;
	}
	if(req.xData.empty())
	{
		return;
	}
#define MAX_SAVEDATA_SIZE (8 * 1024)
	char cExtIndex = 0;
	std::string xName;
	BYTE bCount = 0;

	const char* pData = &req.xData[0];
	DWORD dwDataLen = req.xData.size();

	static char* s_pBuf = new char[MAX_SAVEDATA_SIZE];
	uLongf buflen = MAX_SAVEDATA_SIZE;
	uLongf srclen = dwDataLen;
	int nRet = uncompress((Bytef*)s_pBuf, &buflen, (const Bytef*)pData, srclen);
	if(nRet == Z_OK)
	{
		g_xThreadBuffer.Reset();
		g_xThreadBuffer.Write(s_pBuf, buflen);
	}
	else
	{
		LOG(ERROR) << "Can't uncompress extend save data." << GetName();
		return;
	}

	try
	{
		g_xThreadBuffer >> cExtIndex;
		g_xThreadBuffer >> xName;
		g_xThreadBuffer >> bCount;

		if(xName != GetName())
		{
			LOG(ERROR) << "Invalid extend save data:" << cExtIndex << " with name:" << xName;
			return;
		}

		int nItemCount = 0;

		if(0 == cExtIndex)
		{
			if(0 != bCount)
			{
				ItemAttrib item = {0};
				//	clear all bigstore items
				for(int i = 0; i < sizeof(m_stBigStore) / sizeof(m_stBigStore[0]); ++i)
				{
					ItemAttrib& refItem = m_stBigStore[i];
					if(GETITEMATB(&refItem, Type) == ITEM_NO)
					{
						g_xThreadBuffer >> item;
						refItem = item;
						ObjectValid::EncryptAttrib(&refItem);
						++nItemCount;

						if(nItemCount >= bCount)
						{
							break;
						}
					}
				}
			}
			//	大仓库数据收到了 之后就可以存档大仓库数据了
			m_bBigStoreReceived = true;
		}
	}
	catch (std::exception e)
	{
		LOG(ERROR) << "Invalid ext save." << GetName();
		CMainServer::GetInstance()->GetEngine()->CompulsiveDisconnectUser(GetUserIndex());
	}
}

void HeroObject::DoPacket(const PkgPlayerDifficultyLevelReq &req)
{
	return;

	PkgPlayerDifficultyLevelRsp rsp;
	rsp.uTargetId = GetID();

	if(req.cLevel < kDifficultyDefault ||
		req.cLevel >= kDifficultyTotal ||
		CMainServer::GetInstance()->GetServerMode() == GM_LOGIN ||
		0 != GameWorld::GetInstance().GetDifficultyLevel())
	{
		rsp.nRet = -1;
		SendPacket(rsp);
		return;
	}

	GameWorld::GetInstance().SetDifficultyLevel(req.cLevel);

	PkgPlayerQuickMsgNtf ppqmn;
	ppqmn.uTargetId = GetID();
	ppqmn.nMsgID = QMSG_DIFFICULTYLEVEL;
	ppqmn.nParam = GameWorld::GetInstance().GetDifficultyLevel();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << ppqmn;
	GameSceneManager::GetInstance()->BroadcastPacketAllScene(&g_xThreadBuffer);
}

void HeroObject::DoPacket(const PkgPlayerQuitSelChrReq& req)
{
	//	小退选择人物
	NetThreadEvent evt;
	evt.nEventId = kNetThreadEvent_SmallQuit;
	evt.nArg = GetUserIndex();
	CMainServer::GetInstance()->SendNetThreadEvent(evt);
}