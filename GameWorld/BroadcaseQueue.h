#ifndef _INC_BROADCAST_QUEUE_
#define _INC_BROADCAST_QUEUE_

#include <list>
#include "../../CommonModule/GamePacket.h"

class BroadcastQueue {
public:
	BroadcastQueue();
	~BroadcastQueue();

public:
	void Append(PkgObjectActionRunNot &not);
	void Append(PkgObjectActionWalkNot &not);
	void Append(PkgObjectActionTurnNot &not);

	int Get(PkgGameObjectActionsNot &not, int *pLen);

private:
	std::list<PkgObjectActionRunNot> m_listRunActions;
	std::list<PkgObjectActionWalkNot> m_listWalkActions;
	std::list<PkgObjectActionTurnNot> m_listTurnActions;
	unsigned int m_uLastSendTime;
};

#endif
