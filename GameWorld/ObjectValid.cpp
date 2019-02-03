#include "ObjectValid.h"
#include "ObjectEngine.h"
#include "GameWorld.h"
//////////////////////////////////////////////////////////////////////////
unsigned int EAI_ID_MASK = 0;
unsigned char EAI_LUCKY_MASK = 0;
unsigned char EAI_CURSE_MASK = 0;
unsigned char EAI_HIDE_MASK = 0;
unsigned char EAI_ACCURACY_MASK = 0;
unsigned char EAI_ATKSPEED_MASK = 0;
unsigned char EAI_ATKPALSY_MASK = 0;
unsigned char EAI_ATKPOIS_MASK = 0;
unsigned char EAI_MOVESPEED_MASK = 0;
unsigned char EAI_WEIGHT_MASK = 0;
unsigned short EAI_MAXDC_MASK = 0;
unsigned short EAI_DC_MASK = 0;
unsigned short EAI_MAXAC_MASK = 0;
unsigned short EAI_AC_MASK = 0;
unsigned short EAI_MAXMAC_MASK = 0;
unsigned short EAI_MAC_MASK = 0;
unsigned short EAI_MAXSC_MASK = 0;
unsigned short EAI_SC_MASK = 0;
unsigned short EAI_MAXMC_MASK = 0;
unsigned short EAI_MC_MASK = 0;
unsigned short EAI_MAXHP_MASK = 0;
unsigned short EAI_HP_MASK = 0;
unsigned short EAI_MAXMP_MASK = 0;
unsigned short EAI_MP_MASK = 0;
unsigned int EAI_MAXEXPR_MASK = 0;
unsigned int EAI_EXPR_MASK = 0;
unsigned short EAI_LEVEL_MASK = 0;
unsigned char EAI_REQVALUE_MASK = 0;
unsigned char EAI_REQTYPE_MASK = 0;
unsigned char EAI_SEX_MASK = 0;
unsigned char EAI_TYPE_MASK = 0;
unsigned short EAI_TEX_MASK = 0;
unsigned int EAI_MONEY_MASK = 0;
unsigned int EAI_CRYSTAL_MASK = 0;

enum ENCRYPT_ATTRIB_INDEX
{
	EAI_ID = 0,
	EAI_LUCKY,
	EAI_CURSE,
	EAI_HIDE,
	EAI_ACCURACY,
	EAI_ATKSPEED,
	EAI_ATKPALSY,
	EAI_ATKPOIS,
	EAI_MOVESPEED,
	EAI_WEIGHT,
	EAI_MAXDC,
	EAI_DC,
	EAI_MAXAC,
	EAI_AC,
	EAI_MAXMAC,
	EAI_MAC,
	EAI_MAXSC,
	EAI_SC,
	EAI_MAXMC,
	EAI_MC,
	EAI_MAXHP,
	EAI_HP,
	EAI_MAXMP,
	EAI_MP,
	EAI_MAXEXPR,
	EAI_EXPR,
	EAI_LEVEL,
	EAI_REQVALUE,
	EAI_REQTYPE,
	EAI_SEX,
	EAI_TYPE,
	EAI_TEX,
	EAI_MONEY,
	EAI_CRYSTAL,
	EAI_TOTAL
};

/*
#define ATB_ID_MASK			0x2307AB11
#define ATB_LUCKY_MASK		0x1B
#define ATB_CURSE_MASK		0xC3
#define ATB_HIDE_MASK		0x9A
#define ATB_ACCURACY_MASK	0x61
#define ATB_ATKSPEED_MASK	0xBA
#define ATB_ATKPALSY_MASK	0x33
#define ATB_ATKPOIS_MASK	0xAE
#define ATB_MOVESPEED_MASK	0x81
#define ATB_WEIGHT_MASK		0x3E
#define ATB_MAXDC_MASK		0x5A22
#define ATB_DC_MASK			0xAC05
#define ATB_MAXAC_MASK		0x61EA
#define ATB_AC_MASK			0x9EE1
#define ATB_MAXMAC_MASK		0xEA28
#define ATB_MAC_MASK		0x1A8E
#define ATB_MAXSC_MASK		0x39E2
#define ATB_SC_MASK			0x24CD
#define ATB_MAXMC_MASK		0x7299
#define ATB_MC_MASK			0x26AA
#define ATB_MAXHP_MASK		0x703D21D8
#define ATB_HP_MASK			0x1355BC2D
#define ATB_MAXMP_MASK		0x1EE2EAC1
#define ATB_MP_MASK			0x228EB3A1
#define ATB_MAXEXPR_MASK	0xDC27924D
#define ATB_EXPR_MASK		0x290ACE22
#define ATB_LEVEL_MASK		0x711A
#define ATB_REQVALUE_MASK	0xA8
#define ATB_REQTYPE_MASK	0x72
#define ATB_SEX_MASK		0xAB
#define ATB_TYPE_MASK		0x95
#define ATB_TEX_MASK		0xA33B

#define ATB_MONEY_MASK		0x7D2093EA*/

