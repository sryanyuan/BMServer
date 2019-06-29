#include "ServerStatus.h"

ServerStatus::ServerStatus() {
	
}

ServerStatus::~ServerStatus() {
	
}

std::atomic<long long>* ServerStatus::GetStatusVal(int _nStatusKey) {
	if (_nStatusKey < 0 || _nStatusKey >= ServerStatusTotal) {
		return nullptr;
	}
	return &m_serverStatusVals[_nStatusKey];
}

void ServerStatus::SetVal(int _nStatusKey, long long _nVal) {
	auto av = GetStatusVal(_nStatusKey);
	if (nullptr != av) {
		av->store(_nVal);
	}
}

void ServerStatus::IncVal(int _nStatusKey) {
	auto av = GetStatusVal(_nStatusKey);
	if (nullptr != av) {
		av->fetch_add(1);
	}
}