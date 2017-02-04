#include "netbase.h"
#include "ObjectEngine.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include "../CMainServer/CMainServer.h"
#include "../GameWorld/GameSceneManager.h"
#include "../GameWorld/MonsterObject.h"
#include "ObjectValid.h"
#include "GameWorld.h"
#include <process.h>
#include "ObjectStatus.h"
#include "../GameWorld/GameInstanceScene.h"
#include "../../CommonModule/SettingLoader.h"
#include "../../CommonModule/SimpleActionHelper.h"
#include <google/protobuf/message.h>
//////////////////////////////////////////////////////////////////////////
static const int s_nMoveOft[] =
{
	0,-1,
	1,-1,
	1,0,
	1,1,
	0,1,
	-1,1,
	-1,0,
	-1,-1
};

const int g_nMoveOft[] =
{
	0,-1,
	1,-1,
	1,0,
	1,1,
	0,1,
	-1,1,
	-1,0,
	-1,-1
};
//////////////////////////////////////////////////////////////////////////
ByteBuffer g_xThreadBuffer(30240);
ConsoleHelper g_xConsole;
//////////////////////////////////////////////////////////////////////////
void ConsolePrint(const char* _pszText)
{
	g_xConsole.CPrint(_pszText);
}

unsigned int SendBuffer(unsigned int _nIdx, ByteBuffer* _pBuf)
{
	CNetbase* pNet = CMainServer::GetInstance()->GetEngine();
	if(NULL == pNet)
	{
		return 0;
	}

	if(_pBuf->GetLength() == 0)
	{
		return 0;
	}

	DWORD dwPacketLength = _pBuf->GetLength();
	unsigned char* pBuf = const_cast<unsigned char*>(_pBuf->GetBuffer());
	*(DWORD*)pBuf = dwPacketLength;

	if(TRUE == pNet->SendToUser(_nIdx, (char*)_pBuf->GetBuffer() + sizeof(unsigned int), dwPacketLength - 4, 0))
	{
#ifdef _VIEW_PACKET
		LOG(INFO) << _pBuf->ToHexString() << "sended";
#endif
		return dwPacketLength;
	}
	return 0;
}

unsigned int SendBufferToServer(unsigned int _nIdx, ByteBuffer* _pBuf)
{
	CNetbase* pNet = CMainServer::GetInstance()->GetEngine();
	if(NULL == pNet)
	{
		return 0;
	}

	if(_pBuf->GetLength() == 0)
	{
		return 0;
	}

	DWORD dwPacketLength = _pBuf->GetLength();
	unsigned char* pBuf = const_cast<unsigned char*>(_pBuf->GetBuffer());
	*(DWORD*)pBuf = dwPacketLength;

	if(TRUE == pNet->SendToServer(_nIdx, (char*)_pBuf->GetBuffer() + sizeof(unsigned int), dwPacketLength - 4, 0))
	{
#ifdef _VIEW_PACKET
		LOG(INFO) << _pBuf->ToHexString() << "sended";
#endif
		return dwPacketLength;
	}
	return 0;
}

unsigned int SendProtoToServer(unsigned int _nIdx, int _nCode, google::protobuf::Message& _refMsg)
{
	CNetbase* pNet = CMainServer::GetInstance()->GetEngine();
	if(NULL == pNet)
	{
		return 0;
	}

	static char s_bytesBuffer[0xff];
	//	write code
	memcpy(s_bytesBuffer, &_nCode, sizeof(int));

	int nSize = _refMsg.ByteSize();
	if (0 == nSize)
	{
		return 0;
	}
	if (nSize > sizeof(s_bytesBuffer))
	{
		g_xConsole.CPrint("Byte buffer overflow : %d, size %d", _nCode, nSize);
		return 0;
	}

	if (!_refMsg.SerializeToArray(s_bytesBuffer + sizeof(int), sizeof(s_bytesBuffer) - sizeof(int)))
	{
		g_xConsole.CPrint("Serialize protobuf failed");
		return 0;
	}

	if (TRUE == pNet->SendToServer(_nIdx, (char*)s_bytesBuffer, 4 + nSize, 0))
	{
		return 4 + nSize;
	}

	return 0;
}

void MirLog(const char* _pLog)
{
	LOG(INFO) << _pLog;
}
//////////////////////////////////////////////////////////////////////////

GameObject::GameObject(/*DWORD _dwID*/) : /*CUser(_dwID)
	, */m_xMsgQueue(OBJECT_MSGQUEUE_SIZE)
{
	m_dwTotalExeTime = 0;
	m_dwLastExeTime = 0;
//	m_bGarbage = false;
	m_eType = SOT_NONE;

	m_dwLastAttackTime = 0;
	m_dwLastDeathTime = 0;
	m_dwLastMoveTime = 0;
	m_dwLastAddHpMpTime = 0;
	m_dwDefectTime = 0;

	m_bStoneRestore = true;

//	m_wMapId = 0;
//	m_eState = US_NOTHING;

	m_stData.Reset();

	//memset(&m_ptPosition, 0, sizeof(m_ptPosition));
	for(int i = 0; i < HP_SUPPLY_NUMBER + MP_SUPPLY_NUMBER; ++i)
	{
		m_dwSupply[i] = 0;
	}
	ZeroMemory(m_dwHealSupply, sizeof(m_dwHealSupply));
	ZeroMemory(m_dwHumEffTime, sizeof(m_dwHumEffTime));
	ZeroMemory(m_dwEnergyShieldSupply, sizeof(m_dwEnergyShieldSupply));
	m_dwLastIncHPTime = m_dwLastIncMPTime = 0;
	m_dwLastHealIncHPTime = 0;
	m_dwHumEffFlag = 0;

	m_dwACInrease = 0;
	m_dwPoison = 0;

	m_pValid = new ObjectValid;
	m_pValid->Attach(this);

	m_pStatusCtrl = new ObjectStatusControl(this);

	m_dwInvalidMsgQueueTimes = 0;
	m_bNetDataValid = true;
	//m_bEncrypt = false;
	m_nTotalRecvDamage = 0;
	m_bCanPosion = true;
	m_dwHPRecoverInterval = 40 * 1000;
}

GameObject::~GameObject()
{
	SAFE_DELETE(m_pValid);
	ClearDelayAction();
}


//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* void AddProcess(const PacketHeader* _pPkt)
/************************************************************************/
void GameObject::AddProcess(const PacketHeader* _pPkt)
{
	DWORD dwLength = _pPkt->uLen;
	if(0 == m_xMsgQueue.Write(_pPkt, dwLength))
	{
		LOG(FATAL) << "Can't write pkg to MsgQueue!";
	}
}