int g_nEncryptTable[50];
//////////////////////////////////////////////////////////////////////////
//	Using themida to avoid debugging with ollydbg and cheat engine
//////////////////////////////////////////////////////////////////////////
ObjectValid::ObjectValid()
{
	m_nEXP = m_nMP = m_nHP = 0;
	m_pAttached = NULL;
	m_cLevel = 0;
}

ObjectValid::~ObjectValid()
{

}
//////////////////////////////////////////////////////////////////////////
bool ObjectValid::TestValid()
{
	if(NULL == m_pAttached)
	{
		return true;
	}
	//return true;


	//PROTECT_START

	if(m_pAttached->GetType() == SOT_HERO)
	{
		//	HP
		bool bValid = true;
		int nGetValue = 0;
		nGetValue = m_nHP;
		//RESET_FLAG(nGetValue, VALID_INT_MASK);
		nGetValue = m_nHP ^ VALID_INT_MASK;
		if(m_pAttached->GetObject_HP() != nGetValue)
		{
#ifdef _DEBUG
			LOG(ERROR) << "HP not equal!" << m_pAttached->GetObject_HP() << "|" << nGetValue;
#endif
			return false;
		}

		//	MP
		nGetValue = m_nMP;
		//RESET_FLAG(nGetValue, VALID_INT_MASK);
		nGetValue = m_nMP ^ VALID_INT_MASK;
		if(m_pAttached->GetObject_MP() != nGetValue)
		{
#ifdef _DEBUG
			LOG(ERROR) << "MP not equal!" << m_pAttached->GetObject_MP() << "|" << nGetValue;
#endif
			return false;
		}

		//	Is Hide?
		/*HeroObject* pHero = static_cast<HeroObject*>(m_pAttached);
		if(pHero->IsHide())
		{
			if(pHero->GetHeroJob() != 2)
			{
				return false;
			}
			else
			{
				/ *if(GetTickCount() - m_dwHumEffTime[MMASK_HIDE_INDEX] > 900 * 1000)
				{
#ifdef _DEBUG
					LOG(ERROR) << "Hide time";
#endif
					return false;
				}* /
			}
		}*/
	}
	else if(m_pAttached->GetType() == SOT_MONSTER)
	{
		int nGetValue = 0;
		nGetValue = m_nHP;
		//RESET_FLAG(nGetValue, VALID_INT_MASK);
		nGetValue = m_nHP ^ VALID_INT_MASK;
		if(m_pAttached->GetObject_HP() != nGetValue)
		{
#ifdef _DEBUG
			LOG(ERROR) << "HP not equal!" << m_pAttached->GetObject_HP() << "|" << nGetValue;
#endif
			return false;
		}
	}
	else
	{
		return true;
	}

	//PROTECT_END
	return true;
}

void ObjectValid::SetHP(int _hp)
{

	//PROTECT_START

	m_nHP = _hp;
	//SET_FLAG(m_nHP, VALID_INT_MASK);
	m_nHP = m_nHP ^ VALID_INT_MASK;

	//PROTECT_END
}

void ObjectValid::SetMP(int _mp)
{
	//PROTECT_START

	m_nMP = _mp;
	//SET_FLAG(m_nMP, VALID_INT_MASK);
	m_nMP = m_nMP ^ VALID_INT_MASK;

	//PROTECT_END
}

void ObjectValid::SetEXP(int _exp)
{
	//PROTECT_START

	m_nEXP = _exp;
	//SET_FLAG(m_nEXP, VALID_INT_MASK);
	m_nEXP = m_nEXP ^ VALID_INT_MASK;

	//PROTECT_END
}

void ObjectValid::UpdateAllAttrib()
{
	if(m_pAttached)
	{
		SetHP(m_pAttached->GetObject_HP());
		SetMP(m_pAttached->GetObject_MP());
		if(m_pAttached->GetType() == SOT_HERO)
		{
			//SetEXP(m_pAttached->GetUserData()->stAttrib.EXPR);
		}
	}
}

