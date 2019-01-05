//#include <afx.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <Shlwapi.h>
#endif
#include "../IOServer/SServerEngine.h"
#include "../CMainServer/CMainServer.h"
#include "../common/glog.h"
#include "../GameWorld/GameWorld.h"
#include "../GameWorld/GameSceneManager.h"
#include "../GameWorld/GameDbBuffer.h"
#include "../GameWorld/ExceptionHandler.h"
#include "../../CommonModule/ZipDataReader.h"
#include "../../CommonModule/SettingLoader.h"
#include "../../CommonModule/CommandLineHelper.h"
#include "../../CommonModule/LoginExtendInfoParser.h"
#include "../GameWorld/MonsterObject.h"
#include "../GameWorld/ConnCode.h"
#include "../../CommonModule/DataEncryptor.h"
#include "../../CommonModule/loginsvr.pb.h"
#include "../../CommonModule/ProtoType.h"
#include "../runarg.h"
#include "../GameWorld/GlobalAllocRecord.h"
#include "../GameWorld/TeammateControl.h"
#include "../../CommonModule/version.h"
#include "CMainServerMacros.h"
#include "../Interface/ServerShell.h"
#include "../common/cmsg.h"
#include "../Helper.h"

#include <direct.h>
#include <zlib.h>
#include <time.h>
#include <io.h>

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
	InitializeCriticalSection(&m_csNetThreadEventList);
	m_pServerShell = nullptr;
	m_pIOServer = nullptr;
}

CMainServer::~CMainServer()
{
	if(m_pIOServer != nullptr)
	{
		delete m_pIOServer;
		m_pIOServer = NULL;
	}
	GameWorld::DestroyInstance();
	GameSceneManager::DestroyInstance();

	DeleteCriticalSection(&m_csNetThreadEventList);
	/*m_pWatcherThread->Stop();
	delete m_pWatcherThread;*/
}


//////////////////////////////////////////////////////////////////////////

SServerEngine* CMainServer::GetIOServer()
{
	return m_pIOServer;
}

ServerShell* CMainServer::GetServerShell() {
	return m_pServerShell;
}

void CMainServer::SetServerShell(ServerShell *_pServerShell) {
	m_pServerShell = _pServerShell;
}

void CMainServer::AddInformationToMessageBoard(const char* fmt, ...) {
	if (nullptr == m_pServerShell) {
		return;
	}

	char buffer[MAX_PATH];

	va_list args;
	va_start(args, fmt);
	_vsnprintf(buffer, sizeof(buffer), fmt, args);

	SYSTEMTIME lpTime;
	GetLocalTime(&lpTime);

	char logTime[MAX_PATH];
	sprintf(logTime, "[%d-%d-%d %02d:%02d:%02d] ", lpTime.wYear, lpTime.wMonth, lpTime.wDay, lpTime.wHour, lpTime.wMinute, lpTime.wSecond);

	strcat(logTime, buffer);

	m_pServerShell->AddInformation(logTime);
}

