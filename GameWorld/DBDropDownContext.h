#ifndef _INC_DBDROPDOWNCONTEXT_
#define _INC_DBDROPDOWNCONTEXT_
//////////////////////////////////////////////////////////////////////////
#include "DBThread.h"
#include <vector>
typedef std::vector<unsigned int> DropPosVector;
//////////////////////////////////////////////////////////////////////////
class GameScene;
//////////////////////////////////////////////////////////////////////////
class DBDropDownContext
{
public:
	DBDropDownContext(DBOperationParam* _pParam);

public:
	bool IsDropValid();

	float GetDropMultiple();

	int GetMagicDropMultiple();

	GameScene* GetDropScene();

	int GetDropBasePosX();
	int GetDropBasePosY();

	int InitDropPosition(int _nItems);
	int GetDropPosX(int _nItemIndex);
	int GetDropPosY(int _nItemIndex);

	int GetAdditionPoint();

	int GetOwnerID();

	GroundItem* NewGroundItem(int _nItemID, int _nPosX, int _nPosY, int _nOwner);

protected:
	DBOperationParam* m_pParam;
	DropPosVector m_xDropPosVector;
};
//////////////////////////////////////////////////////////////////////////
#endif