void ObjectValid::IncHP(int _hp)
{
	//PROTECT_START

	int nGetValue = 0;
	nGetValue = m_nHP;
	//RESET_FLAG(nGetValue, VALID_INT_MASK);
	nGetValue = nGetValue ^ VALID_INT_MASK;
	/*if(nGetValue + _hp > m_pAttached->GetUserData()->stAttrib.maxHP)*/
	if(nGetValue + _hp > m_pAttached->GetObject_MaxHP())
	{
		//nGetValue = m_pAttached->GetUserData()->stAttrib.maxHP;
		nGetValue = m_pAttached->GetObject_MaxHP();
		m_nHP = nGetValue;
		//SET_FLAG(m_nHP, VALID_INT_MASK);
		m_nHP = m_nHP ^ VALID_INT_MASK;
	}
	else
	{
		nGetValue += _hp;
		m_nHP = nGetValue;
		//SET_FLAG(m_nHP, VALID_INT_MASK);
		m_nHP = m_nHP ^ VALID_INT_MASK;
	}

	//PROTECT_END
}

void ObjectValid::DecHP(int _hp)
{
	//PROTECT_START

	int nGetValue = 0;
	nGetValue = m_nHP;
	//RESET_FLAG(nGetValue, VALID_INT_MASK);
	nGetValue = nGetValue ^ VALID_INT_MASK;
	if(nGetValue - _hp < 0)
	{
		nGetValue = 0;
		m_nHP = nGetValue;
		//SET_FLAG(m_nHP, VALID_INT_MASK);
		m_nHP = m_nHP ^ VALID_INT_MASK;
	}
	else
	{
		nGetValue -= _hp;
		m_nHP = nGetValue;
		//SET_FLAG(m_nHP, VALID_INT_MASK);
		m_nHP = m_nHP ^ VALID_INT_MASK;
	}

	//PROTECT_END
}

void ObjectValid::IncMP(int _mp)
{
	//PROTECT_START

	int nGetValue = 0;
	nGetValue = m_nMP;
	//RESET_FLAG(nGetValue, VALID_INT_MASK);
	nGetValue = nGetValue ^ VALID_INT_MASK;
	//if(nGetValue + _mp > m_pAttached->GetUserData()->stAttrib.maxMP)
	if(nGetValue + _mp > m_pAttached->GetObject_MaxMP())
	{
		//nGetValue = m_pAttached->GetUserData()->stAttrib.maxMP;
		nGetValue = m_pAttached->GetObject_MaxMP();
		m_nMP = nGetValue;
		//SET_FLAG(m_nMP, VALID_INT_MASK);
		m_nMP = m_nMP ^ VALID_INT_MASK;
	}
	else
	{
		nGetValue += _mp;
		m_nMP = nGetValue;
		//SET_FLAG(m_nMP, VALID_INT_MASK);
		m_nMP = m_nMP ^ VALID_INT_MASK;
	}

	//PROTECT_END
}

void ObjectValid::DecMP(int _mp)
{
	//PROTECT_START

	int nGetValue = 0;
	nGetValue = m_nMP;
	//RESET_FLAG(nGetValue, VALID_INT_MASK);
	nGetValue = nGetValue ^ VALID_INT_MASK;
	if(nGetValue - _mp < 0)
	{
		nGetValue = 0;
		m_nMP = nGetValue;
		//SET_FLAG(m_nMP, VALID_INT_MASK);
		m_nMP = m_nMP ^ VALID_INT_MASK;
	}
	else
	{
		nGetValue -= _mp;
		m_nMP = nGetValue;
		//SET_FLAG(m_nMP, VALID_INT_MASK);
		m_nMP = m_nMP ^ VALID_INT_MASK;
	}

	//PROTECT_END
}

void ObjectValid::IncEXP(int _exp)
{
/*
#ifdef _THEMIDA_
	VM_START
#endif

#ifdef _THEMIDA_
		VM_END
#endif*/
}

void ObjectValid::DecEXP(int _exp)
{
/*
#ifdef _THEMIDA_
	VM_START
#endif
#ifdef _THEMIDA_
		VM_END
#endif*/
}





//////////////////////////////////////////////////////////////////////////
static unsigned char s_bStringTable[] = {0xEA, 0x33, 0x1B, 0x77, 0xA3, 0x79};

unsigned int ObjectValid::GetItemID(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(ID, _pItem->id);
}

