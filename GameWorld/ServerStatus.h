#ifndef _INC_MONITORTHREAD_
#define _INC_MONITORTHREAD_

#include <map>
#include <atomic>

enum {
	ServerStatusHeroAlloc,
	ServerStatusHeroFree,
	ServerStatusMonsAlloc,
	ServerStatusMonsFree,
	ServerStatusNPCAlloc,
	ServerStatusNPCFree,
	ServerStatusSceneAlloc,
	ServerStatusSceneFree,
	ServerStatusTotal,
};

class ServerStatus {
public:
	static ServerStatus* GetInstance(bool _bDeleteInstance = false) {
		static ServerStatus* pIns = new ServerStatus;

		if (_bDeleteInstance) {
			delete pIns;
			pIns = nullptr;
		}
		return pIns;
	}

public:
	ServerStatus();
	~ServerStatus();

	void SetVal(int _nStatusKey, long long _nVal);
	void IncVal(int _nStatusKey);

private:
	std::atomic<long long>* GetStatusVal(int _nStatusKey);

private:
	std::atomic<long long> m_serverStatusVals[ServerStatusTotal];
};

#endif
