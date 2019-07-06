#include "HumDamCalc.h"
#include <Windows.h>

// 10 seconds
static const unsigned int  s_nHumDamContinueTime = 10000;

HumDamCalc::HumDamCalc() {

}

HumDamCalc::~HumDamCalc() {

}

void HumDamCalc::AddDam(int nUid, int nDam) {
	unsigned int uCur = GetTickCount();
	auto it = m_xDamRecord.find(nUid);
	if (it == m_xDamRecord.end()) {
		// Add new record
		HumDamRecord record;
		record.uLastHitTime = uCur;
		record.nDamTotal = nDam;
		m_xDamRecord.insert(std::make_pair(nUid, record));
		return;
	}
	if (uCur - it->second.uLastHitTime > s_nHumDamContinueTime) {
		// Invalid the record
		it->second.uLastHitTime = uCur;
		it->second.nDamTotal = nDam;
	}
	else {
		// Increase the dam
		it->second.uLastHitTime = uCur;
		it->second.nDamTotal += nDam;
	}
}

int HumDamCalc::GetMaxDamUid() {
	int nMaxUid = 0;
	int nMaxDam = 0;

	for (auto &pa : m_xDamRecord) {
		if (pa.second.nDamTotal > nMaxDam) {
			nMaxUid = pa.first;
			nMaxDam = pa.second.nDamTotal;
		}
	}

	if (0 == nMaxUid) {
		return 0;
	}

	m_xDamRecord.erase(nMaxUid);
	return nMaxUid;
}