void ObjectValid::GetItemName(ItemAttrib* _pItem, char* _pBuf)
{
	//strcpy(_pBuf, _pItem->name);
	memcpy(_pBuf, _pItem->name, sizeof(_pItem->name));
	int nStrlen = strlen(_pBuf);
	nStrlen = 19;
	int nStringTableSize = sizeof(s_bStringTable) / sizeof(s_bStringTable[0]);
	for(int i = 0; i < nStrlen; ++i)
	{
		_pBuf[i] = _pBuf[i] ^ s_bStringTable[i % nStringTableSize];
	}
}
unsigned int ObjectValid::GetItemLucky(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(LUCKY, _pItem->lucky);
}
unsigned int ObjectValid::GetItemCurse(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(CURSE, _pItem->curse);
}
unsigned int ObjectValid::GetItemHide(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(HIDE, _pItem->hide);
}
unsigned int ObjectValid::GetItemAccuracy(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(ACCURACY, _pItem->accuracy);
}
unsigned int ObjectValid::GetItemAtkSpeed(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(ATKSPEED, _pItem->atkSpeed);
}
unsigned int ObjectValid::GetItemAtkPalsy(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(ATKPALSY, _pItem->atkPalsy);
}
unsigned int ObjectValid::GetItemAtkPois(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(ATKPOIS, _pItem->atkPois);
}
unsigned int ObjectValid::GetItemMoveSpeed(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MOVESPEED, _pItem->moveSpeed);
}
unsigned int ObjectValid::GetItemWeight(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(WEIGHT, _pItem->weight);
}
unsigned int ObjectValid::GetItemReqType(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(REQTYPE, _pItem->reqType);
}
unsigned int ObjectValid::GetItemReqValue(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(REQVALUE, _pItem->reqValue);
}
unsigned int ObjectValid::GetItemSex(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(SEX, _pItem->sex);
}
unsigned int ObjectValid::GetItemType(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(TYPE, _pItem->type);
}
unsigned int ObjectValid::GetItemMaxDC(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MAXDC, _pItem->maxDC);
}
unsigned int ObjectValid::GetItemDC(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(DC, _pItem->DC);
}
unsigned int ObjectValid::GetItemMaxAC(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MAXAC, _pItem->maxAC);
}
unsigned int ObjectValid::GetItemAC(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(AC, _pItem->AC);
}
unsigned int ObjectValid::GetItemMaxMAC(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MAXMAC, _pItem->maxMAC);
}
unsigned int ObjectValid::GetItemMAC(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MAC, _pItem->MAC);
}
unsigned int ObjectValid::GetItemMaxSC(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MAXSC, _pItem->maxSC);
}
unsigned int ObjectValid::GetItemSC(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(SC, _pItem->SC);
}
unsigned int ObjectValid::GetItemMaxMC(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MAXMC, _pItem->maxMC);
}
unsigned int ObjectValid::GetItemMC(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MC, _pItem->MC);
}
unsigned int ObjectValid::GetItemMaxHP(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MAXHP, _pItem->maxHP);
}
unsigned int ObjectValid::GetItemHP(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(HP, _pItem->HP);
}
unsigned int ObjectValid::GetItemMaxMP(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MAXMP, _pItem->maxMP);
}
unsigned int ObjectValid::GetItemMP(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MP, _pItem->MP);
}
unsigned int ObjectValid::GetItemMaxExpr(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(MAXEXPR, _pItem->maxEXPR);
}
unsigned int ObjectValid::GetItemExpr(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(EXPR, _pItem->EXPR);
}
unsigned int ObjectValid::GetItemLevel(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(LEVEL, _pItem->level);
}
unsigned int ObjectValid::GetItemTex(ItemAttrib* _pItem)
{
	return ATB_DECRYPT(TEX, _pItem->tex);
}
//static unsigned int GetMoney(ItemAttrib* _pItem);


