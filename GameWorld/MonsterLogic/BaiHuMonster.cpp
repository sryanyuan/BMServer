#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
BaiHuMonster::BaiHuMonster()
{

}

BaiHuMonster::~BaiHuMonster()
{

}

int BaiHuMonster::GetRandomAbility(ABILITY_TYPE _type)
{
	int nValue = MonsterObject::GetRandomAbility(_type);

	if(_type == AT_DC)
	{
		if(m_pMaster)
		{
			int nSC = m_pMaster->GetRandomAbility(AT_SC);
			nSC /= 1.2f;
			const UserMagic* pMgc = NULL;
			if(m_pMaster->GetType() == SOT_HERO)
			{
				HeroObject* pHero = static_cast<HeroObject*>(m_pMaster);
				pMgc = pHero->GetUserMagic(MEFF_SUMMONTIGER);
				if(pMgc)
				{
					if(pMgc->bLevel > 0)
					{
						float fInc = 0.2 + (float)pMgc->bLevel / 10;
						if(fInc >= 0.5f)
						{
							fInc = 0.5f;
						}

						nSC *= fInc;
						nValue += nSC;
					}
				}

				int nTigerAddition = pHero->GetStateController()->GetAddTigerAddition();
				if(nTigerAddition != 0)
				{
					float fMulti = (float)nTigerAddition / 10.0f + 1.0f;
					nValue *= fMulti;
				}
			}
		}
	}
	return nValue;
}