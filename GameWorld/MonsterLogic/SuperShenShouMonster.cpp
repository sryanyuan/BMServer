#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
int SuperShenShouMonster::GetRandomAbility(ABILITY_TYPE _type)
{
	int nValue = MonsterObject::GetRandomAbility(_type);

	if(_type == AT_DC)
	{
		if(m_pMaster)
		{
			int nSC = m_pMaster->GetRandomAbility(AT_SC);
			const UserMagic* pMgc = NULL;
			if(m_pMaster->GetType() == SOT_HERO)
			{
				HeroObject* pHero = static_cast<HeroObject*>(m_pMaster);
				pMgc = pHero->GetUserMagic(MEFF_SUPERSUMMON);
				if(pMgc)
				{
					if(pMgc->bLevel > 0)
					{
						float fInc = 0.3 + (float)pMgc->bLevel / 10 * 2;
						if(fInc >= 0.9f)
						{
							fInc = 0.9f;
						}

						nSC *= fInc;
						nValue += nSC;
					}
				}

				int nAddShenShou = pHero->GetStateController()->GetAddShenShouAddition();
				if(nAddShenShou > 0)
				{
					float fMulti = 1 + (float)nAddShenShou / 10;
					nValue *= fMulti;
				}
			}
		}
	}
	return nValue;
}