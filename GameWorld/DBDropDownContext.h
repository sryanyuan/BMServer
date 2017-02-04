#ifndef _INC_DBDROPDOWNCONTEXT_
#define _INC_DBDROPDOWNCONTEXT_
//////////////////////////////////////////////////////////////////////////
#include "DBThread.h"
#include <vector>
typedef std::vector<DWORD> DropPosVector;
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

	GroundItem* NewGroundItem(int _nItemID, int _nPosX, int _nPosY);

protected:
	DBOperationParam* m_pParam;
	DropPosVector m_xDropPosVector;
};
//////////////////////////////////////////////////////////////////////////
#endif