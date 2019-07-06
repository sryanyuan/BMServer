#include "BroadcaseQueue.h"
#include <Windows.h>
#include <zlib.h>
#include "GlobalAllocRecord.h"

#include "../../CommonModule/SimpleActionHelper.h"
#include "../../CommonModule/ConsoleHelper.h"

const int nBroadcastIntervalMS = 100;

extern ConsoleHelper g_xConsole;

BroadcastQueue::BroadcastQueue() {
	m_uLastSendTime = 0;
}

BroadcastQueue::~BroadcastQueue() {

}

static char* pushVectorBytes(std::vector<char> &refVec, int _nCount) {
	char *pRes = nullptr;
	for (int i = 0; i < _nCount; i++) {
		refVec.push_back(0);
	}
	pRes = &refVec.back() - 3;
	return pRes;
}

void BroadcastQueue::Append(PkgObjectActionTurnNot &not) {
	m_listTurnActions.push_back(not);
}

void BroadcastQueue::Append(PkgObjectActionWalkNot &not) {
	m_listWalkActions.push_back(not);
}

void BroadcastQueue::Append(PkgObjectActionRunNot &not) {
	m_listRunActions.push_back(not);
}

int BroadcastQueue::Get(PkgGameObjectActionsNot &not, int *pLen) {
	int nCache = m_listWalkActions.size()  + m_listTurnActions.size() + m_listRunActions.size();
	unsigned int uCurTick = GetTickCount();
	if (uCurTick - m_uLastSendTime < nBroadcastIntervalMS) {
		return 0;
	}

	//not.vecActions.reserve(64);
	m_uLastSendTime = uCurTick;
	int nCount = 0;
	int nPlen = 0;
	
	if (!m_listWalkActions.empty()) {
		not.vecActions.reserve(9 * m_listWalkActions.size());
	}

	/*for (auto &action : m_listRunActions) {
		char *pBuffer = pushVectorBytes(not.vecActions, 4);
		memcpy(pBuffer, &action.uTargetId, 4);
		pBuffer = pushVectorBytes(not.vecActions, 4);
		memcpy(pBuffer, &action.uUserId, 4);
		pBuffer = pushVectorBytes(not.vecActions, 4);
		memcpy(pBuffer, &action.dwPos, 4);
		++nCount;
		nPlen += 16;
	}*/
	m_listRunActions.clear();

	for (auto &action : m_listWalkActions) {
		//not.vecActions.push_back(char(ObjectActionWalk));
		char *pBuffer = pushVectorBytes(not.vecActions, 4);
		memcpy(pBuffer, &action.uTargetId, 4);
		pBuffer = pushVectorBytes(not.vecActions, 4);
		memcpy(pBuffer, &action.uUserId, 4);
		// Left or right
		int nNextX = LOWORD(action.dwPos);
		int nNextY = HIWORD(action.dwPos);
		char cDir = 0;
		if (nNextX > SimpleActionHelper::GetCoordX(action)) {
			cDir |= 0x01;
		}
		else if (nNextX < SimpleActionHelper::GetCoordX(action)) {
			cDir |= 0x02;
		}
		if (nNextY > SimpleActionHelper::GetCoordY(action)) {
			cDir |= 0x04;
		}
		else if (nNextY < SimpleActionHelper::GetCoordY(action)) {
			cDir |= 0x08;
		}
		if (nCount % 2 == 0) {
			not.vecActions.push_back(cDir);
		}
		else {
			not.vecActions[not.vecActions.size() - 8 - 1] |= (cDir << 4);
		}

		++nCount;
		nPlen += 16;
	}
	m_listWalkActions.clear();

	/*for (auto &action : m_listTurnActions) {
		not.vecActions.push_back(char(ObjectActionTurn));
		char *pBuffer = pushVectorBytes(not.vecActions, 4);
		memcpy(pBuffer, &action.uTargetId, 4);
		pBuffer = pushVectorBytes(not.vecActions, 4);
		memcpy(pBuffer, &action.uUserId, 4);
		++nCount;
		nPlen += 12;
	}*/
	m_listTurnActions.clear();

	if (pLen != nullptr) {
		*pLen = nPlen;
	}

	if (0 != nCount) {
		// Do compress ?
		if (not.vecActions.size() > 96 && false) {
			static char *s_pBuf = new char[1024 * 256];
			GlobalAllocRecord::GetInstance()->RecordArray(s_pBuf);
			uLongf cmpsize = 1024;

			int nRet = compress((Bytef*)s_pBuf, &cmpsize, (const Bytef*)&not.vecActions.front(), not.vecActions.size());
			if (nRet == Z_OK && cmpsize < not.vecActions.size())
			{
				not.uUserId |= 0x01;
				not.vecActions.resize(cmpsize);
				memcpy(&not.vecActions.front(), s_pBuf, cmpsize);
			}
		}
	}
	return nCount;
}