void ObjectValid::SetItemID(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(ID, _value);
	_pItem->id = uValue;
}
void ObjectValid::SetItemName(ItemAttrib* _pItem, const char* _pszName)
{

}
void ObjectValid::SetItemLucky(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(LUCKY, _value);
	_pItem->lucky = uValue;
}
void ObjectValid::SetItemCurse(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(CURSE, _value);
	_pItem->curse = uValue;
}
void ObjectValid::SetItemHide(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(HIDE, _value);
	_pItem->hide = uValue;
}
void ObjectValid::SetItemAccuracy(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(ACCURACY, _value);
	_pItem->accuracy = uValue;
}
void ObjectValid::SetItemAtkSpeed(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(ATKSPEED, _value);
	_pItem->atkSpeed = uValue;
}
void ObjectValid::SetItemAtkPalsy(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(ATKPALSY, _value);
	_pItem->atkPalsy = uValue;
}
void ObjectValid::SetItemAtkPois(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(ATKPOIS, _value);
	_pItem->atkPois = uValue;
}
void ObjectValid::SetItemMoveSpeed(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MOVESPEED, _value);
	_pItem->moveSpeed = uValue;
}
void ObjectValid::SetItemWeight(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(WEIGHT, _value);
	_pItem->weight = uValue;
}
void ObjectValid::SetItemReqType(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(REQTYPE, _value);
	_pItem->reqType = uValue;
}
void ObjectValid::SetItemReqValue(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(REQVALUE, _value);
	_pItem->reqValue = uValue;
}
void ObjectValid::SetItemSex(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(SEX, _value);
	_pItem->sex = uValue;
}
void ObjectValid::SetItemType(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(TYPE, _value);
	_pItem->type = uValue;
}
void ObjectValid::SetItemMaxDC(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MAXDC, _value);
	_pItem->maxDC = uValue;
}
void ObjectValid::SetItemDC(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(DC, _value);
	_pItem->DC = uValue;
}
void ObjectValid::SetItemMaxAC(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MAXAC, _value);
	_pItem->maxAC = uValue;
}
void ObjectValid::SetItemAC(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(AC, _value);
	_pItem->AC = uValue;
}
void ObjectValid::SetItemMaxMAC(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MAXMAC, _value);
	_pItem->maxMAC = uValue;
}
void ObjectValid::SetItemMAC(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MAC, _value);
	_pItem->MAC = uValue;
}
void ObjectValid::SetItemMaxSC(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MAXSC, _value);
	_pItem->maxSC = uValue;
}
void ObjectValid::SetItemSC(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(SC, _value);
	_pItem->SC = uValue;
}
void ObjectValid::SetItemMaxMC(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MAXMC, _value);
	_pItem->maxMC = uValue;
}
void ObjectValid::SetItemMC(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MC, _value);
	_pItem->MC = uValue;
}
void ObjectValid::SetItemMaxHP(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MAXHP, _value);
	_pItem->maxHP = uValue;
}
void ObjectValid::SetItemHP(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(HP, _value);
	_pItem->HP = uValue;
}
void ObjectValid::SetItemMaxMP(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MAXMP, _value);
	_pItem->maxMP = uValue;
}
void ObjectValid::SetItemMP(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MP, _value);
	_pItem->MP = uValue;
}
void ObjectValid::SetItemMaxExpr(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(MAXEXPR, _value);
	_pItem->maxEXPR = uValue;
}
void ObjectValid::SetItemExpr(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(EXPR, _value);
	_pItem->EXPR = uValue;
}
void ObjectValid::SetItemLevel(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(LEVEL, _value);
	_pItem->level = uValue;
}
void ObjectValid::SetItemTex(ItemAttrib* _pItem, unsigned int _value)
{
	unsigned int uValue = ATB_ENCRYPT(TEX, _value);
	_pItem->tex = uValue;
}

unsigned int EncryptValue(unsigned int _mask, unsigned int _value)
{
	_value = _value ^ _mask;
	bool bZero = false;

	if(_mask < 0xFF)
	{
		//	1 bytes
		if(_value & 0x80)
		{
			bZero = (_value & 0x20) == 0 ? true : false;
			if(bZero)
			{
				_value |= 0x20;
			}
			else
			{
				_value &= (~0x20);
			}
		}
	}
	else if(_mask < 0xFFFF)
	{
		//	2bytes
		if(_value & 0x4000)
		{
			bZero = (_value & 0x1000) == 0 ? true : false;
			if(bZero)
			{
				_value |= 0x1000;
			}
			else
			{
				_value &= (~0x1000);
			}

			bZero = (_value & 0x0040) == 0 ? true : false;
			if(bZero)
			{
				_value |= 0x0040;
			}
			else
			{
				_value &= (~0x0040);
			}
		}
	}
	else if(_mask < 0xFFFFFFFF)
	{
		if(_value & 0x08000000)
		{
			bZero = (_value & 0x40000000) == 0 ? true : false;
			if(bZero)
			{
				_value |= 0x40000000;
			}
			else
			{
				_value &= (~0x40000000);
			}

			bZero = (_value & 0x00080000) == 0 ? true : false;
			if(bZero)
			{
				_value |= 0x00080000;
			}
			else
			{
				_value &= (~0x00080000);
			}
		}
	}

	return _value;
}

