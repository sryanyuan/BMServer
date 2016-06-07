#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
BlueGhasterMonster::BlueGhasterMonster()
{

}

void BlueGhasterMonster::OnMonsterDead(HeroObject* _pAttacher, bool _bKilledBySlave)
{
	GameObjectList xAllObjs;

	if(GetLocateScene())
	{
		GetLocateScene()->GetMapObjects(GetUserData()->wCoordX, GetUserData()->wCoordY, 1, xAllObjs);

		if(!xAllObjs.empty())
		{
			GameObjectList::iterator begIter = xAllObjs.begin();
			GameObjectList::const_iterator endIter = xAllObjs.end();

			for(begIter;
				begIter != endIter;
				++begIter)
			{
				GameObject* pObj = *begIter;

				if(pObj &&
					pObj != this)
				{
					if(pObj->GetType() == SOT_HERO)
					{
						if(GetObject_ID() == 123)
						{
							pObj->ReceiveDamage(this, IsMagicAttackMode(), pObj->GetObject_HP() / 2.5);

							pObj->SetEffStatus(MMASK_LVDU, 15000, 6);

							PkgPlayerSetEffectAck ack;
							ack.uTargetId = pObj->GetID();
							ack.dwMgcID = MMASK_LVDU;
							ack.bShow = true;
							ack.dwTime = 15 * 1000;
							g_xThreadBuffer.Reset();
							g_xThreadBuffer << ack;
							GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);

							//static_cast<HeroObject*>(pObj)->SendStatusInfo();
						}
						else
						{
							pObj->ReceiveDamage(this, IsMagicAttackMode(), GetLastRecvDamage() * 1.5f);
						}
					}
					else if(pObj->GetType() == SOT_MONSTER)
					{
						MonsterObject* pMons = (MonsterObject*)pObj;
						if(pMons->GetMaster() != NULL)
						{
							if(GetObject_ID() == 123)
							{
								pObj->ReceiveDamage(this, IsMagicAttackMode(), pObj->GetObject_HP() / 2.5);

								pObj->SetEffStatus(MMASK_LVDU, 15000, 6);

								PkgPlayerSetEffectAck ack;
								ack.uTargetId = pObj->GetID();
								ack.dwMgcID = MMASK_LVDU;
								ack.bShow = true;
								ack.dwTime = 15 * 1000;
								g_xThreadBuffer.Reset();
								g_xThreadBuffer << ack;
								GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
							}
							else
							{
								pObj->ReceiveDamage(this, IsMagicAttackMode(), GetLastRecvDamage() * 1.5f);
							}
						}
					}
				}
			}
		}
	}

	__super::OnMonsterDead(_pAttacher, _bKilledBySlave);
}