/************************************************************************/
/* 初始化网络
/************************************************************************/
bool CMainServer::InitNetWork()
{
	//	Get global settings
	char szPath[MAX_PATH];
	if (NULL == GetRunArg("cfgfile") ||
		strlen(GetRunArg("cfgfile")) == 0)
	{
		sprintf(szPath, "%s\\conf\\cfg.ini", GetRootPath());
	}
	else
	{
		sprintf(szPath, "%s\\conf\\%s", GetRootPath(), GetRunArg("cfgfile"));
	}
	g_xConsole.CPrint("Using server config file : %s", szPath);
	SettingLoader::GetInstance()->LoadSetting(szPath);
	GameWorld::GetInstancePtr()->SetGenElitMons((SettingLoader::GetInstance()->GetIntValue("GENELITEMONS") != 0));
	GameWorld::GetInstancePtr()->SetEnableOfflineSell((SettingLoader::GetInstance()->GetIntValue("OFFLINESELL") != 0));
	GameWorld::GetInstancePtr()->SetEnableWorldNotify((SettingLoader::GetInstance()->GetIntValue("WORLDNOTIFY") != 0));

	// Create IOServer
	{
		WSADATA data = {0};
		if (0 != WSAStartup(MAKEWORD(2, 2), &data))
		{
			LOG(FATAL) << "WSASTARTUP ERROR";
			return false;
		}
		m_pIOServer = new SServerEngine;
	}

	// Initialize IOServer
	SServerInitDesc serverInitDesc;
	memset(&serverInitDesc, 0, sizeof(serverInitDesc));
	serverInitDesc.pFuncOnAcceptUser = (FUNC_ONACCEPT)_OnAcceptUser;
	serverInitDesc.pFuncOnDisconnctedUser = (FUNC_ONDISCONNECTED)_OnDisconnectUser;
	serverInitDesc.pFuncOnRecvUser = (FUNC_ONRECV)_OnRecvFromUserTCP;
	serverInitDesc.pFuncOnAcceptServer = (FUNC_ONACCEPT)_OnAcceptServer;
	serverInitDesc.pFuncOnDisconnctedServer = (FUNC_ONDISCONNECTED)_OnDisconnectServer;
	serverInitDesc.pFuncOnRecvServer = (FUNC_ONRECV)_OnRecvFromServerTCP;
	serverInitDesc.uMaxConnUser = MAX_USER_NUMBER;

	if (kSServerResult_Ok != m_pIOServer->Init(&serverInitDesc))
	{
		//	LOG??
		LOG(FATAL) << "网络引擎创建失败";
		return false;
	}
	else
	{
		LOG(INFO) << "网络引擎创建成功";
	}

	// Add timer job
	m_pIOServer->AddTimerJob(0, 10, FUNC_ONTIMER(&CMainServer::_OnGameLoop));

	return true;
}