bool GameObject::AddProcess(ByteBuffer& _xBuf)
{
	if(0 == m_xMsgQueue.Write(_xBuf.GetHead(), _xBuf.GetLength()))
	{
		LOG(ERROR) << "Can't write pkg to MsgQueue!";
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//	DispatchPacket
bool GameObject::DispatchPacket(ByteBuffer& _refBuf)
{
	PacketHeader* pHeader = (PacketHeader*)_refBuf.GetHead();
	return DispatchPacket(_refBuf, pHeader);
}

bool GameObject::DispatchPacket(ByteBuffer& _refBuffer, const PacketHeader *_pHeader) {
	BEGIN_PROCESS_PACKET(_pHeader)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_USER_ACTION_REQ,		PkgUserActionReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_SERVERSTATUS_REQ,		PkgServerStatusReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_SAY_REQ,			PkgPlayerSayReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_CHANGEEQUIPMENT_REQ,	PkgPlayerChangeEquipReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_DROPITEM_REQ,	PkgPlayerDropItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_PICKUPITEM_REQ,	PkgPlayerPickUpItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_UNDRESSITEM_REQ,	PkgPlayerUndressItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_DRESSITEM_REQ,	PkgPlayerDressItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_USEITEM_REQ,		PkgPlayerUseItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_LOADED,					PkgGameLoadedAck,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_CLICKNPC_REQ,	PkgPlayerClickNPCReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_ATTACK_REQ,		PkgPlayerAttackReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_SYNCASSIST_REQ,	PkgPlayerSyncAssistReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_SHOPOP_REQ,		PkgPlayerShopOpReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_USERDATA_REQ,	PkgPlayerUserDataReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_MONSINFO_REQ,	PkgPlayerMonsInfoReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_REVIVE_REQ,		PkgPlayerReviveReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_SLAVE_STOP_REQ,	PkgPlayerSlaveStopReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_CUBEITEMS_REQ,	PkgPlayerCubeItemsReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_CALLSLAVE_REQ,	PkgPlayerCallSlaveReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_KILLSLAVE_REQ,	PkgPlayerKillSlaveReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_SPEOPERATE_REQ,	PkgPlayerSpeOperateReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_MERGYCOSTITEM_REQ,	PkgPlayerMergyCostItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_NETDELAY_REQ,	PkgPlayerNetDelayReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_GM_NOTIFICATION_REQ,	PkgGmNotificationReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_DECOMPOSE_REQ,	PkgPlayerDecomposeReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_FORGEITEM_REQ,	PkgPlayerForgeItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_SPLITITEM_REQ,	PkgPlayerSplitItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_OFFSELLITEM_REQ, PkgPlayerOffSellItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_OFFBUYITEM_REQ,	PkgPlayerOffBuyItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_OFFGETLIST_REQ,	PkgPlayerOffGetListReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_OFFCHECKSOLD_REQ,PkgPlayerOffCheckSoldReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_PRIVATECHAT_REQ, PkgPlayerPrivateChatReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_OFFTAKEBACK_REQ, PkgPlayerOffTakeBackReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_IDENTIFYITEM_REQ,PkgPlayerIdentifyItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_UNBINDITEM_REQ,	PkgPlayerUnbindItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_SERVERDELAY_ACK, PkgPlayerServerDelayAck,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_RANKLIST_REQ,	PkgPlayerRankListReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_GETOLSHOPLIST_REQ, PkgPlayerGetOlShopListReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_BUYOLSHOPITEM_REQ, PkgPlayerBuyOlShopItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_SMELTMATERIALS_REQ,PkgPlayerSmeltMaterialsReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_HAND_MAKE_ITEM_REQ,PkgPlayerHandMakeItemReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_OPEN_POTENTIAL_REQ, PkgPlayerOpenPotentialReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_CHARGE_REQ,		PkgPlayerChargeReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_WORLD_SAY_REQ,	PkgPlayerWorldSayReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_LOGINEXTDATA_REQ,PkgPlayerLoginExtDataReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_DIFFICULTYLEVEL_REQ, PkgPlayerDifficultyLevelReq,				_refBuffer)
		PROCESS_PACKET_WITH_BUF(PKG_GAME_PLAYER_QUITSELCHR_REQ, PkgPlayerQuitSelChrReq,				_refBuffer)
		//PROCESS_DEFAULT()
			default:
	{
		_refBuffer.SetHeadOffset(_pHeader->uLen);
		LOG(ERROR) << "Unknown opcode[" << _pHeader->uOp << "]";
		++m_dwInvalidMsgQueueTimes;
		if(m_dwInvalidMsgQueueTimes > 100)
		{
			m_bNetDataValid = false;
			LOG(ERROR) << "Crashed while processing MsgQueue | Content:" << _refBuffer.ToHexString();
			LOG(ERROR) << "packet[" << _pHeader->uOp << "] len[" << _pHeader->uLen;
			if(GetType() == SOT_HERO)
			{
				HeroObject* pHero = static_cast<HeroObject*>(this);
				char szName[20] = {0};
				ObjectValid::GetItemName(&pHero->GetUserData()->stAttrib, szName);
				LOG(ERROR) << "!!!Name[" << szName << "]";
			}
		}
	}break;
	END_PROCESS_PACKET();

	return m_bNetDataValid;
}

/************************************************************************/
/* virtual void DoMsgQueue(unsigned int _dwTick)
/************************************************************************/
bool GameObject::DoMsgQueue(unsigned int _dwTick)
{
	if(!m_bNetDataValid)
	{
		return false;
	}

	Lock();

	bool bProcessed = false;
	const PacketHeader* pHeader = NULL;
	m_dwInvalidMsgQueueTimes = 0;
	bool bTerminate = false;

	try
	{
		while(m_xMsgQueue.GetLength())
		{
			bProcessed = true;
			pHeader = (PacketHeader*)m_xMsgQueue.GetHead();

			if(pHeader->uOp == PKG_GAME_PLAYER_SHOPOP_REQ &&
				pHeader->uLen == 21)
			{
				m_xMsgQueue.SetHeadOffset(21);
				continue;
			}

			//LOG(INFO) << "处理ID[" << GetID() << "]的数据包[" << pHeader->uOp <<"]"
			//<< " 所在地图[" << m_stData.wMapID << "]";
			DispatchPacket(m_xMsgQueue, pHeader);
		}
	}
	catch(std::exception exp)
	{
		LOG(ERROR) << "Crashed while processing MsgQueue | Content:" << m_xMsgQueue.ToHexString();
		LOG(ERROR) << "packet[" << pHeader->uOp << "] len[" << pHeader->uLen << "]";
		if(GetType() == SOT_HERO)
		{
			HeroObject* pHero = static_cast<HeroObject*>(this);
			LOG(ERROR) << "Player[" << pHero->GetName() << " abnormal.Invalid packet, disconnect.";
			CMainServer::GetInstance()->ForceCloseConnection(pHero->GetUserIndex());
		}
	}
	catch(...)
	{
		LOG(ERROR) << "Fatal error occurred when processing packet[" << pHeader->uOp << "]";
	}
	
	m_xMsgQueue.Reset();

	Unlock();
	return bProcessed;
}
/************************************************************************/
/* virtual void ProcessPacket(PacketHeader* _pPkt)
/************************************************************************/
void GameObject::ProcessPacket(PacketHeader* _pPkt)
{
	switch(_pPkt->uOp)
	{
	default:
		{

		}break;
	}
}

//////////////////////////////////////////////////////////////////////////
GameScene* GameObject::GetLocateScene()
{
	return GameSceneManager::GetInstance()->GetScene(GetMapID());
}

//////////////////////////////////////////////////////////////////////////
bool GameObject::IncHP(DWORD _dwHP)
{
	if(GetObject_HP() + _dwHP > GetObject_MaxHP())
	{
		SetObject_HP(GetObject_MaxHP());
		return false;
	}
	else
	{
		SetObject_HP(GetObject_HP() + _dwHP);
		return true;
	}
}

bool GameObject::IncMP(DWORD _dwMP)
{
	if(GetObject_MP() + _dwMP > GetObject_MaxMP())
	{
		SetObject_MP(GetObject_MaxMP());
		return false;
	}
	else
	{
		SetObject_MP(GetObject_MP() + _dwMP);
		return true;
	}
}

bool GameObject::DecHP(DWORD _dwHP)
{
	if(_dwHP >= GetObject_HP())
	{
		SetObject_HP(0);
		return false;
	}
	else
	{
		SetObject_HP(GetObject_HP() - _dwHP);
		return true;
	}
}

bool GameObject::DecMP(DWORD _dwMP)
{
	if(_dwMP > GetObject_MP())
	{
		SetObject_MP(0);
		return false;
	}
	else
	{
		SetObject_MP(GetObject_MP() - _dwMP);
		return true;
	}
}

void GameObject::SyncHP(HeroObject* _pHero)
{
	PkgPlayerUpdateAttribNtf ntf;
	ntf.uTargetId = GetID();
	ntf.bType = UPDATE_HP;
	ntf.dwParam = GetObject_HP();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << ntf;
	_pHero->SendPlayerBuffer(g_xThreadBuffer);
}

void GameObject::SyncMP(HeroObject *_pHero)
{
	PkgPlayerUpdateAttribNtf ntf;
	ntf.uTargetId = GetID();
	ntf.bType = UPDATE_MP;
	ntf.dwParam = GetObject_MP();
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << ntf;
	_pHero->SendPlayerBuffer(g_xThreadBuffer);
}
//////////////////////////////////////////////////////////////////////////
void GameObject::ResetSupply()
{
	/*for(int i = 0; i < HP_SUPPLY_NUMBER + MP_SUPPLY_NUMBER; ++i)
	{
		m_dwSupply[i] = 0;
	}*/
	ZeroMemory(m_dwSupply, sizeof(m_dwSupply));
	ZeroMemory(m_dwHealSupply, sizeof(m_dwHealSupply));
	ZeroMemory(m_dwEnergyShieldSupply, sizeof(m_dwEnergyShieldSupply));
}
//////////////////////////////////////////////////////////////////////////
bool GameObject::AddDrugState(int _total, int _type)
{
	if(_type == 0)
	{
		for(int i = 0; i < HP_SUPPLY_NUMBER; ++i)
		{
			if(m_dwSupply[i] == 0)
			{
				m_dwSupply[i] = MAKELONG(_total, _total / 5);
				return true;
			}
		}
	}
	else if(_type == 1)
	{
		for(int i = HP_SUPPLY_NUMBER; i < HP_SUPPLY_NUMBER + MP_SUPPLY_NUMBER; ++i)
		{
			if(m_dwSupply[i] == 0)
			{
				m_dwSupply[i] = MAKELONG(_total, _total / 5);
				return true;
			}
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GameObject::AddHealState(int _total)
{
	for(int i = 0; i < HP_SUPPLY_NUMBER; ++i)
	{
		if(m_dwHealSupply[i] == 0)
		{
			m_dwHealSupply[i] = MAKELONG(_total, _total / 5);
			return true;
		}
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GameObject::AddEnergyShieldState(int _total, int _step)
{
	for(int i = 0; i < HP_SUPPLY_NUMBER; ++i)
	{
		if(m_dwEnergyShieldSupply[i] == 0)
		{
			m_dwEnergyShieldSupply[i] = MAKELONG(_total, _total / _step);
			return true;
		}
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
void GameObject::UpdateStatus(DWORD _dwCurTick)
{
	PkgPlayerUpdateAttribNtf ntf;
	ntf.uTargetId = GetID();
	GameScene* pScene = GameSceneManager::GetInstance()->GetScene(m_stData.wMapID);
	DWORD dwAddInterval = 0;

	m_pStatusCtrl->Update();
	m_xStates.Update();

	if(m_stData.eGameState == OS_DEAD)
	{
		ResetSupply();
		ResetEffStatus(MMASK_HIDE);
		ResetEffStatus(MMASK_SHIELD);
		ResetEffStatus(MMASK_LVDU);
		ResetEffStatus(MMASK_CHARMAC);
		ZeroMemory(m_dwHumEffTime, sizeof(m_dwHumEffTime));
		return;
	}

	dwAddInterval = INTERVAL_INC_HP;
	for(int i = 0; i < HP_SUPPLY_NUMBER + MP_SUPPLY_NUMBER; ++i)
	{
		if(m_dwSupply[i] != 0)
		{
			WORD wStep = HIWORD(m_dwSupply[i]);
			WORD wLeft = LOWORD(m_dwSupply[i]);
			WORD wChg = 0;
			bool bIsHP = (i >= HP_SUPPLY_NUMBER ? false : true);

			if(wStep > 0 &&
				wLeft > 0)
			{
				if(bIsHP)
				{
					if(_dwCurTick - m_dwLastIncHPTime >= dwAddInterval)
					{
						if(wLeft <= wStep)
						{
							wChg = wLeft;
							m_dwSupply[i] = 0;
						}
						else
						{
							wChg = wStep;
							wLeft -= wStep;
							m_dwSupply[i] = MAKELONG(wLeft, wStep);
						}
						if(!IncHP(wChg))
						{
							//	Stop because the hp if full-filled
							m_dwSupply[i] = 0;
						}
						else
						{
							//	normally
						}
						m_pValid->IncHP(wChg);

						m_dwLastIncHPTime = _dwCurTick;
						//ntf.dwParam = MAKELONG(UPDATE_HP, m_stData.stAttrib.HP);
						ntf.bType = UPDATE_HP;
						ntf.dwParam = GetObject_HP();
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						//SendBuffer(GetUserIndex(), &g_xThreadBuffer);
						//	增加不发包
						if(SettingLoader::GetInstance()->GetIntValue("DISABLEVALUENOTIFY") == 1)
						{
							//	nothing
						}
						else
						{
							pScene->BroadcastPacket(&g_xThreadBuffer, GetID());
						}
						//pScene->BroadcastPacket(&g_xThreadBuffer);
						if(GetType() == SOT_HERO)
						{
							static_cast<HeroObject*>(this)->SendPacket(ntf);
						}
					}
				}
				else
				{
					if(_dwCurTick - m_dwLastIncMPTime >= dwAddInterval)
					{
						if(wLeft <= wStep)
						{
							wChg = wLeft;
							m_dwSupply[i] = 0;
						}
						else
						{
							wChg = wStep;
							wLeft -= wStep;
							m_dwSupply[i] = MAKELONG(wLeft, wStep);
						}
						if(!IncMP(wChg))
						{
							m_dwSupply[i] = 0;
						}
						else
						{
							//
						}
						m_pValid->IncMP(wChg);

						m_dwLastIncMPTime = _dwCurTick;
						ntf.bType = UPDATE_MP;
						ntf.dwParam = GetObject_MP();
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						//	增加不发包
						//pScene->BroadcastPacket(&g_xThreadBuffer);
						if(SettingLoader::GetInstance()->GetIntValue("DISABLEVALUENOTIFY") == 1)
						{
							//	nothing
						}
						else
						{
							pScene->BroadcastPacket(&g_xThreadBuffer, GetID());
						}

						if(GetType() == SOT_HERO)
						{
							static_cast<HeroObject*>(this)->SendPacket(ntf);
						}
					}
				}
			}
			else
			{
				m_dwSupply[i] = 0;
			}
		}
	}

	for(int i = 0; i < HP_SUPPLY_NUMBER; ++i)
	{
		if(m_dwHealSupply[i] != 0)
		{
			WORD wStep = HIWORD(m_dwHealSupply[i]);
			WORD wLeft = LOWORD(m_dwHealSupply[i]);
			WORD wChg = 0;

			if(wStep > 0 &&
				wLeft > 0)
			{
				if(_dwCurTick - m_dwLastHealIncHPTime >= dwAddInterval)
				{
					if(wLeft <= wStep)
					{
						wChg = wLeft;
						m_dwHealSupply[i] = 0;
					}
					else
					{
						wChg = wStep;
						wLeft -= wStep;
						m_dwHealSupply[i] = MAKELONG(wLeft, wStep);
					}
					if(!IncHP(wChg))
					{
						//	Stop because the hp if full-filled
						m_dwHealSupply[i] = 0;
					}
					else
					{
						//	normally
					}
					m_pValid->IncHP(wChg);

					m_dwLastHealIncHPTime = _dwCurTick;
					//ntf.dwParam = MAKELONG(UPDATE_HP, m_stData.stAttrib.HP);
					ntf.bType = UPDATE_HP;
					ntf.dwParam = GetObject_HP();
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					//SendBuffer(GetUserIndex(), &g_xThreadBuffer);
					//	增加不发包
					//pScene->BroadcastPacket(&g_xThreadBuffer);
					if(SettingLoader::GetInstance()->GetIntValue("DISABLEVALUENOTIFY") == 1)
					{
						//	nothing
					}
					else
					{
						pScene->BroadcastPacket(&g_xThreadBuffer, GetID());
					}

					if(GetType() == SOT_HERO)
					{
						static_cast<HeroObject*>(this)->SendPacket(ntf);
					}
				}
			}
			else
			{
				m_dwHealSupply[i] = 0;
			}
		}
	}

	for(int i = 0; i < HP_SUPPLY_NUMBER; ++i)
	{
		if(m_dwEnergyShieldSupply[i] != 0)
		{
			WORD wStep = HIWORD(m_dwEnergyShieldSupply[i]);
			WORD wLeft = LOWORD(m_dwEnergyShieldSupply[i]);
			WORD wChg = 0;

			if(wStep > 0 &&
				wLeft > 0)
			{
				if(_dwCurTick - m_dwLastHealIncHPTime >= dwAddInterval)
				{
					if(wLeft <= wStep)
					{
						wChg = wLeft;
						m_dwEnergyShieldSupply[i] = 0;
					}
					else
					{
						wChg = wStep;
						wLeft -= wStep;
						m_dwEnergyShieldSupply[i] = MAKELONG(wLeft, wStep);
					}
					if(!IncHP(wChg))
					{
						//	Stop because the hp if full-filled
						m_dwEnergyShieldSupply[i] = 0;
					}
					else
					{
						//	normally
					}
					m_pValid->IncHP(wChg);

					m_dwLastHealIncHPTime = _dwCurTick;
					//ntf.dwParam = MAKELONG(UPDATE_HP, m_stData.stAttrib.HP);
					ntf.bType = UPDATE_HP;
					ntf.dwParam = GetObject_HP();
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					//SendBuffer(GetUserIndex(), &g_xThreadBuffer);
					//	增加不发包
					//pScene->BroadcastPacket(&g_xThreadBuffer);
					if(SettingLoader::GetInstance()->GetIntValue("DISABLEVALUENOTIFY") == 1)
					{
						//	nothing
					}
					else
					{
						pScene->BroadcastPacket(&g_xThreadBuffer, GetID());
					}

					if(GetType() == SOT_HERO)
					{
						static_cast<HeroObject*>(this)->SendPacket(ntf);
					}
				}
			}
			else
			{
				m_dwEnergyShieldSupply[i] = 0;
			}
		}
	}

	//	if hp is full
	if(GetObject_HP() >= GetObject_MaxHP())
	{
		ZeroMemory(m_dwHealSupply, sizeof(m_dwHealSupply));
		ZeroMemory(m_dwEnergyShieldSupply, sizeof(m_dwEnergyShieldSupply));
		for(int i = 0; i < HP_SUPPLY_NUMBER; ++i)
		{
			m_dwSupply[i] = 0;
		}
	}

	//	detect whether the CharmAC is out of date
	if(TEST_FLAG(m_dwHumEffFlag, MMASK_CHARMAC))
	{
		if(_dwCurTick > m_dwHumEffTime[MMASK_CHARMAC_INDEX])
		{
			RESET_FLAG(m_dwHumEffFlag, MMASK_CHARMAC);
			m_dwHumEffTime[MMASK_CHARMAC_INDEX] = 0;
			m_dwACInrease = 0;
			if(GetType() == SOT_HERO)
			{
				static_cast<HeroObject*>(this)->SendSystemMessage("您的防御魔御回复正常");
			}
		}
	}
	//	Hide out of date
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_HIDE))
	{
		if(_dwCurTick > m_dwHumEffTime[MMASK_HIDE_INDEX])
		{
			ResetEffStatus(MMASK_HIDE);

			/*PkgPlayerSetEffectAck ack;
			ack.uTargetId = GetID();
			ack.dwMgcID = MEFF_HIDE;
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << ack;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);*/
		}
	}

	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_STONE))
	{
		if(_dwCurTick > m_dwHumEffTime[MMASK_STONE_INDEX])
		{
			ResetEffStatus(MMASK_STONE);
		}
	}

	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_ICE))
	{
		if(_dwCurTick > m_dwHumEffTime[MMASK_ICE_INDEX])
		{
			ResetEffStatus(MMASK_ICE);
		}
	}

	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_DECDC))
	{
		if(_dwCurTick > m_dwHumEffTime[MMASK_DECDC_INDEX])
		{
			ResetEffStatus(MMASK_DECDC);
		}
	}

	bool bCanAddHPMP = true;
	if(TEST_FLAG(m_dwHumEffFlag, MMASK_LVDU))
	{
		bCanAddHPMP = false;
		if(_dwCurTick > m_dwHumEffTime[MMASK_LVDU_INDEX])
		{
			RESET_FLAG(m_dwHumEffFlag, MMASK_LVDU);
			m_dwHumEffTime[MMASK_LVDU_INDEX] = 0;
			m_dwPoison = 0;
		}
		else if(_dwCurTick - m_dwLastUpdatePoisonTime > INTERVAL_INC_HP * 2)
		{
			m_dwLastUpdatePoisonTime = _dwCurTick;
			//int nDamage = m_dwPoison * 2;
			int nDamage = m_dwPoison;
			/*if(m_dwPoison == 3)
			{
				nDamage = 4;
			}*/

			if(m_stData.eGameState != OS_DEAD &&
				m_bCanPosion)
			{
				if(GetObject_HP() > nDamage)
				{
					//m_stData.stAttrib.HP -= nDamage;
					DecHP(nDamage);
					m_pValid->DecHP(nDamage);
					ntf.bType = UPDATE_HP;
					ntf.dwParam = GetObject_HP();
					g_xThreadBuffer.Reset();
					g_xThreadBuffer << ntf;
					pScene->BroadcastPacket(&g_xThreadBuffer);
					m_nTotalRecvDamage += nDamage;
				}
			}
		}
	}


	//PROTECT_START_VM
	
	if(GetType() == SOT_HERO)
	{
		dwAddInterval = 15 * 1000;
	}
	else
	{
		if(GetObject_ID() == 140 ||
			GetObject_ID() == 143 ||
			GetObject_ID() == 159)
		{
			//	破天魔龙
			dwAddInterval = 20 * 1000;
		}
		else
		{
			dwAddInterval = 40 * 1000;
		}
	}
	dwAddInterval = m_dwHPRecoverInterval;

	if(_dwCurTick - m_dwLastAddHpMpTime > dwAddInterval &&
		bCanAddHPMP)
	{
		int nAddHP = GetObject_MaxHP() / 25;
		int nAddMP = GetObject_MaxMP() / 20;
		int nPreHP = GetObject_HP();
		int nPreMP = GetObject_MP();

		if(m_stData.eGameState != OS_DEAD &&
			GetObject_HP() != 0)
		{
			if(nAddHP == 0)
			{
				nAddHP = 1;
			}
			if(nAddMP == 0)
			{
				nAddMP = 1;
			}

			IncHP(nAddHP);
			m_pValid->IncHP(nAddHP);
			//m_pValid->SetHP(nPreHP);
			if(GetObject_HP() != nPreHP)
			{
				ntf.bType = UPDATE_HP;
				ntf.dwParam = GetObject_HP();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ntf;
				//	只发给自己
				//pScene->BroadcastPacket(&g_xThreadBuffer);
				if(SettingLoader::GetInstance()->GetIntValue("DISABLEVALUENOTIFY") == 1)
				{
					//	nothing
				}
				else
				{
					pScene->BroadcastPacket(&g_xThreadBuffer, GetID());
				}

				if(GetType() == SOT_HERO)
				{
					static_cast<HeroObject*>(this)->SendPacket(ntf);
				}
			}
			IncMP(nAddMP);
			m_pValid->IncMP(nAddMP);
			//m_pValid->SetMP(nPreMP);
			if(GetObject_MP() != nPreMP)
			{
				ntf.bType = UPDATE_MP;
				ntf.dwParam = GetObject_MP();
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ntf;
				//	增加蓝 只发给自己
				//pScene->BroadcastPacket(&g_xThreadBuffer);
				if(GetType() == SOT_HERO)
				{
					static_cast<HeroObject*>(this)->SendPacket(ntf);
				}
			}
		}
		m_dwLastAddHpMpTime = _dwCurTick;
	}

	//	Check status
	if(GetTickCount() - m_dwLastCheckValidTime > 10 * 1000)
	{
		m_dwLastCheckValidTime = GetTickCount();
		if(!m_pValid->TestValid())
		{
			DEBUG_BREAK;
			//LOG(INFO) << "CHEATER";
			//PostQuitMessage(0);
			//_endthreadex(10);
			GameWorld::GetInstance().Stop(10);
			//FlyToPrison();
			SetObject_Expr(0);
		}
	}
	//PROTECT_END
}

