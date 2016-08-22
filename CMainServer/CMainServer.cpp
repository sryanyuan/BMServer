#include <afx.h>
#include "../CMainServer/CMainServer.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include "../Helper.h"
#include <Shlwapi.h>
#include <direct.h>
#include "../Net/Packet.h"
#include "../GameWorld/GameWorld.h"
#include "../GameWorld/GameSceneManager.h"
#include "../GameWorld/GameDbBuffer.h"
#include "../GameWorld/ExceptionHandler.h"
#include "../../CommonModule/ZipDataReader.h"
#include "../../CommonModule/SettingLoader.h"
#include "../../CommonModule/CommandLineHelper.h"
#include "../../CommonModule/LoginExtendInfoParser.h"
#include "../GameWorld/MonsterObject.h"
#include <zlib.h>
#include <time.h>
#include "../ServerDlg.h"
#include "../GameWorld/ConnCode.h"
#include "../../CommonModule/DataEncryptor.h"
#include "../CpuQueryDlg.h"


#if 0
#include <NetbaseWrapper.h>

#ifdef _DEBUG
#pragma comment(lib, "SServerEngine_d.lib")
#else
#pragma comment(lib, "SServerEngine.lib")
#endif

#endif

#ifdef _DEBUG
#pragma comment(lib, "libglog_static_d.lib")
#else
#pragma comment(lib, "libglog_static.lib")
#endif

#define MAX_SAVEDATA_SIZE 20480
ByteBuffer g_xMainBuffer(MAX_SAVEDATA_SIZE);

//////////////////////////////////////////////////////////////////////////
const char* g_szMode[2] = 
{
	"停止",
	"运行"
};

//////////////////////////////////////////////////////////////////////////
CMainServer::CMainServer()
{
	m_pxServer = NULL;
	m_bMode = MODE_STOP;
	m_dwUserNumber = 0;
	InitLogFile();
	m_dwThreadID = GetCurrentThreadId();
	m_pWatcherThread = NULL;
	m_eMode = GM_NORMAL;
	srand((unsigned int)time(NULL));
	m_bLoginConnected = false;
	m_dwLsConnIndex = 0;
	m_bAppException = false;
	m_dwListenPort = 0;
	m_bUseHTTech = false;
	InitializeCriticalSection(&m_csNetThreadEventList);
}

CMainServer::~CMainServer()
{
	if(m_pxServer)
	{
		if(!m_bUseHTTech)
		{
			DestoryNetEngineObject(m_pxServer);
		}
		else
		{
			delete m_pxServer;
		}
		
		m_pxServer = NULL;
	}
	GameWorld::DestroyInstance();

	DeleteCriticalSection(&m_csNetThreadEventList);
	/*m_pWatcherThread->Stop();
	delete m_pWatcherThread;*/
}


//////////////////////////////////////////////////////////////////////////

/************************************************************************/
/* 初始化网络
/************************************************************************/
bool CMainServer::InitNetWork()
{
	DESC_NETWORK desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.OnRecvFromUserTCP = _OnRecvFromUserTCP;
	desc.OnDisconnectUser = _OnDisconnectUser;
	desc.OnAcceptUser = _OnAcceptUser;

	desc.dwMainMsgQueMaxBufferSize = 1 * ONE_MB;
	desc.dwMaxUserNum = MAX_USER_NUMBER;
	desc.dwServerBufferSizePerConnection = 500 * ONE_KB;
	desc.dwUserBufferSizePerConnection = 64 * ONE_KB;
	
	desc.OnRecvFromServerTCP				= &CMainServer::_OnRecvFromServerTCP;
	desc.OnAcceptServer						= &CMainServer::_OnAcceptServer;
	desc.OnDisconnectServer					= &CMainServer::_OnDisconnectServer;
	desc.dwMaxServerNum						= 5;

	desc.dwServerMaxTransferSize			= 32 * 1024;
	desc.dwUserMaxTransferSize				= 32 * 1024;		//	数据包max size
	desc.dwConnectNumAtSameTime				= 10;
	desc.dwFlag								= 0;

	CUSTOM_EVENT ev[8];
	ZeroMemory(ev, sizeof(ev));
	ev[desc.dwCustomDefineEventNum].dwPeriodicTime = 10;
	ev[desc.dwCustomDefineEventNum++].pEventFunc = &CMainServer::_OnGameLoop;
	desc.pEvent = ev;

/*#if 1
	//desc.dwFlag = 1;
	m_pxServer = new NetbaseWrapper;
	WSADATA wsa={0};
	WSAStartup(MAKEWORD(2,2),&wsa);
#else
	m_pxServer = CreateNetEngineObject(1);
#endif*/
#if 0
	CCpuQueryDlg dlg;
	if(IDOK == dlg.DoModal())
	{
		m_bUseHTTech = true;
		m_pxServer = new NetbaseWrapper;
		WSADATA wsa={0};
		WSAStartup(MAKEWORD(2,2),&wsa);
	}
	else
#endif
	{
		m_pxServer = CreateNetEngineObject(1);
	}

	if(!m_pxServer->CreateNetwork(&desc, 50, 0))
	{
		//	LOG??
		LOG(FATAL) << "网络引擎创建失败";
		return false;
	}
	else
	{
		LOG(INFO) << "网络引擎创建成功";
	}
	return true;
}

