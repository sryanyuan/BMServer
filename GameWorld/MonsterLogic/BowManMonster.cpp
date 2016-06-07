#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
int BowManMonster::GetRandomAbility(ABILITY_TYPE _type)
{
	int nValue = MonsterObject::GetRandomAbility(_type);

	if(_type == AT_DC)
	{
		if(m_pMaster)
		{
			int nSC = m_pMaster->GetRandomAbility(AT_SC);
			nSC /= 1.4f;
			const UserMagic* pMgc = NULL;
			if(m_pMaster->GetType() == SOT_HERO)
			{
				HeroObject* pHero = static_cast<HeroObject*>(m_pMaster);
				pMgc = pHero->GetUserMagic(MEFF_SUMMONBOWMAN);
				if(pMgc)
				{
					if(pMgc->bLevel > 0)
					{
						//	要消耗主人的能量
						int nCostMP = pMgc->bLevel * 5;
						if(pHero->GetObject_MP() > nCostMP)
						{
							pHero->DecMP(nCostMP);
							pHero->GetValidCheck()->DecMP(nCostMP);
							pHero->SyncMP(pHero);

							float fInc = 0.6 + (float)pMgc->bLevel / 10 * 3;
							nSC *= fInc;
							nValue += nSC;
						}
						else
						{
							return 0;
						}
					}
				}
			}
		}
	}
	return nValue;
}