//////////////////////////////////////////////////////////////////////////
int GameObject::GetRandomAbility(ABILITY_TYPE _type)
{
	int nValue = 0;
	switch(_type)
	{
	case AT_AC:
		{
			nValue = GetRandomIn(GetObject_AC(), GetObject_MaxAC());
		}break;
	case AT_DC:
		{
			nValue = GetRandomIn(GetObject_DC(), GetObject_MaxDC());
		}break;
	case AT_MC:
		{
			nValue = GetRandomIn(GetObject_MC(), GetObject_MaxMC());
		}break;
	case AT_SC:
		{
			nValue = GetRandomIn(GetObject_SC(), GetObject_MaxSC());
		}break;
	case AT_MAC:
		{
			nValue = GetRandomIn(GetObject_MAC(), GetObject_MaxMAC());
		}break;
	default:
		{
			nValue = 0;
		}break;
	}

	if(TEST_FLAG(m_dwHumEffFlag, MMASK_CHARMAC) &&
		(_type == AT_AC || _type == AT_MAC))
	{
		nValue += m_dwACInrease;
	}

	//	being cursed
	if(TEST_FLAG_BOOL(m_dwHumEffFlag, MMASK_DECDC) &&
		(_type == AT_DC || _type == AT_SC || _type == AT_MC))
	{
		nValue /= 2;
	}

	//	poison decrease the defence both the ac and mac
	if(TEST_FLAG(m_dwHumEffFlag, MMASK_LVDU) &&
		(_type == AT_AC || _type == AT_MAC))
	{
		//nValue -= m_dwPoison * 2;
		float fDefenceMinus = float(m_dwPoison * 2) / 100;
		if(fDefenceMinus > 0.5f)
		{
			fDefenceMinus = 0.5f;
		}
		nValue *= (1.0f - fDefenceMinus);
	}
	if(nValue < 0)
	{
		nValue = 0;
	}

	return nValue;
}
//////////////////////////////////////////////////////////////////////////
void GameObject::SetEffStatus(DWORD _effMask, DWORD _time, DWORD _param)
{
	char szBuf[MAX_PATH];
	if(_effMask == MMASK_CHARMAC)
	{
		if(GetType() == SOT_HERO)
		{
			sprintf(szBuf, "防御魔御增加%d秒,上限提升%d",
				_time / 1000, _param);
			static_cast<HeroObject*>(this)->SendSystemMessage(szBuf);
		}
		SET_FLAG(m_dwHumEffFlag, _effMask);
		m_dwHumEffTime[MMASK_CHARMAC_INDEX] = GetTickCount() + _time;
		m_dwACInrease = _param;
	}
	else if(_effMask == MMASK_LVDU)
	{
		if(GetType() == SOT_HERO)
		{
			static_cast<HeroObject*>(this)->SendSystemMessage("您中毒了");
			static_cast<HeroObject*>(this)->SendStatusInfo();
		}
		SET_FLAG(m_dwHumEffFlag, _effMask);
		m_dwHumEffTime[MMASK_LVDU_INDEX] = GetTickCount() + _time;
		m_dwPoison = _param;
	}
	else if(_effMask == MMASK_HIDE)
	{
		SET_FLAG(m_dwHumEffFlag, _effMask);
		m_dwHumEffTime[MMASK_HIDE_INDEX] = GetTickCount() + _time;
	}
	else if(_effMask == MMASK_STONE)
	{
		if(GetType() == SOT_HERO)
		{
			static_cast<HeroObject*>(this)->SendSystemMessage("您被石化了");
		}
		SET_FLAG(m_dwHumEffFlag, _effMask);
		m_dwHumEffTime[MMASK_STONE_INDEX] = GetTickCount() + _time;
	}
	else if(_effMask == MMASK_ICE)
	{
		SET_FLAG(m_dwHumEffFlag, _effMask);
		m_dwHumEffTime[MMASK_ICE_INDEX] = GetTickCount() + _time;
	}
	else if(_effMask == MMASK_DECDC)
	{
		if(GetType() == SOT_HERO)
		{
			static_cast<HeroObject*>(this)->SendSystemMessage("您受到诅咒");
		}

		SET_FLAG(m_dwHumEffFlag, _effMask);
		m_dwHumEffTime[MMASK_DECDC_INDEX] = GetTickCount() + _time;
	}
}
//////////////////////////////////////////////////////////////////////////
void GameObject::ResetEffStatus(DWORD _effMask)
{
	if(_effMask == MMASK_HIDE)
	{
		RESET_FLAG(m_dwHumEffFlag, MMASK_HIDE);
		m_dwHumEffTime[MMASK_HIDE_INDEX] = 0;
	}
	else if(_effMask == MMASK_STONE)
	{
		RESET_FLAG(m_dwHumEffFlag, MMASK_STONE);
		m_dwHumEffTime[MMASK_STONE_INDEX] = 0;
	}
	else if(_effMask == MMASK_ICE)
	{
		RESET_FLAG(m_dwHumEffFlag, MMASK_ICE);
		m_dwHumEffTime[MMASK_ICE_INDEX] = 0;
	}
	else if(_effMask == MMASK_DECDC)
	{
		RESET_FLAG(m_dwHumEffFlag, MMASK_DECDC);
		m_dwHumEffTime[MMASK_DECDC_INDEX] = 0;
	}
}
//////////////////////////////////////////////////////////////////////////
bool GameObject::IsMissed(GameObject* _pTarget)
{
	int nAccuracy = GetObject_Accuracy() - _pTarget->GetObject_Hide() - 2;
	nAccuracy += 10;
	if(nAccuracy <= 2)
	{
		nAccuracy = 2;
	}
	if(nAccuracy > rand() % 10)
	{
		return false;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
void GameObject::ForceGotoValidPosition()
{
#ifdef _DEBUG
	LOG(INFO) << "Go back to valid position";
#endif

	m_stData.wCoordX = m_wLastValidPositionX;
	m_stData.wCoordY = m_wLastValidPositionY;
	m_stData.eGameState = OS_STAND;

	if(GetType() == SOT_HERO)
	{
		PkgPlayerForceActionAck ack;
		ack.uTargetId = GetID();
		ack.bType = FORCE_POSITION;
		ack.dwData = MAKE_POSITION_DWORD(this);
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << ack;
		static_cast<HeroObject*>(this)->SendPlayerBuffer(g_xThreadBuffer);
	}
}
//////////////////////////////////////////////////////////////////////////
bool GameObject::WalkOneStep(int _nDrt)
{
	if(_nDrt < 0 &&
		_nDrt > 7)
	{
		return false;
	}

	PkgObjectActionNot not;
	not.uAction = ACTION_WALK;
	not.uTargetId = GetID();
	not.uParam0 = MAKE_POSITION_DWORD(this);

	PkgObjectActionWalkNot snot;
	snot.uTargetId = GetID();
	snot.uUserId = SimpleActionHelper::MakeWalkAction(m_stData.nDrt, m_stData.wCoordX, m_stData.wCoordY);

	int nTargetX = m_stData.wCoordX + s_nMoveOft[_nDrt * 2];
	int nTargetY = m_stData.wCoordY + s_nMoveOft[_nDrt * 2 + 1];
	if(GetLocateScene()->CanMove(nTargetX, nTargetY))
	{
		if(GetLocateScene()->MoveMappedObject(m_stData.wCoordX, m_stData.wCoordY, nTargetX, nTargetY, this))
		{
			m_stData.wCoordX = nTargetX;
			m_stData.wCoordY = nTargetY;
			m_stData.nDrt = _nDrt;
			m_stData.eGameState = OS_WALK;
			/*m_stData.*/m_dwLastWalkTime = m_dwCurrentTime;

			not.uParam1 = MAKE_POSITION_DWORD(this);

			snot.dwPos = MAKE_POSITION_DWORD(this);

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << snot;

			if(GetType() == SOT_HERO)
			{
				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer, GetID());
			}
			else
			{
				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			}
		}
		else
		{
			ForceGotoValidPosition();
			return false;
		}

		m_dwLastWalkTime = GetTickCount();
		
		return true;
	}
	else
	{
		
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GameObject::RunOneStep(int _nDrt)
{
	if(_nDrt < 0 &&
		_nDrt > 7)
	{
		return false;
	}

	PkgObjectActionNot not;
	not.uAction = ACTION_RUN;
	not.uTargetId = GetID();
	not.uParam0 = MAKE_POSITION_DWORD(this);

	PkgObjectActionRunNot snot;
	snot.uTargetId = GetID();
	snot.uUserId = SimpleActionHelper::MakeWalkAction(m_stData.nDrt, m_stData.wCoordX, m_stData.wCoordY);

	int nTargetX = m_stData.wCoordX + s_nMoveOft[_nDrt * 2];
	int nTargetY = m_stData.wCoordY + s_nMoveOft[_nDrt * 2 + 1];
	int nNextTargetX = m_stData.wCoordX + s_nMoveOft[_nDrt * 2] * 2;
	int nNextTargetY = m_stData.wCoordY + s_nMoveOft[_nDrt * 2 + 1] * 2;
	if(GetLocateScene()->CanMove(nTargetX, nTargetY) &&
		GetLocateScene()->CanMove(nNextTargetX, nNextTargetY))
	{
		if(GetLocateScene()->MoveMappedObject(m_stData.wCoordX, m_stData.wCoordY, nNextTargetX, nNextTargetY, this))
		{
			m_stData.wCoordX = nNextTargetX;
			m_stData.wCoordY = nNextTargetY;
			m_stData.nDrt = _nDrt;
			m_stData.eGameState = OS_WALK;
			/*m_stData.*/m_dwLastRunTime = m_dwCurrentTime;

			not.uParam1 = MAKE_POSITION_DWORD(this);

			snot.dwPos = MAKE_POSITION_DWORD(this);

			g_xThreadBuffer.Reset();
			g_xThreadBuffer << snot;

			if(GetType() == SOT_HERO)
			{
				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer, GetID());
			}
			else
			{
				GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			}
		}
		else
		{
			ForceGotoValidPosition();
			return false;
		}

		m_dwLastWalkTime = GetTickCount();
		return true;
	}
	else
	{
	
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GameObject::TurnTo(int _nDrt)
{
	if(_nDrt < 0 &&
		_nDrt > 7)
	{
		return false;
	}

	m_stData.nDrt = _nDrt;
	PkgObjectActionNot not;
	not.uAction = ACTION_TURN;
	not.uParam0 = MAKE_POSITION_DWORD(this);
	not.uParam1 = m_stData.nDrt;
	not.uTargetId = GetID();
	

	//	简化版
	PkgObjectActionTurnNot snot;
	snot.uTargetId = GetID();
	snot.uUserId = SimpleActionHelper::MakeWalkAction(m_stData.nDrt, m_stData.wCoordX, m_stData.wCoordY);
	g_xThreadBuffer.Reset();
	g_xThreadBuffer << snot;

	if(GetType() == SOT_HERO)
	{
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer, GetID());
	}
	else
	{
		GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
	}
	/*m_stData.*/m_dwLastWalkTime = m_dwCurrentTime;
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool GameObject::FlyTo(int _x, int _y)
{
	if(GetLocateScene()->GetMapData(_x, _y)->bFlag == 0)
	{
		//	Can move
		if(GetLocateScene()->MoveMappedObject(m_stData.wCoordX, m_stData.wCoordY, _x, _y, this))
		{
			m_stData.wCoordX = _x;
			m_stData.wCoordY = _y;
			PkgObjectActionNot not;
			not.uAction = ACTION_FLY;
			not.uTargetId = GetID();
			not.uParam0 = GetMapID();
			not.uParam1 = MAKE_POSITION_DWORD(this);
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << not;
			GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
			return true;
		}
		else
		{
			ForceGotoValidPosition();
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool GameObject::FlyToMap(int _x, int _y, int _nMapID)
{
	GameScene* pCurScene = GetLocateScene();
	GameScene* pNewScene = GameSceneManager::GetInstance()->GetScene(_nMapID);
	if(NULL == pNewScene)
	{
		return false;
	}

	if(pNewScene->GetMapData(_x, _y)->bFlag == 0)
	{
		if(GetType() == SOT_HERO)
		{
			HeroObject* pHero = static_cast<HeroObject*>(this);
			pHero->SetNeedTransAni(true);
			return pHero->SwitchScene(_nMapID, _x, _y);
		}
		else if(GetType() == SOT_MONSTER)
		{
			GameScene* pNextScene = GameSceneManager::GetInstance()->GetScene(_nMapID);
			GameScene* pCurScene = GetLocateScene();

			if(pCurScene->GetMapID() == _nMapID)
			{
				return false;
			}

			if(pNextScene)
			{
				pCurScene->AddWaitRemove(this);
				pNextScene->AddWaitInsert(this);

				//if(!pCurScene->RemoveMappedObject(m_stData.wCoordX, m_stData.wCoordY, this))
				if(!pCurScene->RemoveMappedObject(GetValidPositionX(), GetValidPositionY(), this))
				{
					char szName[20];
					ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
					LOG(ERROR) << "Name:" << szName << " Coordinate x:" << GetUserData()->wCoordX << " y:" << GetUserData()->wCoordY
						<< "Valid pos x:" << GetValidPositionX() << " pos y:" << GetValidPositionY();
				}
				//pCurScene->EraseTarget(this);
				m_stData.wCoordX = _x;
				m_stData.wCoordY = _y;
				// Test? [11/13/2013 yuanxj]
				m_stData.wMapID = _nMapID;
				// end [11/13/2013 yuanxj]

				//	No target
				static_cast<MonsterObject*>(this)->SetTarget(NULL);
				//m_stData.wMapID = _nMapID;

				/*PkgPlayerChangeMapAck ack;
				ack.uTargetId = GetID();
				ack.wMapID = _dwMapID;
				ack.wPosY = m_stData.wCoordY;
				ack.wPosX = m_stData.wCoordX;
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ack;
				SendPlayerBuffer(g_xThreadBuffer);*/
				return true;
			}
			return false;
		}
	}
	return false;
}

bool GameObject::FlyToInstanceMap(int _x, int _y, GameInstanceScene* _pInsScene)
{
	GameScene* pCurScene = GetLocateScene();
	GameScene* pNewScene = _pInsScene;
	if(NULL == pNewScene)
	{
		return false;
	}

	if(pNewScene->GetMapData(_x, _y)->bFlag == 0)
	{
		/*if(GetType() == SOT_HERO)
		{
			return static_cast<HeroObject*>(this)->SwitchScene(_nMapID, _x, _y);
		}
		else if(GetType() == SOT_MONSTER)*/
		{
			GameScene* pNextScene = pNewScene;
			GameScene* pCurScene = GetLocateScene();

			if(pCurScene->GetMapID() == pNextScene->GetMapID())
			{
				return false;
			}

			if(pNextScene)
			{
				pCurScene->AddWaitRemove(this);
				pNextScene->AddWaitInsert(this);

				//if(!pCurScene->RemoveMappedObject(m_stData.wCoordX, m_stData.wCoordY, this))
				if(!pCurScene->RemoveMappedObject(GetValidPositionX(), GetValidPositionY(), this))
				{
					char szName[20];
					ObjectValid::GetItemName(&GetUserData()->stAttrib, szName);
					LOG(ERROR) << "Name:" << szName << " Coordinate x:" << GetUserData()->wCoordX << " y:" << GetUserData()->wCoordY
						<< "Valid pos x:" << GetValidPositionX() << " pos y:" << GetValidPositionY();
				}
				pCurScene->EraseTarget(this);
				m_stData.wCoordX = _x;
				m_stData.wCoordY = _y;
				// Test? [11/13/2013 yuanxj]
				m_stData.wMapID = pNextScene->GetMapID();
				// end [11/13/2013 yuanxj]

				//	No target
				if(GetType() == SOT_MONSTER)
				{
					static_cast<MonsterObject*>(this)->SetTarget(NULL);
				}
				else if(GetType() == SOT_HERO)
				{
					PkgPlayerChangeMapAck ack;
					ack.uTargetId = GetID();
					ack.wMapID = pNextScene->GetMapID();
					ack.wPosY = m_stData.wCoordY;
					ack.wPosX = m_stData.wCoordX;
					static_cast<HeroObject*>(this)->SendPacket(ack);
				}
				//m_stData.wMapID = _nMapID;

				/*PkgPlayerChangeMapAck ack;
				ack.uTargetId = GetID();
				ack.wMapID = _dwMapID;
				ack.wPosY = m_stData.wCoordY;
				ack.wPosX = m_stData.wCoordX;
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ack;
				SendPlayerBuffer(g_xThreadBuffer);*/
				return true;
			}
			return false;
		}
	}
	return false;
}




//////////////////////////////////////////////////////////////////////////
void GameObject::EncryptObject()
{
	/*if(!IsEncrypt())
	{
		ItemAttrib oriatb;
		oriatb = m_stData.stAttrib;
		SetEncrypt();

		SetObject_ID(oriatb.id);
		//	Name
		SetObject_Lucky(oriatb.lucky);
		SetObject_Curse(oriatb.curse);
		SetObject_Hide(oriatb.hide);
		SetObject_Accuracy(oriatb.accuracy);
		SetObject_AtkSpeed(oriatb.atkSpeed);
		SetObject_AtkPalsy(oriatb.atkPalsy);
		SetObject_AtkPois(oriatb.atkPois);
		SetObject_MoveSpeed(oriatb.moveSpeed);
		SetObject_Weight(oriatb.weight);
		SetObject_ReqType(oriatb.reqType);
		SetObject_ReqValue(oriatb.reqValue);
		SetObject_Sex(oriatb.sex);
		SetObject_Type(oriatb.type);
		SetObject_MaxDC(oriatb.maxDC);
		SetObject_DC(oriatb.DC);
		SetObject_MaxAC(oriatb.maxAC);
		SetObject_AC(oriatb.AC);
		SetObject_MaxMAC(oriatb.maxMAC);
		SetObject_MAC(oriatb.MAC);
		SetObject_MaxSC(oriatb.maxSC);
		SetObject_SC(oriatb.SC);
		SetObject_MaxMC(oriatb.maxMC);
		SetObject_MC(oriatb.MC);
		SetObject_MaxHP(oriatb.maxHP);
		SetObject_HP(oriatb.HP);
		SetObject_MaxMP(oriatb.maxMP);
		SetObject_MP(oriatb.MP);
		SetObject_MaxExpr(oriatb.maxEXPR);
		SetObject_Expr(oriatb.EXPR);
		SetObject_Level(oriatb.level);
	}*/
	ObjectValid::EncryptAttrib(&m_stData.stAttrib);
}

const char* GameObject::GetName()
{
	static char s_szName[20] = {0};
	s_szName[0] = 0;

	if(!IsEncrypt())
	{
		strcpy(s_szName, m_stData.stAttrib.name);
	}
	else
	{
		ObjectValid::GetItemName(&m_stData.stAttrib, s_szName);
	}

	return s_szName;
}

void GameObject::AddDelayAction(DelayActionBase *_pAction)
{
	m_xDelayActions.push_back(_pAction);
}

void GameObject::ClearDelayAction()
{
	DelayActionList::iterator it = m_xDelayActions.begin();
	for(it;
		it != m_xDelayActions.end();
		++it)
	{
		delete *it;
	}
	m_xDelayActions.clear();
}