/************************************************************************/
/* 启动游戏服务器
/************************************************************************/
bool CMainServer::StartServer(char* _szIP, WORD _wPort)
{
	if(!m_pxServer)
	{
		return false;
	}

	if(!m_pxServer->StartServerWithUserSide(_szIP, _wPort))
	{
		//	LOG??
		LOG(ERROR) << "地址[" << _szIP << "]:[" << _wPort << "]启动服务器失败";
		return false;
	}

	AddInfomation("服务端版本: %s", BACKMIR_CURVERSION);

	//	Initialize game scene
	if(!GameSceneManager::GetInstance()->CreateAllScene())
	{
		LOG(ERROR) << "载入地图场景失败";
		return false;
	}
	else
	{
		AddInfomation("游戏地图场景成功加载");
	}

	//	Run the Game world
	if(!GameWorld::GetInstance().Run())
	{
		return false;
	}
	else
	{
		AddInfomation("游戏世界成功启动");
	}

	//	Run the crc thread
	//InitCRCThread();
	//m_pWatcherThread->Run();

	LOG(INFO) << "地址[" << _szIP << "]:[" << _wPort << "]启动服务器成功";
	m_bMode = MODE_RUNNING;
	m_dwListenPort = _wPort;
	m_xListenIP = _szIP;
	UpdateServerState();

	if(GetServerMode() == GM_LOGIN)
	{
		GameWorld::GetInstancePtr()->SetFinnalExprMulti(SettingLoader::GetInstance()->GetIntValue("FINNALEXPRMULTI"));
#ifdef _DEBUG
		LOG(INFO) << "Get expr multi : " << GameWorld::GetInstance().GetFinnalExprMulti() << " mode:" << GetServerMode();
		g_xConsole.CPrint("Final expr multi:%d", GameWorld::GetInstance().GetFinnalExprMulti());
#endif
	}

	//ConnectToLoginSvr();
	
	return true;
}
/************************************************************************/
/* 连接登陆服务器                                                                
/************************************************************************/
bool CMainServer::ConnectToLoginSvr()
{
	if(m_bLoginConnected)
	{
		return true;
	}

	if(GetServerMode() == GM_LOGIN)
	{
		if(!m_xLoginAddr.empty())
		{
			size_t uPos = m_xLoginAddr.find(':');
			if(uPos == std::string::npos)
			{
				return false;
			}

			std::string xLsAddr;
			int nPort = 0;
			xLsAddr = m_xLoginAddr.substr(0, uPos);
			nPort = atoi(m_xLoginAddr.substr(uPos+1).c_str());

			char szIP[50];
			strcpy(szIP, xLsAddr.c_str());

			return m_pxServer->ConnectToServerWithServerSide(szIP, nPort, &CMainServer::_OnLsConnSuccess, &CMainServer::_OnLsConnFailed, NULL) ? true : false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return true;
	}
}

void STDCALL CMainServer::_OnLsConnSuccess(DWORD _dwIndex, void* _pParam)
{
	CMainServer::GetInstance()->m_bLoginConnected = true;
	CMainServer::GetInstance()->m_dwLsConnIndex = _dwIndex;
	AddInfomation("连接登陆服务器成功");

	CommandLineHelper xHelper;
	xHelper.InitParam();
	const char* pszLsAddr = xHelper.GetParam("outerip");
	if(NULL == pszLsAddr)
	{
		return;
	}

	//	服务器注册
	ByteBuffer xBuf;
	xBuf.Reset();
	xBuf << (int)0;
	xBuf << (int)PKG_LOGIN_SERVERVERIFY_REQ;
	xBuf << (short)1;
	xBuf << (int)0;
	xBuf << (char)strlen(pszLsAddr);
	xBuf.Write(pszLsAddr, strlen(pszLsAddr));
	SendBufferToServer(_dwIndex, &xBuf);

	//	发送登陆服务器注册成功事件
	MSG msgQuery;
	msgQuery.message = WM_LSCONNECTED;
	msgQuery.wParam = (WPARAM)_dwIndex;
	msgQuery.lParam = 0;
	GameWorld::GetInstancePtr()->PostRunMessage(&msgQuery);
}

void STDCALL CMainServer::_OnLsConnFailed(void* _pParam)
{
	CMainServer::GetInstance()->m_bLoginConnected = false;
	CMainServer::GetInstance()->m_dwLsConnIndex = 0;
	AddInfomation("连接登陆服务器失败");
	g_xConsole.CPrint("Login server address:%s, can't connect to.", CMainServer::GetInstance()->m_xLoginAddr.c_str());
}

/************************************************************************/
/* 停止游戏服务器
/************************************************************************/
void CMainServer::StopServer()
{
	// 	for(PlayerMap::const_iterator iter = m_xPlayers.begin();
	// 		iter != m_xPlayers.end();
	// 		++iter)
	// 	{
	// 		iter->second->OnDisconnect();
	// 	}
	// 	m_xPlayers.clear();

	if(m_pxServer)
	{
		if(!m_bUseHTTech)
		{
			DestoryNetEngineObject(m_pxServer);
		}
		else
		{
			delete m_pxServer;
		}
	}
	LOG(INFO) << "服务器已停止";
	AddInfomation("服务器已停止");
	m_bMode = MODE_STOP;
	UpdateServerState();
	m_pxServer = NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CMainServer::InitDatabase()
{
	PROTECT_START_VM
	//	Get encrypt table
	ObjectValid::GenerateEncryptTable();
	//	First open database
	char szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	PathRemoveFileSpec(szPath);
	strcat(szPath, "\\Help\\legend.bm");
	if(!DBThread::GetInstance()->ConnectDB(szPath))
	{
		LOG(ERROR) << "Can not connect to db[" << szPath << "]";
		ALERT_MSGBOX("无法连接数据库，请不要放在中文路径中");
		return false;
	}

	bool bRet = true;

	if(DBThread::GetInstance()->LoadItemsPrice())
	{
		AddInfomation("读取装备价格成功");
	}
	else
	{
		AddInfomation("读取装备价格失败");
		bRet = false;
	}
	if(DBThread::GetInstance()->LoadMagicAttrib())
	{
		AddInfomation("读取魔法信息成功");
	}
	else
	{
		AddInfomation("读取魔法信息失败");
		bRet = false;
	}

	if(!CheckVersion())
	{
		AddInfomation("游戏已损坏");
		bRet = false;
	}

	if(!CreateGameDbBuffer())
	{
		AddInfomation("无法创建缓存");
		bRet = false;
	}

	EncryptGlobalValue();

	//	读套装属性
	/*const char* pszTestData = "1:圣战套装|203,303,403,823|2,1;3,2;4,3|0,3;1,2;4,1";
	ItemExtraAttribList extraAttrib;
	ParseExtraItemData(pszTestData, &extraAttrib);*/

	g_bEncryptSuitName = true;
	if(!InitItemExtraAttribForServer())
	{
		LOG(ERROR) << "无法读取套装信息";
	}

	//	Get global settings
	sprintf(szPath, "%s\\cfg.ini", GetRootPath());
	SettingLoader::GetInstance()->LoadSetting(szPath);
	GameWorld::GetInstancePtr()->SetGenElitMons((SettingLoader::GetInstance()->GetIntValue("GENELITEMONS") != 0));
	GameWorld::GetInstancePtr()->SetEnableOfflineSell((SettingLoader::GetInstance()->GetIntValue("OFFLINESELL") != 0));
	GameWorld::GetInstancePtr()->SetEnableWorldNotify((SettingLoader::GetInstance()->GetIntValue("WORLDNOTIFY") != 0));

	//	Get item grade information
	if(!InitItemGradeForServer())
	{
		LOG(ERROR) << "无法读取装备额外信息";
	}
	
	PROTECT_END_VM
	return bRet;
	/*if(!m_xDatabase->Run() &&
		false)
	{
		//	Error occur
		LOG(ERROR) << "Can not start database!";
		return 0;
	}*/
}
/************************************************************************/
/* 发送数据包
/************************************************************************/
void CMainServer::SendPacket(DWORD _dwIndex, PacketBase* _pPacket)
{
	m_pxServer->SendToUser(_dwIndex, (char*)_pPacket + 4, _pPacket->dwSize - 4, 0);
}

/************************************************************************/
/* 添加ConnectionIndex与UserID的映射
/************************************************************************/
void CMainServer::InsertUserConnectionMapKey(WPARAM _wParam, LPARAM _lParam)
{
	RECORD_FUNCNAME_SERVER;
	//	Set mapped key
}

/************************************************************************/
/* 连接事件处理
/************************************************************************/
void CMainServer::OnAcceptUser(DWORD _dwIndex)
{
	static ByteBuffer s_xMainTcpBuf;

	RECORD_FUNCNAME_SERVER;

	char szIP[20];
	WORD wPort = 0;

	//	生成一个连接序号
	DWORD dwConnCode = GetNewConnCode();
	SetConnCode(_dwIndex, dwConnCode);

	//CUser* pUser = new CUser(_dwIndex);
	//PlayerMap::iterator iter = m_xPlayers.find(_dwIndex);
	//if(iter == m_xPlayers.end())
	{
		m_pxServer->GetUserAddress(_dwIndex, szIP, &wPort);
		AddInfomation("玩家[%s]:[%d]连接", szIP, wPort);
		LOG(INFO) << "玩家[" << szIP << "]:" << wPort << "连接, INDEX:[" << _dwIndex << "], conn code[" << dwConnCode << "]";

		//pUser->OnConnect();
		//m_xPlayers.insert(std::make_pair(_dwIndex, pUser));
		UpdateServerState();

		/*
		MSG msg;
				msg.message = WM_USERCONNECTED;
				msg.wParam = _dwIndex;
				GameWorld::GetInstance().PostRunMessage(&msg);*/
		
		{
			PkgLoginGameTypeNot not;
			not.bType = PLGTN_GAMESERVER;
			not.dwConnIdx = _dwIndex;
			s_xMainTcpBuf.Reset();
			s_xMainTcpBuf << not;
			SendBuffer(_dwIndex, &s_xMainTcpBuf);
		}
	}
}

/************************************************************************/
/* 断开事件处理
/************************************************************************/
void CMainServer::OnDisconnectUser(DWORD _dwIndex)
{
	RECORD_FUNCNAME_SERVER;

	char szIP[20];
	WORD wPort = 0;

	//	连接序号归位0
	SetConnCode(_dwIndex, 0);

	//PlayerMap::iterator iter = m_xPlayers.find(_dwIndex);
	//if(iter != m_xPlayers.end())
	{
		if(_dwIndex <= MAX_CONNECTIONS)
		{
			if(g_pxHeros[_dwIndex] != NULL)
			{
				//AddInfomation("玩家[%s]断开", g_pxHeros[_dwIndex]->GetUserData()->stAttrib.name);
				//m_pxServer->GetUserAddress(_dwIndex, szIP, &wPort);
				//AddInfomation("玩家[%s]:[%d]断开", szIP, wPort);
				AddInfomation("玩家[%d]断开", _dwIndex);

				LOG(INFO) << "player[" << _dwIndex << "]disconnect";

				if (GameWorld::GetInstance().GetThreadRunMode()) {
					DelayedProcess dp;
					dp.uOp = DP_USERLOGOUT;
					dp.uParam0 = g_pxHeros[_dwIndex]->GetID();
					GameWorld::GetInstance().AddDelayedProcess(&dp);
					g_pxHeros[_dwIndex] = NULL;
				} else {
					//	directly invoke and set nil
					GameWorld::GetInstance().SyncOnHeroDisconnected(g_pxHeros[_dwIndex]);
					g_pxHeros[_dwIndex] = NULL;
				}
			}
		}

		//iter->second->OnDisconnect();
		//m_xPlayers.erase(iter);
		--m_dwUserNumber;
		UpdateServerState();

		//	erase the global table
		if(_dwIndex <= MAX_CONNECTIONS)
		{
			g_pxHeros[_dwIndex] = NULL;
		}
	}

	//m_pxServer->SendToUser(_dwIndex, "111", 3, 0);
}

/************************************************************************/
/* 检测玩家是否合法
/************************************************************************/
bool CMainServer::CheckUserValid(GameObject* _pObj)
{
	ItemAttrib& stItem = _pObj->GetUserData()->stAttrib;
	bool bValid = true;

	if(stItem.HP > stItem.maxHP)
	{
		bValid = false;
	}
	if(stItem.MP > stItem.maxMP)
	{
		bValid = false;
	}
	if(stItem.EXPR > stItem.maxEXPR)
	{
		bValid = false;
	}

	return bValid;
}

/************************************************************************/
/* 数据处理
/************************************************************************/
void CMainServer::OnRecvFromUserTCP(DWORD _dwIndex, ByteBuffer* _xBuf)
{
	RECORD_FUNCNAME_SERVER;

	DWORD dwOpCode = GET_PACKET_OP_CLIENT((*_xBuf));
	static ByteBuffer s_xMainTcpBuf;

	if(dwOpCode >= PKG_LOGIN_START)
	{
		if(GetServerMode() == GM_LOGIN)
		{
			if(dwOpCode == PKG_LOGIN_CONNINDEX_NOT)
			{
				PkgLoginConnIdxNot not;
				*_xBuf >> not;
				not.dwGSIdx = _dwIndex;

				PkgLoginGsConnIdxNot plgcin;
				plgcin.dwConnCode = GetConnCode(_dwIndex);
				plgcin.dwLSIdx = not.dwLSIdx;
				plgcin.dwGSIdx = not.dwGSIdx;

				s_xMainTcpBuf.Reset();
				s_xMainTcpBuf << plgcin;
				SendBufferToServer(m_dwLsConnIndex, &s_xMainTcpBuf);
			}
		}
	}
	else
	{
		switch(dwOpCode)
		{
		case PKG_SYSTEM_USERLOGIN_REQ:
			{
				if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
				{
					//	不接受连接
					return;
				}
				PkgUserLoginReq req;
				*_xBuf >> req;
				OnPreProcessPacket(_dwIndex, 0, 0, NULL, req);
			}break;
		default:
			{
				if(_dwIndex <= MAX_CONNECTIONS)
				{
					if(g_pxHeros[_dwIndex])
					{
						GameWorld::GetInstance().OnMessage(_dwIndex, g_pxHeros[_dwIndex]->GetID(), _xBuf);
					}
				}
			}break;
		}
	}
}

void CMainServer::OnRecvFromServerTCP(DWORD _dwIndex, ByteBuffer* _xBuf)
{
	DWORD dwOpCode = GET_PACKET_OP((*_xBuf));

	if(dwOpCode == PKG_LOGIN_SENDHUMDATA_NOT)
	{
		PkgLoginSendHumDataNot not;
		*_xBuf >> not;

		//LOG(INFO) << "Player index[" << not.dwPlayerIndex << "] request to login. conn code[" << not.dwConnCode << "], actual conn code[" << GetConnCode(not.dwPlayerIndex) << "]";

		//	检查conn code
		if(GetConnCode(not.dwPlayerIndex) != not.dwConnCode)
		{
			LOG(ERROR) << "conn code not equal! index[" << not.dwPlayerIndex << "] " << GetConnCode(not.dwPlayerIndex) << "!=" << not.dwConnCode;
			return;
		}
		if(not.dwConnCode == 0)
		{
			LOG(ERROR) << "conn code invalid.";
			return;
		}

		PkgUserLoginReq req;
		memcpy(&req.stHeader, &not.stHeader, sizeof(HeroHeader));
		req.xData = not.xData;
		OnPreProcessPacket(not.dwPlayerIndex, not.dwLSIndex, not.dwUID, not.xExtendInfo.c_str(), req);

		//
		GameWorld::GetInstance().DisableAutoReset();
	}
	else if(dwOpCode == PKG_LOGIN_SENDHUMEXTDATA_NOT)
	{
		PkgLoginSendHumExtDataNot not;
		*_xBuf >> not;

		if(GetConnCode(not.dwPlayerIndex) != not.dwConnCode)
		{
			LOG(ERROR) << "conn code not equal! index[" << not.dwPlayerIndex << "] " << GetConnCode(not.dwPlayerIndex) << "!=" << not.dwConnCode;
			return;
		}
		if(not.dwConnCode == 0)
		{
			LOG(ERROR) << "conn code invalid.";
			return;
		}

		//	转换成客户端的数据包
		PkgPlayerLoginExtDataReq req;
		req.xData = not.xData;
		req.cIndex = not.cExtIndex;

		static ByteBuffer s_xExtDataBuffer;
		s_xExtDataBuffer.Reset();
		s_xExtDataBuffer << req;

		HeroObject* pHero = g_pxHeros[not.dwPlayerIndex];
		if(NULL != pHero)
		{
			if (GameWorld::GetInstance().GetThreadRunMode()) {
				pHero->PushMessage(&s_xExtDataBuffer);
			} else {
				PacketHeader* pHeader = (PacketHeader*)s_xExtDataBuffer.GetHead();
				pHero->DispatchPacket(s_xExtDataBuffer, pHeader);
			}
		}
	}
	else if(dwOpCode == PKG_LOGIN_HUMRANKLIST_NOT)
	{
		//	排行榜信息
		PkgLoginHumRankListNot not;
		*_xBuf >> not;

		g_xConsole.CPrint("Receive login server msg:Human rank list:");
		g_xConsole.CPrint(not.xData.c_str());

		//	post 给游戏线程
		char* pszRankCopy = new char[not.xData.size() + 1];
		strcpy(pszRankCopy, not.xData.c_str());
		MSG msgQuery;
		msgQuery.message = WM_PLAYERRANKLIST;
		msgQuery.wParam = (WPARAM)pszRankCopy;
		msgQuery.lParam = 0;

		if (GameWorld::GetInstance().GetThreadRunMode()) {
			GameWorld::GetInstancePtr()->PostRunMessage(&msgQuery);
		} else {
			GameWorld::GetInstance().Thread_ProcessMessage(&msgQuery);
		}
	}
	else if(dwOpCode == PKG_LOGIN_CHECKBUYSHOPITEM_ACK)
	{
		//	检查是否能买物品
		PkgLoginCheckBuyShopItemAck* pAck = new PkgLoginCheckBuyShopItemAck;
		*_xBuf >> *pAck;

		MSG msgQuery;
		msgQuery.message = WM_CHECKBUYOLSHOPITEM;
		msgQuery.wParam = (WPARAM)pAck;

		if (GameWorld::GetInstance().GetThreadRunMode()) {
			GameWorld::GetInstancePtr()->PostRunMessage(&msgQuery);
		} else {
			GameWorld::GetInstance().Thread_ProcessMessage(&msgQuery);
		}
	}
	else if(dwOpCode == PKG_LOGIN_CONSUMEDONATE_ACK)
	{
		//	扣钱的通知
		PkgLoginConsumeDonateAck* pAck = new PkgLoginConsumeDonateAck;
		*_xBuf >> *pAck;

		MSG msgQuery;
		msgQuery.message = WM_CONSUMEDONATE;
		msgQuery.wParam = (WPARAM)pAck;
		
		if (GameWorld::GetInstance().GetThreadRunMode()) {
			GameWorld::GetInstancePtr()->PostRunMessage(&msgQuery);
		} else {
			GameWorld::GetInstance().Thread_ProcessMessage(&msgQuery);
		}
	}
	else if(dwOpCode == PKG_LOGIN_SCHEDULE_ACTIVE_RSP)
	{
		//	调度激活
		PkgLoginScheduleActiveRsp rsp;
		*_xBuf >> rsp;

		MSG msgQuery = {0};
		msgQuery.message = WM_SCHEDULEACTIVE;
		msgQuery.wParam = rsp.nEventId;
		
		if (GameWorld::GetInstance().GetThreadRunMode()) {
			GameWorld::GetInstancePtr()->PostRunMessage(&msgQuery);
		} else {
			GameWorld::GetInstance().Thread_ProcessMessage(&msgQuery);
		}
	}
	else
	{
		g_xConsole.CPrint("Unsolved server opcode[%d]", dwOpCode);
	}
}

//////////////////////////////////////////////////////////////////////////

void CMainServer::ForceCloseConnection(DWORD _dwIndex)
{
	PostMessage(g_hServerDlg, WM_CLOSECONNECTION, (WPARAM)_dwIndex, 0);
}

/************************************************************************/
/* 初始化日志
/************************************************************************/
bool CMainServer::InitLogFile()
{
	char szFilePath[MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	google::InitGoogleLogging(szFilePath);
	PathRemoveFileSpec(szFilePath);
	strcat(szFilePath, "\\serverlog\\");

	std::list<std::string> xFiles;
	char szDetectFile[MAX_PATH];
	strcpy(szDetectFile, szFilePath);
	strcat(szDetectFile, "*.*");
	BOOL bFind = FALSE;
	CFileFind xFinder;
	bFind = xFinder.FindFile(szDetectFile);
	while(bFind)
	{
		bFind = xFinder.FindNextFile();
		if(!xFinder.IsDots() &&
			! xFinder.IsDirectory())
		{
			std::string xStr = xFinder.GetFilePath();
			xFiles.push_back(xStr);
		}
	}
	xFinder.Close();

	if(xFiles.size() > 10)
	{
		for(std::list<std::string>::iterator iter = xFiles.begin();
			iter != xFiles.end();
			++iter)
		{
			DeleteFile((*iter).c_str());
		}
	}
	xFiles.clear();

	if(!PathFileExists(szFilePath))
	{
		mkdir(szFilePath);
	}
	google::SetLogDestination(google::GLOG_INFO, szFilePath);

	return true;
}

/************************************************************************/
/* 获取服务器状态信息
/************************************************************************/
void CMainServer::MakeServerState(ServerState* _pState)
{
	//_pState->wOnline = m_xPlayers.size();
	_pState->wOnline = (WORD)m_dwUserNumber;
	_pState->bMode = m_bMode;
}

/************************************************************************/
/* 更新服务器状态信息
/************************************************************************/
void CMainServer::UpdateServerState()
{
	ServerState state;
	MakeServerState(&state);
	UpdateDialogInfo(&state);

	//InterlockedExchange((unsigned int*)(&GameWorld::GetInstance().m_nOnlinePlayers), (unsigned int)state.wOnline);

#ifdef _DEBUG
#else
#endif
}

//////////////////////////////////////////////////////////////////////////
void CMainServer::_OnGameLoop(DWORD _dwEvtIndex)
{
	if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN &&
		!CMainServer::GetInstance()->m_bLoginConnected)
	{
		static DWORD s_dwLastConnTime = GetTickCount();

		if(GetTickCount() - s_dwLastConnTime > 5000)
		{
			if(!CMainServer::GetInstance()->m_xLoginAddr.empty())
			{
				CMainServer::GetInstance()->ConnectToLoginSvr();
			}
			s_dwLastConnTime = GetTickCount();
		}
	}

	//	process time event
	static DWORD s_dwLastProcessNetThreadEventTime = GetTickCount();
	if (GetTickCount() - s_dwLastProcessNetThreadEventTime > 3000) {
		s_dwLastProcessNetThreadEventTime = GetTickCount();
		CMainServer::GetInstance()->ProcessNetThreadEvent();
	}

	//	update gameworld
	static unsigned int s_nWorldRunState = 0;
	if (0 == s_nWorldRunState) {
		s_nWorldRunState = GameWorld::GetInstance().WorldRun();;
	}
}

void CMainServer::ProcessNetThreadEvent()
{
	BMLockGuard guard(&m_csNetThreadEventList);

	NetThreadEventList::iterator it = m_xNetThreadEventList.begin();
	for(;
		it != m_xNetThreadEventList.end();
		++it)
	{
		const NetThreadEvent& refEvt = *it;

		if(refEvt.nEventId == kNetThreadEvent_SmallQuit)
		{
			DWORD dwIndex = refEvt.nArg;

			//	small quit
			if(dwIndex <= MAX_CONNECTIONS)
			{
				if(g_pxHeros[dwIndex] != NULL)
				{
					HeroObject* pHero = g_pxHeros[dwIndex];
					pHero->SetSmallQuit(true);
					AddInfomation("玩家[%d]小退", dwIndex);

					LOG(INFO) << "player[" << dwIndex << "] small quit";

					if (GameWorld::GetInstance().GetThreadRunMode()) {
						DelayedProcess dp;
						dp.uOp = DP_USERLOGOUT;
						dp.uParam0 = g_pxHeros[dwIndex]->GetID();
						GameWorld::GetInstance().AddDelayedProcess(&dp);
						g_pxHeros[dwIndex] = NULL;
					} else {
						if (0 == GameWorld::GetInstance().SyncOnHeroDisconnected(g_pxHeros[dwIndex])) {
							
						}

						g_pxHeros[dwIndex] = NULL;
					}
				}
			}
		}
	}

	m_xNetThreadEventList.clear();
}

void CMainServer::SendNetThreadEvent(const NetThreadEvent& _refEvt)
{
	BMLockGuard guard(&m_csNetThreadEventList);
	m_xNetThreadEventList.push_back(_refEvt);
}
/************************************************************************/
/* 接收玩家
/************************************************************************/
void CMainServer::_OnAcceptUser(DWORD _dwIndex)
{
	GetInstance()->OnAcceptUser(_dwIndex);
}

/************************************************************************/
/* 玩家离开
/************************************************************************/
void CMainServer::_OnDisconnectUser(DWORD _dwIndex)
{
	GetInstance()->OnDisconnectUser(_dwIndex);
}

/************************************************************************/
/* 接收玩家数据
/************************************************************************/
void CMainServer::_OnRecvFromUserTCP(DWORD _dwIndex, char *_pMsg, DWORD _dwLen)
{
	RECORD_FUNCNAME_SERVER;

	//	校验数据包
	bool bValid = true;
	DWORD dwOpCode = 0;
	WORD wCheckSum = 0;

	do 
	{
		if(_dwLen + 4 < sizeof(PacketHeader))
		{
			LOG(ERROR) << "包体长度[" << _dwLen << "]小于PacketBase长度[" << sizeof(PacketHeader) << "]";
			bValid = false;
			break;
		}
		if(_dwLen >= 8 * 1024)
		{
			LOG(ERROR) << "Packet length too long...";
			bValid = false;
			break;
		}

		PacketHeader header;
		memcpy((char*)&header + 4, _pMsg, sizeof(unsigned int) * 3);
		dwOpCode = LOWORD(header.uOp);
		if(dwOpCode >= GAME_PKG_END)
		{
			LOG(ERROR) << "Invalid packet opcode:" << dwOpCode;
			bValid = false;
			break;
		}

		wCheckSum = HIWORD(header.uOp);
		WORD wCurrentCheckSum = DataEncryptor::GetCheckSum(dwOpCode, _pMsg + 4, _dwLen - 4);
		if(wCurrentCheckSum != wCheckSum)
		{
			LOG(ERROR) << "Invalid check sum:" << wCurrentCheckSum << "!=" << wCheckSum << " op:" << dwOpCode;
			bValid = false;
			break;
		}

		g_xMainBuffer.Reset();
		DWORD dwLength = _dwLen + sizeof(DWORD);
		if(0 == g_xMainBuffer.Write(&dwLength, sizeof(DWORD)))
		{
			LOG(FATAL) << "Unresolved packet!";
			bValid = false;
			break;
		}
		if(0 == g_xMainBuffer.Write(_pMsg, _dwLen))
		{
			LOG(FATAL) << "Unresolved packet!";
			bValid = false;
			break;
		}
	} while (false);

	/*WORD wCheckSum = GET_PACKET_CKSUM_CLIENT_RAW(_pMsg);
	WORD wServerCheckSum = DataEncryptor::GetCheckSum(dwOpCode, _pMsg, _dwLen);
	if(wServerCheckSum != wCheckSum)
	{
		LOG(ERROR) << "Check sum error:" << wCheckSum << "!=" << wServerCheckSum;
		bValid = false;
	}*/

	if(bValid)
	{
		DWORD* pOp = ((DWORD*)g_xMainBuffer.GetBuffer()) + 1;
		*pOp = dwOpCode;
		GetInstance()->OnRecvFromUserTCP(_dwIndex, &g_xMainBuffer);
	}
	else
	{
		CMainServer::GetInstance()->GetEngine()->CompulsiveDisconnectUser(_dwIndex);
	}
}

void CMainServer::_OnRecvFromServerTCP(DWORD _dwIndex, char* _pMsg, DWORD _dwLen)
{
	//	校验数据包
	bool bValid = true;
	if(_dwLen + 4 < 8)
	{
		LOG(ERROR) << "包体长度[" << _dwLen << "]小于长度[8]";
		bValid = false;
	}

	PacketHeader* pHeader = (PacketHeader*)_pMsg;
	g_xMainBuffer.Reset();
	DWORD dwLength = _dwLen + sizeof(DWORD);
	if(0 == g_xMainBuffer.Write(&dwLength, sizeof(DWORD)))
	{
		LOG(FATAL) << "Unresolved packet!";
		bValid = false;
	}
	if(0 == g_xMainBuffer.Write(_pMsg, _dwLen))
	{
		LOG(FATAL) << "Unresolved packet!";
		bValid = false;
	}

	if(bValid)
	{
		CMainServer::GetInstance()->OnRecvFromServerTCP(_dwIndex, &g_xMainBuffer);
	}
}

void CMainServer::_OnAcceptServer(DWORD _dwIndex)
{
	//
}

void CMainServer::_OnDisconnectServer(DWORD _dwIndex)
{
	CMainServer::GetInstance()->m_bLoginConnected = false;
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData110(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
	/*UserData* pUserData = _pHero->GetUserData();

	if(_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
		//g_xMainBuffer >> pUserData->stAttrib.name;
		//g_xMainBuffer >> pUserData->stAttrib.sex;
		_xBuf >> pUserData->wMapID;
		DWORD dwPos = 0;
		_xBuf >> dwPos;
		pUserData->wCoordX = LOWORD(dwPos);
		pUserData->wCoordY = HIWORD(dwPos);
		_xBuf >> dwPos;
		pUserData->stAttrib.HP = LOWORD(dwPos);
		pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
	}
	else
	{
		//
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	ItemAttrib item;
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}
	//	Magic
	BYTE bMagic = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel >= 1)
			{
				if(_pHero->AddUserMagic(wID))
				{
					--bLevel;
					for(int j = 0; j < bLevel; ++j)
					{
						_pHero->UpgradeUserMagic(wID);
					}
				}
			}
		}
	}
	return true;*/
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData111(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
	/*UserData* pUserData = _pHero->GetUserData();
	return false;

	if(_xBuf.GetLength() >= 17)
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
		pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
	}
	else
	{
		//
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	ItemAttrib item;
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}
	//	Magic
	BYTE bMagic = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;

			if(wID > 17)
			{
				++wID;
			}
			if(bLevel >= 1)
			{
				if(_pHero->AddUserMagic(wID))
				{
					--bLevel;
					for(int j = 0; j < bLevel; ++j)
					{
						_pHero->UpgradeUserMagic(wID);
					}
				}
			}
		}
	}
	return true;*/
}
//////////////////////////////////////////////////////////////////////////
bool CMainServer::LoadHumData112(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
	/*UserData* pUserData = _pHero->GetUserData();
#ifdef NDEBUG
	return false;
#endif

	if(_xBuf.GetLength() >= 17)
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
		pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
	}
	else
	{
		//
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	ItemAttrib item;
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}
	//	Magic
	BYTE bMagic = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel >= 1)
			{
				if(_pHero->AddUserMagic(wID))
				{
					--bLevel;
					for(int j = 0; j < bLevel; ++j)
					{
						_pHero->UpgradeUserMagic(wID);
					}
				}
			}
		}
	}
	return true;*/
}

bool CMainServer::LoadHumData113(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
	/*UserData* pUserData = _pHero->GetUserData();
#ifdef NDEBUG
	return false;
#endif

	if(_xBuf.GetLength() >= 17)
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
		pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
	}
	else
	{
		//
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	ItemAttrib item;
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			item.atkPois = 1;
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			item.atkPois = 1;
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}
	//	Magic
	BYTE bMagic = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel >= 1)
			{
				if(_pHero->AddUserMagic(wID))
				{
					--bLevel;
					for(int j = 0; j < bLevel; ++j)
					{
						_pHero->UpgradeUserMagic(wID);
					}
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			item.atkPois = 1;
			_pHero->AddStoreItem(&item);
		}
	}
	return true;*/
}

bool CMainServer::LoadHumData114(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
	/*UserData* pUserData = _pHero->GetUserData();

#ifdef NDEBUG
	return false;
#endif

	if(_xBuf.GetLength() >= 17)
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
		pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
	}
	else
	{
		//
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	ItemAttrib item;
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			item.atkPois = 1;
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			item.atkPois = 1;
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	bool bCanAdd = false;
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			bCanAdd = false;
			if(bLevel >= 1)
			{
				if(wID > PREMG_DC_BEGIN &&
					wID < PREMG_DC_END)
				{
					//	nothing
					bCanAdd = true;
				}
				else if(wID > PREMG_MC_BEGIN &&
					wID < PREMG_MC_END)
				{
					wID += (MEFF_MC_BEGIN - PREMG_MC_BEGIN);
					bCanAdd = true;
				}
				else if(wID > PREMG_SC_BEGIN &&
					wID < PREMG_SC_END)
				{
					wID += (MEFF_SC_BEGIN - PREMG_SC_BEGIN);
					bCanAdd = true;
				}

				if(bCanAdd)
				{
					if(_pHero->AddUserMagic(wID))
					{
						--bLevel;
						for(int j = 0; j < bLevel; ++j)
						{
							_pHero->UpgradeUserMagic(wID);
						}
					}
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			item.atkPois = 1;
			_pHero->AddStoreItem(&item);
		}
	}
	return true;*/
}

bool CMainServer::LoadHumData115(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	return false;
	/*UserData* pUserData = _pHero->GetUserData();

	if(_xBuf.GetLength() >= 17)
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
		pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
	}
	else
	{
		//
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	ItemAttrib item;
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			memcpy(_pHero->GetEquip((PLAYER_ITEM_TYPE)bPos), &item, sizeof(ItemAttrib));
		}
	}

	//	Magic
	BYTE bMagic = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel >= 1)
			{
				if(_pHero->AddUserMagic(wID))
				{
					--bLevel;
					for(int j = 0; j < bLevel; ++j)
					{
						_pHero->UpgradeUserMagic(wID);
					}
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->AddStoreItem(&item);
		}
	}

	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
	//return true;*/
}

bool CMainServer::LoadHumData116(HeroObject* _pHero, ByteBuffer& _xBuf)
{
#ifdef _DEBUG
	UserData* pUserData = _pHero->GetUserData();

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel >= 1)
			{
				if(_pHero->AddUserMagic(wID))
				{
					--bLevel;
					for(int j = 0; j < bLevel; ++j)
					{
						_pHero->UpgradeUserMagic(wID);
					}
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if(item.id == 97)
			{
				if(item.reqValue == 0)
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

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 3)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(_pHero->AddUserMagic(wID))
				{
					--bLevel;
					for(int j = 0; j < bLevel; ++j)
					{
						_pHero->UpgradeUserMagic(wID);
					}
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	/*UserData* pUserData = _pHero->GetUserData();

	if(_xBuf.GetLength() >= 17)
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
		pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
	}
	else
	{
		//
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
	{
		_xBuf >> dwMoney;
		_pHero->SetMoney(dwMoney);
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}

	BYTE bBag = 0;
	BYTE bBody = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBag;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	ItemAttrib item;
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bBody;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bMagic;
	}
	else
	{
		/ *delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;* /
		return false;
	}

	const MagicInfo* pMagicInfo = NULL;
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 3)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(wID < MEFF_USERTOTAL)
				{
					pMagicInfo = &g_xMagicInfoTable[wID];
					if(pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel])
					{
						if(_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for(int j = 0; j < bLevel; ++j)
							{
								_pHero->UpgradeUserMagic(wID);
							}
						}
					}
					else
					{
						return false;
					}
				}
			}
		}
	}

	//	storage
	BYTE bStorage = 0;
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	DWORD dwReserve = 0;
	_xBuf >> dwReserve;
	_xBuf >> dwReserve;

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif*/
}

bool CMainServer::LoadHumData201(HeroObject* _pHero, ByteBuffer& _xBuf)
{
	UserData* pUserData = _pHero->GetUserData();

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 3)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if(pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if(_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for(int j = 0; j < bLevel; ++j)
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
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if(_xBuf.GetLength() == 8)
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

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 3)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if(pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if(_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for(int j = 0; j < bLevel; ++j)
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
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if(_xBuf.GetLength() == 8)
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

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 3)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if(pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if(_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for(int j = 0; j < bLevel; ++j)
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
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if(_xBuf.GetLength() == 8)
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

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 3)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if(pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if(_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for(int j = 0; j < bLevel; ++j)
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
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if(_xBuf.GetLength() == 8)
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

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 3)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if(pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if(_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for(int j = 0; j < bLevel; ++j)
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
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if(_xBuf.GetLength() == 8)
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

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 3)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if(pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if(_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for(int j = 0; j < bLevel; ++j)
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
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if(_xBuf.GetLength() == 8)
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

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 3)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if(pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if(_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for(int j = 0; j < bLevel; ++j)
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
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if(_xBuf.GetLength() == 8)
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

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 7)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if(pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if(_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for(int j = 0; j < bLevel; ++j)
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
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if(_xBuf.GetLength() == 8)
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

	if(_xBuf.GetLength() >= 17)
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
		//pUserData->stAttrib.maxHP = g_nHPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxHP = GetGlobalHP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.HP > pUserData->stAttrib.maxHP)
		{
			pUserData->stAttrib.HP = pUserData->stAttrib.maxHP;
		}
		pUserData->stAttrib.MP = HIWORD(dwPos);
		//pUserData->stAttrib.maxMP = g_nMPTable[pUserData->stAttrib.level - 1][pUserData->bJob];
		pUserData->stAttrib.maxMP = GetGlobalMP(pUserData->stAttrib.level, pUserData->bJob);
		if(pUserData->stAttrib.MP > pUserData->stAttrib.maxMP)
		{
			pUserData->stAttrib.MP = pUserData->stAttrib.maxMP;
		}
		_xBuf >> dwPos;
		pUserData->stAttrib.EXPR = dwPos;
		//pUserData->stAttrib.maxEXPR = g_nExprTable[pUserData->stAttrib.level - 1];
		pUserData->stAttrib.maxEXPR = GetGlobalExpr(pUserData->stAttrib.level);
	}
	else
	{
		//
		/*delete pObj;
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return;*/
		return false;
	}


	if(_xBuf.GetLength() >= 2 * MAX_QUEST_NUMBER)
	{
		_xBuf >> *_pHero->GetQuest();
	}

	DWORD dwMoney = 0;
	if(_xBuf.GetLength() > 4)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bBag > 0)
	{
		for(int i = 0; i < bBag; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddBagItem(&item);
		}
	}
	if(_xBuf.GetLength() >= 1)
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
	if(bBody > 0)
	{
		BYTE bPos = 0;
		for(int i = 0; i < bBody; ++i)
		{
			_xBuf >> bPos;
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);

			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
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
	if(_xBuf.GetLength() >= 1)
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
	if(bMagic > 0)
	{
		BYTE bLevel = 0;
		WORD wID = 0;

		for(int i = 0; i < bMagic; ++i)
		{
			_xBuf >> wID;
			_xBuf >> bLevel;
			if(bLevel > 7)
			{
				return false;
			}
			if(bLevel >= 1)
			{
				if(wID < MEFF_USERTOTAL)
				{
#ifdef _DEBUG
#else
					pMagicInfo = &g_xMagicInfoTable[wID];
					if(pUserData->stAttrib.level >= pMagicInfo->wLevel[bLevel - 1])
#endif
					{
						if(_pHero->AddUserMagic(wID))
						{
							--bLevel;
							for(int j = 0; j < bLevel; ++j)
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
	if(_xBuf.GetLength() >= 1)
	{
		_xBuf >> bStorage;
	}
	else
	{
		return false;
	}

	if(bStorage > 0)
	{
		ItemAttrib item;

		for(int i = 0; i < bStorage; ++i)
		{
			_xBuf >> item;
			//item.atkPois = 1;
			SET_FLAG(item.atkPois, POIS_MASK_BIND);
			_pHero->OnItemDataLoaded(&item);

			if(item.id == 97)
			{
				if(item.reqValue == 0)
				{
					item.reqType = 1;
					item.reqValue = 38;
				}
			}
			_pHero->AddStoreItem(&item);
		}
	}

	//	two dword reserve
	if(_xBuf.GetLength() >= 8)
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
	if(!_pHero->ReadSaveExtendField(_xBuf))
	{
		return false;
	}

#ifdef _DEBUG
	return true;
#else
	return DBThread::GetInstance()->VerifyHeroItem(_pHero);
#endif
}
/************************************************************************/
/* 提前处理的数据包
/************************************************************************/
bool CMainServer::OnPreProcessPacket(DWORD _dwIndex, DWORD _dwLSIndex, DWORD _dwUID, const char* _pExtendInfo, PkgUserLoginReq& req)
{
	RECORD_FUNCNAME_SERVER;
#ifdef _DEBUG
	if(NULL != _pExtendInfo)
	{
		LOG(INFO) << "额外登录信息:" << _pExtendInfo;
	}
#endif
//AddInfomation("玩家[%s]登入游戏",
	//req.stAttrib.name);

	//	登入游戏 获得ID
	//PkgUserLoginAck ack;
	//ack.bOk = true;
	//ack.nId = GameWorld::GetInstance().GenerateObjectID();
	LoginExtendInfoParser xLoginInfoParser(NULL);
	if(NULL != _pExtendInfo)
	{
		xLoginInfoParser.SetContent(_pExtendInfo);
		if(!xLoginInfoParser.Parse())
		{
			LOG(ERROR) << "无法解析附加登录信息" << _pExtendInfo;
		}
	}

	//	进行一次合法性检测
	if(strlen(req.stHeader.szName) > 19)
	{
		LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
		return false;;
	}
	
	if(req.stHeader.bJob > 2 ||
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

	if (GameWorld::GetInstance().GetThreadRunMode()) {
		HANDLE hLoginQueryEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		MSG msgQuery;
		msgQuery.message = WM_PLAYERLOGIN;
		msgQuery.wParam = (WPARAM)hLoginQueryEvent;
		msgQuery.lParam = (LPARAM)&info;
		GameWorld::GetInstancePtr()->PostRunMessage(&msgQuery);

		//	wait the event
		DWORD dwWaitRet = WaitForSingleObject(hLoginQueryEvent, 1000);
		if(WAIT_FAILED == dwWaitRet)
		{
			LOG(ERROR) << "wait for Event LoginQuery failed. Error:" << GetLastError();
			//return;
		}
		else if(WAIT_TIMEOUT == dwWaitRet)
		{
			LOG(ERROR) << "wait for Event LoginQuery timeout.";
			//return;
		}
		else if(WAIT_OBJECT_0 == dwWaitRet)
		{
			//	ok
			if(!info.bExists)
			{
				//	可以进行登录
				LOG(INFO) << "Player[" << req.stHeader.szName << "] login ok! index[" << _dwIndex << "] conn code[" << GetConnCode(_dwIndex) << "]";
			}
			else
			{
				//	踢出之前的玩家
				LOG(ERROR) << "Player[" << req.stHeader.szName << "] login failed! same uid relogin, index[" << _dwIndex << "]";
				ForceCloseConnection(info.dwConnID);
				//return;
			}
		}

		CloseHandle(hLoginQueryEvent);
		hLoginQueryEvent = NULL;
	} else {
		GameWorld::GetInstance().SyncIsHeroExists(&info);

		if(!info.bExists)
		{
			//	可以进行登录
			LOG(INFO) << "Player[" << req.stHeader.szName << "] login ok! index[" << _dwIndex << "] conn code[" << GetConnCode(_dwIndex) << "]";
		}
		else
		{
			//	踢出之前的玩家
			LOG(ERROR) << "Player[" << req.stHeader.szName << "] login failed! same uid relogin, index[" << _dwIndex << "]";
			ForceCloseConnection(info.dwConnID);
			//return;
		}
	}

	if(info.bExists)
	{
		return false;
	}

	//	继续进行一次判断，看此用户是否已经登录完毕了
	if(g_pxHeros[_dwIndex] != NULL)
	{
		HeroObject* pExistHero = g_pxHeros[_dwIndex];
		LOG(ERROR) << "Player[" << req.stHeader.szName << "] already logined.";
		return false;
	}

	if(req.xData.size() == 0)
	{
		//	New hero

		//	进行登录操作
		HeroObject* pObj = new HeroObject(_dwIndex);
		pObj->SetMapID(0);
		pObj->GetUserData()->wCoordX = 20;
		pObj->GetUserData()->wCoordY = 16;
		pObj->GetUserData()->bJob = req.stHeader.bJob;
		ItemAttrib* pAttrib = &pObj->GetUserData()->stAttrib;
		pAttrib->level = 1;
		strcpy(pAttrib->name, req.stHeader.szName);
		//pAttrib->HP = g_nHPTable[pAttrib->level - 1][pObj->GetUserData()->bJob];
		pAttrib->HP = GetGlobalHP(pAttrib->level, pObj->GetUserData()->bJob);
		pAttrib->maxHP = pAttrib->HP;
		//pAttrib->MP = g_nMPTable[pAttrib->level - 1][pObj->GetUserData()->bJob];
		pAttrib->MP = GetGlobalMP(pAttrib->level, pObj->GetUserData()->bJob);
		pAttrib->maxMP = pAttrib->MP;
		pAttrib->EXPR = 0;
		//pAttrib->maxEXPR = g_nExprTable[pAttrib->level - 1];
		pAttrib->maxEXPR = GetGlobalExpr(pAttrib->level);
		pAttrib->sex = req.stHeader.bSex;

		if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
		{
			pObj->SetLSIndex(_dwLSIndex);
			pObj->SetUID(_dwUID);
			pObj->SetDonateMoney(xLoginInfoParser.GetDonateMoney());
			pObj->SetDonateLeft(xLoginInfoParser.GetDonateLeft());

			GiftItemIDVector& refGiftItems = pObj->GetGiftItemIDs();
			if(xLoginInfoParser.GetGiftCount() != 0)
			{
				refGiftItems.resize(xLoginInfoParser.GetGiftCount());

				for(int i = 0; i < xLoginInfoParser.GetGiftCount(); ++i)
				{
					refGiftItems[i] = xLoginInfoParser.GetGiftID(i);
				}
			}
		}

		if (GameWorld::GetInstance().GetThreadRunMode()) {
			DelayedProcess dp;
			dp.uOp = DP_USERLOGIN;
			dp.uParam0 = (unsigned int)pObj;
			dp.uParam1 = 0;
			GameWorld::GetInstance().AddDelayedProcess(&dp);

			//	record the player in the global table
			if(_dwIndex <= MAX_CONNECTIONS)
			{
				g_pxHeros[_dwIndex] = pObj;
			}

			//	Set mapped key
			++m_dwUserNumber;
			UpdateServerState();

			AddInfomation("玩家[%s]登入游戏",
				req.stHeader.szName);
		} else {
			if (0 != GameWorld::GetInstance().SyncOnHeroConnected(pObj, true)) {
				//	delete the hero
				SAFE_DELETE(pObj);
				g_pxHeros[_dwIndex] = NULL;
			} else {
				//	record the player in the global table
				if(_dwIndex <= MAX_CONNECTIONS)
				{
					g_pxHeros[_dwIndex] = pObj;
				}

				//	Set mapped key
				++m_dwUserNumber;
				UpdateServerState();

				AddInfomation("玩家[%s]登入游戏",
					req.stHeader.szName);
			}
		}
	}
	else
	{
		const char* pData = &req.xData[0];
		DWORD dwDataLen = req.xData.size();

		static char* s_pBuf = new char[MAX_SAVEDATA_SIZE];
		uLongf buflen = MAX_SAVEDATA_SIZE;
		uLongf srclen = dwDataLen;
		int nRet = uncompress((Bytef*)s_pBuf, &buflen, (const Bytef*)pData, srclen);
		if(nRet == Z_OK)
		{
			if(g_xMainBuffer.GetLength() > buflen)
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
			AddInfomation(szText);
			pObj->SetVersion(uVersion);

			if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
			{
				pObj->SetLSIndex(_dwLSIndex);
				pObj->SetUID(_dwUID);
				pObj->SetDonateMoney(xLoginInfoParser.GetDonateMoney());
				pObj->SetDonateLeft(xLoginInfoParser.GetDonateLeft());

				GiftItemIDVector& refGiftItems = pObj->GetGiftItemIDs();
				if(xLoginInfoParser.GetGiftCount() != 0)
				{
					refGiftItems.resize(xLoginInfoParser.GetGiftCount());

					for(int i = 0; i < xLoginInfoParser.GetGiftCount(); ++i)
					{
						refGiftItems[i] = xLoginInfoParser.GetGiftID(i);
					}
				}
			}

			if(uVersion == BACKMIR_VERSION209 ||
				uVersion == BACKMIR_VERSION208)
			{
				if(LoadHumData208(pObj, g_xMainBuffer))
				{
					//	Now Ok
					if (GameWorld::GetInstance().GetThreadRunMode()) {
						DelayedProcess dp;
						dp.uOp = DP_USERLOGIN;
						dp.uParam0 = (unsigned int)pObj;
						dp.uParam1 = 1;
						GameWorld::GetInstance().AddDelayedProcess(&dp);

						//	record the player in the global table
						if(_dwIndex <= MAX_CONNECTIONS)
						{
							g_pxHeros[_dwIndex] = pObj;
						}

						//	Set mapped key
						++m_dwUserNumber;
						UpdateServerState();

						/*AddInfomation("玩家[%s]登入游戏",
						req.stHeader.szName);*/
						sprintf(szText, "%s[%s]", s_pszUserLogin, req.stHeader.szName);
						AddInfomation(szText);
					} else {
						if (0 != GameWorld::GetInstance().SyncOnHeroConnected(pObj, false)) {
							SAFE_DELETE(pObj);
							g_pxHeros[_dwIndex] = NULL;
						} else {
							//	record the player in the global table
							if(_dwIndex <= MAX_CONNECTIONS)
							{
								g_pxHeros[_dwIndex] = pObj;
							}

							//	Set mapped key
							++m_dwUserNumber;
							UpdateServerState();

							/*AddInfomation("玩家[%s]登入游戏",
							req.stHeader.szName);*/
							sprintf(szText, "%s[%s]", s_pszUserLogin, req.stHeader.szName);
							AddInfomation(szText);
						}
					}
				}
				else
				{
					//AddInfomation("不支持的存档[%d]或非法的存档", uVersion);
					sprintf(szText, "%s[%d]", s_pszInvalid, uVersion);
					//AddInfomation("不支持的存档[%d]或非法的存档", uVersion);
					AddInfomation(szText);

					delete pObj;
#ifdef _DEBUG
					LOG(WARNING) << "玩家[" << _dwIndex << "]数据非法，强行踢出";
#endif
					return false;
				}
			}
			else if(uVersion == BACKMIR_VERSION210)
			{
				if(LoadHumData210(pObj, g_xMainBuffer))
				{
					//	Now Ok
					if (GameWorld::GetInstance().GetThreadRunMode()) {
						DelayedProcess dp;
						dp.uOp = DP_USERLOGIN;
						dp.uParam0 = (unsigned int)pObj;
						dp.uParam1 = 1;
						GameWorld::GetInstance().AddDelayedProcess(&dp);

						//	record the player in the global table
						if(_dwIndex <= MAX_CONNECTIONS)
						{
							g_pxHeros[_dwIndex] = pObj;
						}

						//	Set mapped key
						++m_dwUserNumber;
						UpdateServerState();

						/*AddInfomation("玩家[%s]登入游戏",
						req.stHeader.szName);*/
						sprintf(szText, "%s[%s]", s_pszUserLogin, req.stHeader.szName);
						AddInfomation(szText);
					} else {
						if (0 != GameWorld::GetInstance().SyncOnHeroConnected(pObj, false)) {
							SAFE_DELETE(pObj);
							g_pxHeros[_dwIndex] = NULL;
						} else {
							//	record the player in the global table
							if(_dwIndex <= MAX_CONNECTIONS)
							{
								g_pxHeros[_dwIndex] = pObj;
							}

							//	Set mapped key
							++m_dwUserNumber;
							UpdateServerState();

							/*AddInfomation("玩家[%s]登入游戏",
							req.stHeader.szName);*/
							sprintf(szText, "%s[%s]", s_pszUserLogin, req.stHeader.szName);
							AddInfomation(szText);
						}
					}
				}
				else
				{
					//AddInfomation("不支持的存档[%d]或非法的存档", uVersion);
					sprintf(szText, "%s[%d]", s_pszInvalid, uVersion);
					//AddInfomation("不支持的存档[%d]或非法的存档", uVersion);
					AddInfomation(szText);

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

bool CMainServer::InitCRCThread()
{
	PROTECT_START_VM

		//////////////////////////////////////////////////////////////////////////
	bool bRet = false;

	if(NULL == m_pWatcherThread)
	{
		m_pWatcherThread = new WatcherThread;
		//	Load the CRC Data
		//	Write the correct crc data
		char szBuf[MAX_PATH];
		sprintf(szBuf, "%s\\help\\",
			GetRootPath());
		int nStrlen = strlen(szBuf);
		szBuf[nStrlen] = 'd';
		szBuf[nStrlen + 1] = 'e';
		szBuf[nStrlen + 2] = 'a';
		szBuf[nStrlen + 3] = 'd';
		szBuf[nStrlen + 4] = '.';
		szBuf[nStrlen + 5] = 'i';
		szBuf[nStrlen + 6] = 'n';
		szBuf[nStrlen + 7] = 'i';
		szBuf[nStrlen + 8] = '\0';

		if(PathFileExists(szBuf))
		{
			//	Write the data
			//	Check valid
			CSimpleIniA xIniFile;
			xIniFile.LoadFile(szBuf);

			const char* pszValue = xIniFile.GetValue("GameWorld", "GameSceneManager");
			if(NULL != pszValue)
			{
				if(0 == strcmp(pszValue, "DBThread...Open"))
				{
					DWORD dwCRC32 = 0;
					WORD wCRC16 = 0;

					//	First function
					int nCheckIndex = 0;
					int nCodeLength = 0;
					char szIndex[10];
					char szCRC[32];
					//	Get Object
					void* pFunc = GetFuncAddr(&GameObject::GetObject_MaxAC);
					LPBYTE pData = (LPBYTE)pFunc;
					nCodeLength = 20;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					dwCRC32 = 0;
					pFunc = GetFuncAddr(&GameObject::GetObject_MaxMAC);
					pData = (LPBYTE)pFunc;
					nCodeLength = 20;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					dwCRC32 = 0;
					pFunc = GetFuncAddr(&GameObject::GetObject_MaxDC);
					pData = (LPBYTE)pFunc;
					nCodeLength = 20;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					dwCRC32 = 0;
					pFunc = GetFuncAddr(&GameObject::GetObject_MaxMC);
					pData = (LPBYTE)pFunc;
					nCodeLength = 20;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					dwCRC32 = 0;
					pFunc = GetFuncAddr(&GameObject::GetObject_MaxSC);
					pData = (LPBYTE)pFunc;
					nCodeLength = 20;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	Get random
					dwCRC32 = 0;
					GameObject xObj;
					pFunc = GetVirtualFuncAddr(&xObj, &GameObject::GetRandomAbility);
					pData = (LPBYTE)pFunc;
					nCodeLength = 40;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	Receive damage
					dwCRC32 = 0;
					MonsterObject xMons;
					pFunc = GetVirtualFuncAddr(&xMons, &MonsterObject::ReceiveDamage);
					pData = (LPBYTE)pFunc;
					nCodeLength = 20;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					dwCRC32 = 0;
					HeroObject xHero(0);
					pFunc = GetVirtualFuncAddr(&xHero, &HeroObject::ReceiveDamage);
					pData = (LPBYTE)pFunc;
					nCodeLength = 50;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	Do spell
					dwCRC32 = 0;
					pFunc = GetFuncAddr(&HeroObject::DoSpell);
					pData = (LPBYTE)pFunc;
					nCodeLength = 300;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	Parse attack message
					dwCRC32 = 0;
					pFunc = GetFuncAddr(&MonsterObject::ParseAttackMsg);
					pData = (LPBYTE)pFunc;
					nCodeLength = 300;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	Drop down monster items
					dwCRC32 = 0;
					void (MonsterObject::* pFunc_v_ph)(HeroObject*);
					pFunc_v_ph = &MonsterObject::DropMonsterItems;
					pFunc = GetFuncAddr(pFunc_v_ph);
					pData = (LPBYTE)pFunc;
					nCodeLength = 10;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	attack target
					dwCRC32 = 0;
					pFunc = GetFuncAddr(&MonsterObject::AttackTarget);
					pData = (LPBYTE)pFunc;
					nCodeLength = 100;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	add money
					dwCRC32 = 0;
					pFunc = GetFuncAddr(&HeroObject::AddMoney);
					pData = (LPBYTE)pFunc;
					nCodeLength = 100;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	gain exp
					dwCRC32 = 0;
					pFunc = GetFuncAddr(&HeroObject::GainExp);
					pData = (LPBYTE)pFunc;
					nCodeLength = 50;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	autogeneratemonster
					dwCRC32 = 0;
					pFunc = GetFuncAddr(&GameScene::AutoGenerateMonster);
					pData = (LPBYTE)pFunc;
					nCodeLength = 100;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	use drug item
					dwCRC32 = 0;
					pFunc = GetFuncAddr(&HeroObject::UseDrugItem);
					pData = (LPBYTE)pFunc;
					nCodeLength = 80;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	upgrade items
					dwCRC32 = 0;
					pFunc = GetFuncAddr(&DBThread::UpgradeItems);
					pData = (LPBYTE)pFunc;
					nCodeLength = 150;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	upgrade attrib
					dwCRC32 = 0;
					pFunc = GetFuncAddr(&DBThread::UpgradeAttrib);
					pData = (LPBYTE)pFunc;
					nCodeLength = 150;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	db dropdown monster items
					dwCRC32 = 0;
					pFunc = GetFuncAddr(&DBThread::DBDropItemExCallback);
					pData = (LPBYTE)pFunc;
					nCodeLength = 300;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	dopacket pkgplayerforgeitemreq
					dwCRC32 = 0;
					pFunc = m_pWatcherThread->GetVerifyFunctionAddr(WatcherThread::VF_HERO_DOPACKET_FORGEITEM);
					pData = (LPBYTE)pFunc;
					nCodeLength = 300;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	upgrade attrib
					dwCRC32 = 0;
					pFunc = m_pWatcherThread->GetVerifyFunctionAddr(WatcherThread::VF_WORLD_UPGRADEATTRIB);
					pData = (LPBYTE)pFunc;
					nCodeLength = 150;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	upgrade items with addtion
					dwCRC32 = 0;
					pFunc = m_pWatcherThread->GetVerifyFunctionAddr(WatcherThread::VF_WORLD_UPGRADEITEMSWITHADDITION);
					pData = (LPBYTE)pFunc;
					nCodeLength = 250;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	update status
					dwCRC32 = 0;
					pFunc = m_pWatcherThread->GetVerifyFunctionAddr(WatcherThread::VF_HERO_UPDATESTATUS);
					pData = (LPBYTE)pFunc;
					nCodeLength = 100;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					dwCRC32 = 0;
					pFunc = m_pWatcherThread->GetVerifyFunctionAddr(WatcherThread::VF_OBJ_UPDATESTATUS);
					pData = (LPBYTE)pFunc;
					nCodeLength = 150;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	thread_dowork
					dwCRC32 = 0;
					pFunc = m_pWatcherThread->GetVerifyFunctionAddr(WatcherThread::VF_WATCHER_THREAD_DOWORK);
					pData = (LPBYTE)pFunc;
					nCodeLength = 150;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	dowork_delayedprocess
					dwCRC32 = 0;
					pFunc = m_pWatcherThread->GetVerifyFunctionAddr(WatcherThread::VF_WORLD_DOWORK_DELAYEDPROCESS);
					pData = (LPBYTE)pFunc;
					nCodeLength = 150;
					CalcCRC32(&dwCRC32, nCodeLength, pData);
					itoa(nCheckIndex++, szIndex, 10);
					//itoa(dwCRC32, szCRC, 10);
					xIniFile.SetLongValue("GameWorld", szIndex, dwCRC32);
					xIniFile.SetLongValue("Size", szIndex, nCodeLength);

					//	save
					xIniFile.SaveFile(szBuf);
					xIniFile.Reset();
				}
			}
		}

		//	Load check data
		//if(false)
		{
			int nCRCItemCounter = 0;

			if(false)
			{
				sprintf(szBuf, "%s\\Help\\Mouse.idx",
					GetRootPath());
				BYTE* pData = NULL;
				size_t uSize = 0;

				if(READZIP_OK == ReadZipData(szBuf, "dead.ini", &pData, &uSize, SaveFile::CalcInternalPassword()))
				{
					CSimpleIniA xIniFile;
					if(SI_OK == xIniFile.LoadData((const char*)pData, uSize))
					{
						CSimpleIni::TNamesDepend xAllKeys;
						const CSimpleIni::Entry* pEntry = NULL;

						if(xIniFile.GetAllKeys("GameWorld", xAllKeys))
						{
							if(!xAllKeys.empty())
							{
								CSimpleIni::TNamesDepend::const_iterator begIter = xAllKeys.begin();
								CSimpleIni::TNamesDepend::const_iterator endIter = xAllKeys.end();

								char szTag[10];

								for(begIter;
									begIter != endIter;
									++begIter)
								{
									pEntry = &(*begIter);

									CRCVerifyElement stElement;
									ZeroMemory(&stElement, sizeof(CRCVerifyElement));
									stElement.dwCRC32 = xIniFile.GetLongValue("GameWorld",
										pEntry->pItem);
									stElement.dwSize = xIniFile.GetLongValue("Size",
										pEntry->pItem);
									stElement.uTag = atoi(pEntry->pItem);
									stElement.pData = m_pWatcherThread->GetVerifyFunctionAddr((WatcherThread::Verify_Function)stElement.uTag);
									m_pWatcherThread->Push(stElement);

									if(stElement.dwSize != 0 &&
										stElement.pData != NULL &&
										stElement.dwCRC32 != 0)
									{
										++nCRCItemCounter;
									}
								}
							}
						}
					}
					SAFE_DELETE_ARRAY(pData);
				}
			}

			//	Load internal data
			if(true)
			{
				for(int i = 0; i < WatcherThread::VF_TOTAL; ++i)
				{
					CRCVerifyElement stElement;
					ZeroMemory(&stElement, sizeof(CRCVerifyElement));
					stElement.uTag = i;
					stElement.pData = m_pWatcherThread->GetVerifyFunctionAddr((WatcherThread::Verify_Function)stElement.uTag);
					stElement.dwSize = WatcherThread::s_nVerifySize[i];
					CalcCRC32(&stElement.dwCRC32, stElement.dwSize, (LPBYTE)stElement.pData);
					m_pWatcherThread->Push(stElement);

					if(stElement.dwSize != 0 &&
						stElement.pData != NULL &&
						stElement.dwCRC32 != 0)
					{
						++nCRCItemCounter;
					}
				}
			}

			if(WatcherThread::VF_TOTAL != nCRCItemCounter)
			{
#ifndef _DISABLE_CONSOLE
				g_xConsole.CPrint("Lost crc check data , require[%d], read[%d]",
					WatcherThread::VF_TOTAL, nCRCItemCounter);
#endif
				if(GetServerMode() == GM_LOGIN)
				{
					LOG(ERROR) << "DESTORY_GAMESCENE";
				}
				else
				{
					DESTORY_GAMESCENE;
				}
			}
		}
		
		//	Run
		if(!m_pWatcherThread->Run())
		{
			bRet = false;
		}
	}
		//////////////////////////////////////////////////////////////////////////
		PROTECT_END_VM

	return bRet;
}