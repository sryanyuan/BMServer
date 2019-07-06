#ifndef _INC_HUMDAMCALC_
#define _INC_HUMDAMCALC_

#include <map>

struct HumDamRecord {
	unsigned int uLastHitTime;
	int nDamTotal;
};
typedef std::map<int, HumDamRecord> HumDamRecordMap;

class HumDamCalc {
public:
	HumDamCalc();
	~HumDamCalc();

public:
	void AddDam(int nUid, int nDam);
	// Find the max uid and POP
	int GetMaxDamUid();

private:
	HumDamRecordMap m_xDamRecord;
};

#endif