unsigned int DecryptValue(unsigned int _mask, unsigned int _value)
{
	bool bZero = false;

	if(_mask < 0xFF)
	{
		//	1 bytes
		if(_value & 0x80)
		{
			bZero = (_value & 0x20) == 0 ? true : false;
			if(bZero)
			{
				_value |= 0x20;
			}
			else
			{
				_value &= (~0x20);
			}
		}
	}
	else if(_mask < 0xFFFF)
	{
		//	2bytes
		if(_value & 0x4000)
		{
			bZero = (_value & 0x1000) == 0 ? true : false;
			if(bZero)
			{
				_value |= 0x1000;
			}
			else
			{
				_value &= (~0x1000);
			}

			bZero = (_value & 0x0040) == 0 ? true : false;
			if(bZero)
			{
				_value |= 0x0040;
			}
			else
			{
				_value &= (~0x0040);
			}
		}
	}
	else if(_mask < 0xFFFFFFFF)
	{
		if(_value & 0x08000000)
		{
			bZero = (_value & 0x40000000) == 0 ? true : false;
			if(bZero)
			{
				_value |= 0x40000000;
			}
			else
			{
				_value &= (~0x40000000);
			}

			bZero = (_value & 0x00080000) == 0 ? true : false;
			if(bZero)
			{
				_value |= 0x00080000;
			}
			else
			{
				_value &= (~0x00080000);
			}
		}
	}

	_value = _value ^ _mask;
	return _value;
}

void ObjectValid::EncryptAttrib(ItemAttrib* _pItem)
{
	if(!TEST_FLAG_BOOL(_pItem->extra, EXTRA_MASK_ENCRYPT))
	{
		ItemAttrib oriatb;
		oriatb = *_pItem;
		SET_FLAG(_pItem->extra, EXTRA_MASK_ENCRYPT);

		SetItemID(_pItem, oriatb.id);
		//	Name
		int nStrlen = strlen(_pItem->name);
		nStrlen = 19;
		int nStringTableSize = sizeof(s_bStringTable) / sizeof(s_bStringTable[0]);
		for(int i = 0; i < nStrlen; ++i)
		{
			_pItem->name[i] = _pItem->name[i] ^ s_bStringTable[i % nStringTableSize];
		}
		SetItemLucky(_pItem, oriatb.lucky);
		SetItemCurse(_pItem, oriatb.curse);
		SetItemHide(_pItem, oriatb.hide);
		SetItemAccuracy(_pItem, oriatb.accuracy);
		SetItemAtkSpeed(_pItem, oriatb.atkSpeed);
		SetItemAtkPalsy(_pItem, oriatb.atkPalsy);
		SetItemAtkPois(_pItem, oriatb.atkPois);
		SetItemMoveSpeed(_pItem, oriatb.moveSpeed);
		SetItemWeight(_pItem, oriatb.weight);
		SetItemReqType(_pItem, oriatb.reqType);
		SetItemReqValue(_pItem, oriatb.reqValue);
		SetItemSex(_pItem, oriatb.sex);
		SetItemType(_pItem, oriatb.type);
		SetItemMaxDC(_pItem, oriatb.maxDC);
		SetItemDC(_pItem, oriatb.DC);
		SetItemMaxAC(_pItem, oriatb.maxAC);
		SetItemAC(_pItem, oriatb.AC);
		SetItemMaxMAC(_pItem, oriatb.maxMAC);
		SetItemMAC(_pItem, oriatb.MAC);
		SetItemMaxSC(_pItem, oriatb.maxSC);
		SetItemSC(_pItem, oriatb.SC);
		SetItemMaxMC(_pItem, oriatb.maxMC);
		SetItemMC(_pItem, oriatb.MC);
		SetItemMaxHP(_pItem, oriatb.maxHP);
		SetItemHP(_pItem, oriatb.HP);
		SetItemMaxMP(_pItem, oriatb.maxMP);
		SetItemMP(_pItem, oriatb.MP);
		SetItemMaxExpr(_pItem, oriatb.maxEXPR);
		SetItemExpr(_pItem, oriatb.EXPR);
		SetItemLevel(_pItem, oriatb.level);
		SetItemTex(_pItem, oriatb.tex);
	}
}