/************************************************************************/
/* 启动游戏服务器
/************************************************************************/
bool CMainServer::StartServer(char* _szIP, WORD _wPort)
{
	if(nullptr == m_pIOServer)
	{
		return false;
	}

	if (kSServerResult_Ok != m_pIOServer->Start(_szIP, _wPort))
	{
		//	LOG??
		LOG(ERROR) << "地址[" << _szIP << "]:[" << _wPort << "]启动服务器失败";
		return false;
	}

	AddInformationToMessageBoard("服务端版本: %s", BACKMIR_CURVERSION);

	// Initialize game world
	if (0 != GameWorld::GetInstance().Init())
	{
		LOG(ERROR) << "初始化游戏世界失败";
		return false;
	}

	//	Initialize game scene
	if(!GameSceneManager::GetInstance()->CreateAllScene())
	{
		LOG(ERROR) << "载入地图场景失败";
		return false;
	}
	else
	{
		AddInformationToMessageBoard("游戏地图场景成功加载");
	}

	//	Run the Game world
	if(!GameWorld::GetInstance().Run())
	{
		return false;
	}
	else
	{
		AddInformationToMessageBoard("游戏世界成功启动");
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

			AddInformationToMessageBoard("开始尝试连接登录服务器：%s:%d", szIP, nPort);
			return m_pIOServer->SyncConnect(szIP, nPort, 
				FUNC_ONCONNECTSUCCESS(&CMainServer::_OnLsConnSuccess), 
				FUNC_ONCONNECTFAILED(&CMainServer::_OnLsConnFailed), 
				NULL) == kSServerResult_Ok ? true : false;
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
	CMainServer::GetInstance()->AddInformationToMessageBoard("连接登陆服务器成功");

	const char* pszLsAddr = GetRunArg("outerip");
	const char* pszServerId = GetRunArg("serverid");
	const char* pszServerName = GetRunArg("servername");

	if(NULL == pszLsAddr)
	{
		return;
	}
	if (NULL == pszServerId)
	{
		return;
	}

	// VerifyV1 
	ByteBuffer xBuf;
	xBuf.Reset();
	xBuf << int(0)
		<< (int)PKG_LOGIN_SERVERVERIFYV2_REQ
		<< (short)GetServerID()
		<< (int)0
		<< (char)strlen(pszLsAddr);
	xBuf.Write(pszLsAddr, strlen(pszLsAddr));
	xBuf << (char)strlen(pszServerName);
	xBuf.Write(pszServerName, strlen(pszServerName));
	SendBufferToServer(_dwIndex, &xBuf);

	//	发送登陆服务器注册成功事件
	MSG msgQuery;
	msgQuery.message = WM_LSCONNECTED;
	msgQuery.wParam = (WPARAM)_dwIndex;
	msgQuery.lParam = 0;
	GameWorld::GetInstancePtr()->PostRunMessage(&msgQuery);
}

void STDCALL CMainServer::_OnLsConnFailed(DWORD _dwIndex, void* _pParam)
{
	CMainServer::GetInstance()->m_bLoginConnected = false;
	CMainServer::GetInstance()->m_dwLsConnIndex = 0;
	CMainServer::GetInstance()->AddInformationToMessageBoard("连接登陆服务器失败");
	g_xConsole.CPrint("Login server address:%s, can't connect to.", CMainServer::GetInstance()->m_xLoginAddr.c_str());
}

/************************************************************************/
/* 停止游戏服务器
/************************************************************************/
void CMainServer::StopServer()
{
	if (nullptr != m_pIOServer)
	{
		delete m_pIOServer;
		m_pIOServer = nullptr;
	}
	LOG(INFO) << "服务器已停止";
	AddInformationToMessageBoard("服务器已停止");
	m_bMode = MODE_STOP;
	UpdateServerState();
}

void CMainServer::WaitForStopEngine()
{
	// 等待网络引擎停止 给网络线程中的GameWorld发送消息
	SServerEngine* pIOServer = GetIOServer();

	GameWorld* pWorld = GameWorld::GetInstancePtr();

	MSG msg = { 0 };
	msg.message = WM_STOPNETENGINE;
	msg.wParam = 0;
	msg.lParam = 0;
	pWorld->PostRunMessage(&msg);
	pIOServer->Join();
	//pWorld->Join();
	LOG(INFO) << "Net process stop";

	// Where we can destroy all game scene and game world
	GameSceneManager::GetInstance()->ReleaseAllScene();
	//GameSceneManager::DestroyInstance();
	//GameWorld::DestroyInstance();
	FreeListManager::GetInstance()->Clear();
	FreeListManager::GetInstance(true);
	SettingLoader::GetInstance()->Clear();
	SettingLoader::GetInstance(true);
	StoveManager::GetInstance(true);
	GameTeamManager::GetInstance(true);
	DBThread::GetInstance()->Stop();
	DBThread::GetInstance()->Join();
	DBThread::GetInstance(true);
	ReleaseGlobalSuitExtraAttrib();
	google::protobuf::ShutdownProtobufLibrary();
	GlobalAllocRecord::GetInstance()->DeleteAll();
	GlobalAllocRecord::DestroyInstance();
	// Stop watching thread
	m_pWatcherThread->Stop();
	m_pWatcherThread->Join();
	SAFE_DELETE(m_pWatcherThread);
}

void CMainServer::StopEngine()
{
	m_pIOServer->Stop();
}

//////////////////////////////////////////////////////////////////////////
bool CMainServer::InitDatabase()
{
	PROTECT_START_VM
	//	Get encrypt table
	ObjectValid::GenerateEncryptTable();
	//	First open database
	char szPath[MAX_PATH];
	GetRootPath(szPath, MAX_PATH);
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
		AddInformationToMessageBoard("读取装备价格成功");
	}
	else
	{
		AddInformationToMessageBoard("读取装备价格失败");
		bRet = false;
	}
	if(DBThread::GetInstance()->LoadMagicAttrib())
	{
		AddInformationToMessageBoard("读取魔法信息成功");
	}
	else
	{
		AddInformationToMessageBoard("读取魔法信息失败");
		bRet = false;
	}

	if(!CheckVersion())
	{
		AddInformationToMessageBoard("游戏已损坏");
		bRet = false;
	}

	if(!CreateGameDbBuffer())
	{
		AddInformationToMessageBoard("无法创建缓存");
		bRet = false;
	}

	EncryptGlobalValue();

	/*g_bEncryptSuitName = true;
	if(!InitItemExtraAttribForServer())
	{
		LOG(ERROR) << "无法读取套装信息";
	}

	//	Get item grade information
	if(!InitItemGradeForServer())
	{
		LOG(ERROR) << "无法读取装备额外信息";
	}*/
	
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

	SServerConn *pConn = m_pIOServer->GetUserConn(_dwIndex);
	if (nullptr != pConn) {
		pConn->GetAddress(szIP, &wPort);
	}
	AddInformationToMessageBoard("玩家[%s]:[%d]连接", szIP, wPort);
	LOG(INFO) << "玩家[" << szIP << "]:" << wPort << "连接, INDEX:[" << _dwIndex << "], conn code[" << dwConnCode << "]";

	// Add to distinct ip set
	std::map<std::string, int>::iterator it = m_xIPs.find(szIP);
	if (it == m_xIPs.end()) {
		m_xIPs.insert(std::make_pair(szIP, 1));
	}
	else {
		it->second++;
	}
	if (nullptr != m_pServerShell) {
		m_pServerShell->UpdateDistinctIPCount(m_xIPs.size());
	}

	UpdateServerState();

	PkgLoginGameTypeNot not;
	not.bType = PLGTN_GAMESERVER;
	not.dwConnIdx = _dwIndex;
	s_xMainTcpBuf.Reset();
	s_xMainTcpBuf << not;
	SendBuffer(_dwIndex, &s_xMainTcpBuf);
}

