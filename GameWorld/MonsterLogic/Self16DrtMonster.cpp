#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
bool Self16DrtMonster::CanAttack()
{
	if(m_pTarget == NULL)
	{
		return false;
	}

	int nOftX = m_pTarget->GetUserData()->wCoordX - m_stData.wCoordX;
	int nOftY = m_pTarget->GetUserData()->wCoordY - m_stData.wCoordY;

	if(nOftX == 0 &&
		nOftY == 0)
	{
		return false;
	}

	if(abs(nOftX) <= GetViewRange() &&
		abs(nOftY) <= GetViewRange())
	{
		return true;
	}

	return false;
}