void ObjectValid::DecryptAttrib(ItemAttrib* _pItem)
{
	if(TEST_FLAG_BOOL(_pItem->extra, EXTRA_MASK_ENCRYPT))
	{
		RESET_FLAG(_pItem->extra, EXTRA_MASK_ENCRYPT);

		_pItem->id = GetItemID(_pItem);
		//	Name
		int nStrlen = strlen(_pItem->name);
		nStrlen = 19;
		int nStringTableSize = sizeof(s_bStringTable) / sizeof(s_bStringTable[0]);
		for(int i = 0; i < nStrlen; ++i)
		{
			_pItem->name[i] = _pItem->name[i] ^ s_bStringTable[i % nStringTableSize];
		}
		_pItem->lucky = GetItemLucky(_pItem);
		_pItem->curse = GetItemCurse(_pItem);
		_pItem->hide = GetItemHide(_pItem);
		_pItem->accuracy = GetItemAccuracy(_pItem);
		_pItem->atkSpeed = GetItemAtkSpeed(_pItem);
		_pItem->atkPalsy = GetItemAtkPalsy(_pItem);
		_pItem->atkPois = GetItemAtkPois(_pItem);
		_pItem->moveSpeed = GetItemMoveSpeed(_pItem);
		_pItem->weight = GetItemWeight(_pItem);
		_pItem->reqType = GetItemReqType(_pItem);
		_pItem->reqValue = GetItemReqValue(_pItem);
		_pItem->sex = GetItemSex(_pItem);
		_pItem->type = GetItemType(_pItem);
		_pItem->maxDC = GetItemMaxDC(_pItem);
		_pItem->DC = GetItemDC(_pItem);
		_pItem->maxAC = GetItemMaxAC(_pItem);
		_pItem->AC = GetItemAC(_pItem);
		_pItem->maxMAC = GetItemMaxMAC(_pItem);
		_pItem->MAC = GetItemMAC(_pItem);
		_pItem->maxSC = GetItemMaxSC(_pItem);
		_pItem->SC = GetItemSC(_pItem);
		_pItem->maxMC = GetItemMaxMC(_pItem);
		_pItem->MC = GetItemMC(_pItem);
		_pItem->maxHP = GetItemMaxHP(_pItem);
		_pItem->HP = GetItemHP(_pItem);
		_pItem->maxMP = GetItemMaxMP(_pItem);
		_pItem->MP = GetItemMP(_pItem);
		_pItem->maxEXPR = GetItemMaxExpr(_pItem);
		_pItem->EXPR = GetItemExpr(_pItem);
		_pItem->level = GetItemLevel(_pItem);
		_pItem->tex = GetItemTex(_pItem);
	}
}




