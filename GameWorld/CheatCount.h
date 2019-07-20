#ifndef _INC_CHEATCOUNT_
#define _INC_CHEATCOUNT_

#include <windows.h>

class CheatCount {
public:
	CheatCount(int nLimit) {
		m_nLimit = nLimit;
		m_nCheatCount = m_uLastCheatTime = 0;
	}
	~CheatCount() {

	}

public:
	void IncCount() {
		unsigned int uTick = GetTickCount();
		if (uTick - m_uLastCheatTime > 120 * 1000) {
			m_nCheatCount = 0;
		}
		++m_nCheatCount;
		m_uLastCheatTime = uTick;
	}
	bool ReachLimit() {
		return m_nCheatCount >= m_nLimit;
	}
	void Reset() {
		m_nCheatCount = 0;
	}

public:
	unsigned int m_uLastCheatTime;
	int m_nCheatCount;
	int m_nLimit;
};

#endif
