#include "../MonsterObject.h"
#include "../../GameWorld/GameSceneManager.h"
//////////////////////////////////////////////////////////////////////////
KulouMonster::KulouMonster()
{
	m_stData.eGameState = OS_SHOW;
	m_dwLastShowTime = GetTickCount();
}

KulouMonster::~KulouMonster()
{

}

//////////////////////////////////////////////////////////////////////////
void KulouMonster::DoAction(unsigned int _dwTick)
{
	if(m_stData.eGameState == OS_SHOW)
	{
		//
		if(m_dwCurrentTime - m_dwLastShowTime >= 1500)
		{
			m_stData.eGameState = OS_STAND;
			m_dwLastShowTime = _dwTick;
		}
	}
	else
	{
		__super::DoAction(_dwTick);
	}
}
//////////////////////////////////////////////////////////////////////////
int KulouMonster::GetRandomAbility(ABILITY_TYPE _type)
{
	int nValue = __super::GetRandomAbility(_type);

	if(_type == AT_DC)
	{
		if(m_pMaster)
		{
			int nSC = m_pMaster->GetRandomAbility(AT_SC);
			const UserMagic* pMgc = NULL;
			if(m_pMaster->GetType() == SOT_HERO)
			{
				HeroObject* pHero = static_cast<HeroObject*>(m_pMaster);
				pMgc = pHero->GetUserMagic(MEFF_SUMMON);
				if(pMgc)
				{
					if(pMgc->bLevel > 0)
					{
						float fInc = 0.3 + (float)pMgc->bLevel / 10;
						if(fInc > 1.0f)
						{
							fInc = 1.0f;
						}

						nSC *= fInc;
						nValue += nSC;
					}
				}
			}
		}
	}
	return nValue;
}