/************************************************************************/
/* 断开事件处理
/************************************************************************/
void CMainServer::OnDisconnectUser(DWORD _dwIndex)
{
	RECORD_FUNCNAME_SERVER;

	char szIP[20];
	WORD wPort = 0;
	SServerConn *pConn = m_pIOServer->GetUserConn(_dwIndex);
	if (nullptr != pConn) {
		pConn->GetAddress(szIP, &wPort);
		std::map<std::string, int>::iterator it = m_xIPs.find(szIP);
		if (it != m_xIPs.end()) {
			it->second--;
			if (0 == it->second) {
				m_xIPs.erase(it);
			}
		}
		// Update distinct ip count to server shell
		if (nullptr != m_pServerShell) {
			m_pServerShell->UpdateDistinctIPCount(m_xIPs.size());
		}
	}

	//	连接序号归位0
	SetConnCode(_dwIndex, 0);

	{
		if(_dwIndex <= MAX_CONNECTIONS)
		{
			if(g_pxHeros[_dwIndex] != NULL)
			{
				//AddInfomation("玩家[%s]断开", g_pxHeros[_dwIndex]->GetUserData()->stAttrib.name);
				//m_pxServer->GetUserAddress(_dwIndex, szIP, &wPort);
				//AddInfomation("玩家[%s]:[%d]断开", szIP, wPort);
				AddInformationToMessageBoard("玩家[%d]断开", _dwIndex);

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
			else if(dwOpCode == protocol::RegisterClientReq)
			{
				protocol::MRegisterClientReq req;
				if (!req.ParseFromArray(_xBuf->GetBuffer() + 8, _xBuf->GetLength() - 8))
				{
					LOG(ERROR) << "Failed to parse protobuf";
					return;
				}

				protocol::MUserInternalVerifyReq ir;
				ir.set_lid(req.lid());
				ir.set_gid(req.sid());
				ir.set_accesstoken(req.accesstoken());
				ir.set_conncode(GetConnCode(_dwIndex));
				SendProtoToServer(m_dwLsConnIndex, protocol::UserInternalVerifyReq, ir);
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

	if (dwOpCode >= protocol::LSOp_MIN &&
		dwOpCode <= protocol::LSOp_MAX)
	{
		if (GetProtoType() != ProtoType_Protobuf)
		{
			SetProtoType(ProtoType_Protobuf);
			g_xConsole.CPrint("Using protobuf protocol");
		}
	}

	if (ProtoType_Protobuf == GetProtoType())
	{
		OnRecvFromServerTCPProtobuf(_dwIndex, _xBuf);
		return;
	}

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
				try
				{
					pHero->DispatchPacket(s_xExtDataBuffer, pHeader);
				}
				catch(std::exception& e)
				{
					LOG(ERROR) << "!!!Packet unserialize failed while processing hero <" << pHero->GetName() << "> UID(" << pHero->GetUID() << ") packet <" << pHeader->uOp << "> , exception:" << e.what();
					CMainServer::GetInstance()->GetIOServer()->CloseUserConnection(_dwIndex);
				}
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

void CMainServer::OnRecvFromServerTCPProtobuf(DWORD _dwIndex, ByteBuffer* _xBuf)
{
	DWORD dwOpCode = GET_PACKET_OP((*_xBuf));
	g_xConsole.CPrint("Process protobuf : %d", dwOpCode);

	switch (dwOpCode)
	{
	case protocol::ProtoTypeNtf:
		{
			//	register server
			protocol::MRegisterServerReq req;
			const char* szServerName = GetRunArg("servername");
			if (NULL == szServerName)
			{
				szServerName = "Undefined";
			}
			req.set_servername(szServerName);
			req.set_exposeaddress(GetRunArg("outerip"));
			req.set_serverid(atoi(GetRunArg("serverid")));
			SendProtoToServer(_dwIndex, protocol::RegisterServerReq, req);
		}break;
	case protocol::PlayerLoginHumDataNtf:
		{
			//	recv hum data
			protocol::MPlayerLoginHumDataNtf ntf;
			if (!ntf.ParseFromArray(_xBuf->GetBuffer() + 8, _xBuf->GetLength() - 8))
			{
				LOG(ERROR) << "Failed to parse proto " << dwOpCode;
				return;
			}

			//	检查conn code
			if(GetConnCode(ntf.gid()) != ntf.connid())
			{
				LOG(ERROR) << "conn code not equal! index[" << ntf.gid() << "] " << GetConnCode(ntf.gid()) << "!=" << ntf.connid();
				return;
			}
			if(ntf.connid() == 0)
			{
				LOG(ERROR) << "conn code invalid.";
				return;
			}

			HeroHeader header;
			header.bJob = ntf.job();
			header.bSex = ntf.sex();
			header.uLevel = ntf.level();
			strcpy(header.szName, ntf.name().c_str());
			PkgUserLoginReq req;
			memcpy(&req.stHeader, &header, sizeof(HeroHeader));
			if (ntf.data().size() != 0)
			{
				req.xData.resize(ntf.data().size());
				memcpy(&req.xData[0], &ntf.data()[0], ntf.data().size());
			}
			OnPreProcessPacket(ntf.gid(), ntf.lid(), ntf.uid(), ntf.jsondata().c_str(), req);

			//
			GameWorld::GetInstance().DisableAutoReset();
		}break;
	case protocol::SyncPlayerRankNtf:
		{
			//	recv rank
			protocol::MSyncPlayerRankNtf ntf;
			if (!ntf.ParseFromArray(_xBuf->GetBuffer() + 8, _xBuf->GetLength() - 8))
			{
				LOG(ERROR) << "Failed to parse proto " << dwOpCode;
				return;
			}

			//	post 给游戏线程
			char* pszRankCopy = new char[ntf.data().size() + 1];
			strcpy(pszRankCopy, ntf.data().c_str());
			MSG msgQuery;
			msgQuery.message = WM_PLAYERRANKLIST;
			msgQuery.wParam = (WPARAM)pszRankCopy;
			msgQuery.lParam = 0;

			if (GameWorld::GetInstance().GetThreadRunMode()) {
				GameWorld::GetInstancePtr()->PostRunMessage(&msgQuery);
			} else {
				GameWorld::GetInstance().Thread_ProcessMessage(&msgQuery);
			}
		}break;
	default:
		{
			g_xConsole.CPrint("Unknown protobuf opcode : %d", dwOpCode);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void CMainServer::ForceCloseConnection(DWORD _dwIndex)
{
	m_pIOServer->CloseUserConnection(_dwIndex);
}

/************************************************************************/
/* 初始化日志
/************************************************************************/
bool CMainServer::InitLogFile()
{
	char szFilePath[MAX_PATH];
	GetRootPath(szFilePath, MAX_PATH);

	google::InitGoogleLogging(szFilePath);
	PathRemoveFileSpec(szFilePath);
	strcat(szFilePath, "\\serverlog\\");

	std::list<std::string> xFiles;
	char szDetectFile[MAX_PATH];
	strcpy(szDetectFile, szFilePath);
	strcat(szDetectFile, "*.*");

	intptr_t handle;
	_finddata_t findData;
	handle = _findfirst(szDetectFile, &findData);
	if (-1 != handle) {
		do 
		{
			if ((findData.attrib & _A_SUBDIR) == 0) {
				// Not a sub dir
				char szFullPath[MAX_PATH];
				sprintf(szFullPath, "%s%s", szFilePath, findData.name);
				xFiles.emplace_back(szFullPath);
			}
		} while (_findnext(handle, &findData) == 0);
		_findclose(handle);
		handle = 0;
	}

	if(xFiles.size() > 10)
	{
		for(std::list<std::string>::iterator iter = xFiles.begin();
			iter != xFiles.end();
			++iter)
		{
			remove((*iter).c_str());
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
			if(!CMainServer::GetInstance()->m_xLoginAddr.empty() &&
				NULL != GetRunArg("serverid"))
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
	} else {
		// process stop engine msg
		GameWorld::GetInstance().ProcessThreadMsg();
	}

	ASSERT(_CrtCheckMemory());
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
					AddInformationToMessageBoard("玩家[%d]小退", dwIndex);

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
		if(dwOpCode >= protocol::LSOp_MAX)
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
		CMainServer::GetInstance()->GetIOServer()->CloseUserConnection(_dwIndex);
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

bool CMainServer::CreateLoginHero(HeroObject* _pHero, 
	HeroHeader& _refHeroHeader, 
	vector<char>& _refLoginData, 
	LoginExtendInfoParser& _refLoginExt,
	std::string _refErrMsg) {
	HeroObject *pObj = _pHero;
	UserData* pUserData = _pHero->GetUserData();

	// Set header fields to hum data
	pUserData->bJob = _refHeroHeader.bJob;
	pUserData->stAttrib.sex = _refHeroHeader.bSex;
	strcpy(pUserData->stAttrib.name, _refHeroHeader.szName);

	if (_refLoginData.empty()) {
		// The hero is a new hero, initialize it as a born child
		pObj->SetMapID(0);
		pObj->GetUserData()->wCoordX = 20;
		pObj->GetUserData()->wCoordY = 16;
		// Initialize level
		ItemAttrib* pAttrib = &pUserData->stAttrib;
		pAttrib->level = 1;

		pAttrib->HP = GetHeroBaseAttribHP(pAttrib->level, pObj->GetUserData()->bJob);
		pAttrib->maxHP = pAttrib->HP;
		pAttrib->MP = GetHeroBaseAttribMP(pAttrib->level, pObj->GetUserData()->bJob);
		pAttrib->maxMP = pAttrib->MP;
		pAttrib->EXPR = 0;
		pAttrib->maxEXPR = GetHeroBaseAttribExpr(pAttrib->level);

		return true;
	}

	// Initialize hero with saved data
	const char* pData = &_refLoginData[0];
	DWORD dwDataLen = _refLoginData.size();

	static char* s_pBuf = new char[MAX_SAVEDATA_SIZE];
	// Add to global static pointer list
	GlobalAllocRecord::GetInstance()->RecordArray(s_pBuf);
	uLongf buflen = MAX_SAVEDATA_SIZE;
	uLongf srclen = dwDataLen;
	int nRet = uncompress((Bytef*)s_pBuf, &buflen, (const Bytef*)pData, srclen);
	if (Z_OK != nRet) {
		sprintf(s_pBuf, "玩家[%s]存档数据错误", _refHeroHeader.szName);
		_refErrMsg = s_pBuf;
		return false;
	}

	if (g_xMainBuffer.GetLength() > buflen)
	{
		LOG(ERROR) << "Buffer overflow.Too large hum data size:" << buflen;
		return false;
	}

	g_xMainBuffer.Reset();
	g_xMainBuffer.Write(s_pBuf, buflen);

	USHORT uVersion = 0;
	g_xMainBuffer >> uVersion;

	// Check save version
	if (uVersion < BACKMIR_VERSION208 || uVersion > BACKMIR_VERSION210) {
		sprintf(s_pBuf, "玩家[%s]不支持的存档版本%d", _refHeroHeader.szName, uVersion);
		_refErrMsg = s_pBuf;
		return false;
	}

	const char s_pszArchive[] = "Archive version:";
	const char s_pszInvalid[] = "Invalid archive";
	const char s_pszUserLogin[] = "User login:";
	const char s_pszUnsupportVersion[] = "Unsupport version:";
	char szText[MAX_PATH];

	sprintf(szText, "%s[%d]", s_pszArchive, uVersion);
	AddInformationToMessageBoard(szText);
	pObj->SetVersion(uVersion);

	// Apply hum save data
	if (!LoadHumData(_pHero, g_xMainBuffer, uVersion)) {
		sprintf(s_pBuf, "玩家[%s]读取存档数据错误", _refHeroHeader.szName);
		_refErrMsg = s_pBuf;
		return false;
	}
	return true;
}

bool CMainServer::LoadHumData(HeroObject *_pHero, ByteBuffer& _xBuf, USHORT _uVersion) {
	UserData* pUserData = _pHero->GetUserData();

	// >= 208
	if (_xBuf.GetLength() >= 17)
	{
		_xBuf >> pUserData->stAttrib.level;
		_xBuf >> pUserData->bJob;
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

	// >= 210
	if (_uVersion < BACKMIR_VERSION210) {
		return true;
	}
	//	save extend fields
	if (!_pHero->ReadSaveExtendField(_xBuf))
	{
		return false;
	}

	return true;
}

bool CMainServer::OnPlayerRequestLogin(DWORD _dwIndex, DWORD _dwLSIndex, DWORD _dwUID, const char* _pExtendInfo, PkgUserLoginReq& req) {
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

	// Create a new hero object and check valid
	HeroObject *pObj = new HeroObject(_dwIndex);
	// In login game server mode, apply the login server context
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
	// Create hero with quest data
	std::string xErrMsg;
	if (!CreateLoginHero(pObj, req.stHeader, req.xData, xLoginInfoParser, xErrMsg)) {
		SAFE_DELETE(pObj);
		LOG(ERROR) << "Create login hero[" << req.stHeader.szName << "] failed";
		if (!xErrMsg.empty()) {
			AddInformationToMessageBoard(xErrMsg.c_str());
		}
		return false;
	}
	// Add it to game world
	if (0 != GameWorld::GetInstance().SyncOnHeroConnected(pObj, req.xData.empty())) {
		// Add failed
		SAFE_DELETE(pObj);
		g_pxHeros[_dwIndex] = NULL;
		LOG(ERROR) << "Add hero[" << req.stHeader.szName << "] to game world failed";
		return false;
	}

	// Record the player in the global table
	g_pxHeros[_dwIndex] = pObj;

	// Set mapped key
	++m_dwUserNumber;
	UpdateServerState();

	AddInformationToMessageBoard("玩家[%s]登入游戏",
		req.stHeader.szName);
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

unsigned int CMainServer::SendBuffer(unsigned int _nIdx, ByteBuffer* _pBuf)
{
	SServerEngine* pNet = CMainServer::GetInstance()->GetIOServer();
	if (NULL == pNet)
	{
		return 0;
	}

	if (_pBuf->GetLength() == 0)
	{
		return 0;
	}

	DWORD dwPacketLength = _pBuf->GetLength();
	unsigned char* pBuf = const_cast<unsigned char*>(_pBuf->GetBuffer());
	*(DWORD*)pBuf = dwPacketLength;

	if (TRUE == pNet->SyncSendPacketToUser(_nIdx, (char*)_pBuf->GetBuffer() + sizeof(unsigned int), dwPacketLength - 4))
	{
#ifdef _VIEW_PACKET
		LOG(INFO) << _pBuf->ToHexString() << "sended";
#endif
		return dwPacketLength;
	}
	return 0;
}

unsigned int CMainServer::SendBufferToServer(unsigned int _nIdx, ByteBuffer* _pBuf)
{
	SServerEngine* pNet = CMainServer::GetInstance()->GetIOServer();
	if (NULL == pNet)
	{
		return 0;
	}

	if (_pBuf->GetLength() == 0)
	{
		return 0;
	}

	DWORD dwPacketLength = _pBuf->GetLength();
	unsigned char* pBuf = const_cast<unsigned char*>(_pBuf->GetBuffer());
	*(DWORD*)pBuf = dwPacketLength;

	if (TRUE == pNet->SyncSendPacketToServer(_nIdx, (char*)_pBuf->GetBuffer() + sizeof(unsigned int), dwPacketLength - 4))
	{
#ifdef _VIEW_PACKET
		LOG(INFO) << _pBuf->ToHexString() << "sended";
#endif
		return dwPacketLength;
	}
	return 0;
}

unsigned int CMainServer::SendProtoToServer(unsigned int _nIdx, int _nCode, google::protobuf::Message& _refMsg)
{
	SServerEngine* pNet = CMainServer::GetInstance()->GetIOServer();
	if (NULL == pNet)
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

	if (TRUE == pNet->SyncSendPacketToServer(_nIdx, (char*)s_bytesBuffer, 4 + nSize))
	{
		return 4 + nSize;
	}

	return 0;
}