//////////////////////////////////////////////////////////////////////////
void ObjectValid::GenerateEncryptTable()
{
#ifdef _EXTEND_ENCRYPT
	/*for(int i = 0; i < EAI_TOTAL; ++i)
	{
		g_nEncryptTable[i] = rand() % 0xFFFFFFFF;
	}*/
	/*EAI_ID_MASK = rand() % 0xFFFFFFFF;
	EAI_LUCKY_MASK = rand() % 0xFFFFFFFF;
	EAI_CURSE_MASK = rand() % 0xFFFFFFFF;
	EAI_HIDE_MASK = rand() % 0xFFFFFFFF;
	EAI_ACCURACY_MASK = rand() % 0xFFFFFFFF;
	EAI_ATKSPEED_MASK = rand() % 0xFFFFFFFF;
	EAI_ATKPALSY_MASK = rand() % 0xFFFFFFFF;
	EAI_ATKPOIS_MASK = rand() % 0xFFFFFFFF;
	EAI_MOVESPEED_MASK = rand() % 0xFFFFFFFF;
	EAI_WEIGHT_MASK = rand() % 0xFFFFFFFF;
	EAI_MAXDC_MASK = rand() % 0xFFFFFFFF;
	EAI_DC_MASK = rand() % 0xFFFFFFFF;
	EAI_MAXAC_MASK = rand() % 0xFFFFFFFF;
	EAI_AC_MASK = rand() % 0xFFFFFFFF;
	EAI_MAXMAC_MASK = rand() % 0xFFFFFFFF;
	EAI_MAC_MASK = rand() % 0xFFFFFFFF;
	EAI_MAXSC_MASK = rand() % 0xFFFFFFFF;
	EAI_SC_MASK = rand() % 0xFFFFFFFF;
	EAI_MAXMC_MASK = rand() % 0xFFFFFFFF;
	EAI_MC_MASK = rand() % 0xFFFFFFFF;
	EAI_MAXHP_MASK = rand() % 0xFFFFFFFF;
	EAI_HP_MASK = rand() % 0xFFFFFFFF;
	EAI_MAXMP_MASK = rand() % 0xFFFFFFFF;
	EAI_MP_MASK = rand() % 0xFFFFFFFF;
	EAI_MAXEXPR_MASK = rand() % 0xFFFFFFFF;
	EAI_EXPR_MASK = rand() % 0xFFFFFFFF;
	EAI_LEVEL_MASK = rand() % 0xFFFFFFFF;
	EAI_REQVALUE_MASK = rand() % 0xFFFFFFFF;
	EAI_REQTYPE_MASK = rand() % 0xFFFFFFFF;
	EAI_SEX_MASK = rand() % 0xFFFFFFFF;
	EAI_TYPE_MASK = rand() % 0xFFFFFFFF;
	EAI_TEX_MASK = rand() % 0xFFFFFFFF;
	EAI_MONEY_MASK = rand() % 0xFFFFFFFF;*/

	GENERATE_MASK(EAI_ID_MASK);
	GENERATE_MASK(EAI_LUCKY_MASK);
	GENERATE_MASK(EAI_CURSE_MASK);
	GENERATE_MASK(EAI_HIDE_MASK);
	GENERATE_MASK(EAI_ACCURACY_MASK);
	GENERATE_MASK(EAI_ATKSPEED_MASK);
	GENERATE_MASK(EAI_ATKPALSY_MASK);
	GENERATE_MASK(EAI_ATKPOIS_MASK);
	GENERATE_MASK(EAI_MOVESPEED_MASK);
	GENERATE_MASK(EAI_WEIGHT_MASK);
	GENERATE_MASK(EAI_MAXDC_MASK);
	GENERATE_MASK(EAI_DC_MASK);
	GENERATE_MASK(EAI_MAXAC_MASK);
	GENERATE_MASK(EAI_AC_MASK);
	GENERATE_MASK(EAI_MAXMAC_MASK);
	GENERATE_MASK(EAI_MAC_MASK);
	GENERATE_MASK(EAI_MAXSC_MASK);
	GENERATE_MASK(EAI_SC_MASK);
	GENERATE_MASK(EAI_MAXMC_MASK);
	GENERATE_MASK(EAI_MC_MASK);
	GENERATE_MASK(EAI_MAXHP_MASK);
	GENERATE_MASK(EAI_HP_MASK);
	GENERATE_MASK(EAI_MAXMP_MASK);
	GENERATE_MASK(EAI_MP_MASK);
	GENERATE_MASK(EAI_MAXEXPR_MASK);
	GENERATE_MASK(EAI_EXPR_MASK);
	GENERATE_MASK(EAI_LEVEL_MASK);
	GENERATE_MASK(EAI_REQVALUE_MASK);
	GENERATE_MASK(EAI_REQTYPE_MASK);
	GENERATE_MASK(EAI_SEX_MASK);
	GENERATE_MASK(EAI_TYPE_MASK);
	GENERATE_MASK(EAI_TEX_MASK);
	GENERATE_MASK(EAI_MONEY_MASK);
	GENERATE_MASK(EAI_CRYSTAL_MASK);

	/*for(int i = 0; i < sizeof(s_bStringTable) / sizeof(s_bStringTable[0]); ++i)
	{
		s_bStringTable[i] = rand() % 0xFF;
	}*/
#endif
}

unsigned char GetRandomMask(unsigned char _bVar)
{
	unsigned char bVar = 1 + rand() % 0xFF;
	return bVar;
}

unsigned short GetRandomMask(unsigned short _wVar)
{
	/*DWORD dwRand = rand();

	WORD bHigh = ((WORD)(dwRand % 9)) << 3;
	BYTE bLow = dwRand % 0x7FFF;
	WORD wVar = bHigh << 4 + bLow;*/
	//WORD wVar = ((WORD)(rand() % 9)) << 12 + rand() % 0x7FFF;
	unsigned short wHign = rand() % 9;
	wHign = wHign << 12;
	wHign += rand() % 0x7FFF;

	return wHign;
}

unsigned int GetRandomMask(unsigned int _dwVar)
{
	//DWORD dwRand = rand();

	//DWORD dwHigh = ((DWORD)(((WORD)(dwRand % 9)) << 3 + dwRand % 0x7FFF)) << 4;
	//WORD wLow = ((WORD)(dwRand % 9)) << 3 + dwRand % 0x7FFF;
	//DWORD dwVar = ((DWORD)(((WORD)(rand() % 9)) << 12 + rand() % 0x7FFF)) << 16 + ((WORD)(rand() % 9)) << 12 + rand() % 0x7FFF;
	//DWORD dwVar = dwHigh + wLow;

	unsigned int dwHigh = rand() % 9;
	dwHigh = dwHigh << 12;
	dwHigh += rand() % 0x7FFF;
	dwHigh = dwHigh << 16;

	unsigned short wLow = rand() % 9;
	wLow = wLow << 12;
	wLow += rand() % 0x7FFF;

	unsigned int dwVar = dwHigh + wLow;

	return dwVar;
}