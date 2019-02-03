#ifndef _INC_OBJECTVALID_
#define _INC_OBJECTVALID_
//////////////////////////////////////////////////////////////////////////
#include "../../CommonModule/GDefine.h"
#include "../../CommonModule/ByteBuffer.h"
class GameObject;
//////////////////////////////////////////////////////////////////////////
#define VALID_INT_MASK		0x21A85C33
#define VALID_SHORT_MASK	0x9D37
#define VALID_CHAR_MASK		0x7C
//////////////////////////////////////////////////////////////////////////
//	Normal mask
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

#define ATB_MONEY_MASK		0x7D2093EA
#define ATB_CRYSTAL_MASK	0xA15fea89

//////////////////////////////////////////////////////////////////////////
//	Encryptor and decryptor
inline unsigned int EncryptValue(unsigned int _mask, unsigned int _value);
inline unsigned int DecryptValue(unsigned int _mask, unsigned int _value);

//////////////////////////////////////////////////////////////////////////
//	Using extended encryption
#define _EXTEND_ENCRYPT

//////////////////////////////////////////////////////////////////////////
//	Encrypt mode
#ifdef _EXTEND_ENCRYPT

#define ATB_ENCRYPT(MASK, VALUE)	EncryptValue(EAI_##MASK##_MASK, VALUE);
#define ATB_DECRYPT(MASK, VALUE)	DecryptValue(EAI_##MASK##_MASK, VALUE);
/*
#define ATB_ENCRYPT(MASK, VALUE)	EncryptValue(g_nEncryptTable[EAI_##MASK], VALUE);
#define ATB_DECRYPT(MASK, VALUE)	DecryptValue(g_nEncryptTable[EAI_##MASK], VALUE);*/

#else

#define ATB_ENCRYPT(MASK, VALUE)	EncryptValue(ATB_##MASK##_MASK, VALUE);
#define ATB_DECRYPT(MASK, VALUE)	DecryptValue(ATB_##MASK##_MASK, VALUE);

#endif

//////////////////////////////////////////////////////////////////////////
//	Encrypt mask generator
unsigned char GetRandomMask(unsigned char _bVar);
unsigned short GetRandomMask(unsigned short _wVar);
unsigned int GetRandomMask(unsigned int _dwVar);

//////////////////////////////////////////////////////////////////////////
//	Define of GENERATE_MASK
#ifdef _DEBUG

#define GENERATE_MASK(MASK)\
	MASK = GetRandomMask(MASK);\
	LOG(INFO) << #MASK << " IS\t" << (unsigned int)MASK;
#else

#define GENERATE_MASK(MASK)\
	MASK = GetRandomMask(MASK);

#endif

//////////////////////////////////////////////////////////////////////////
//	Define of get/set attribute
#define GET_OBJECT_ATBE(var, NAME)	inline unsigned int GetObject_##NAME()\
									{\
										if(IsEncrypt())\
										{\
											return ObjectValid::GetItem##NAME(&m_stData.stAttrib);\
										}\
										else\
										{\
											return m_stData.stAttrib.var;\
										}\
									}
#define SET_OBJECT_ATBE(var, NAME)		inline void SetObject_##NAME(unsigned int _var)\
										{\
											if(IsEncrypt())\
											{\
												ObjectValid::SetItem##NAME(&m_stData.stAttrib, _var);\
											}\
											else\
											{\
												m_stData.stAttrib.var = _var;\
											}\
										}
#define GET_OBJECT_ATBN(var, NAME)	inline unsigned int GetObject_##NAME()\
									{\
										return m_stData.stAttrib.var;\
									}
#define SET_OBJECT_ATBN(var, NAME)	inline void SetObject_##NAME(unsigned int _var)\
									{\
										m_stData.stAttrib.var = _var;\
									}

#define SETGET_OBJECT_ATBN(var, NAME)	GET_OBJECT_ATBN(var, NAME);SET_OBJECT_ATBN(var, NAME);
#define SETGET_OBJECT_ATBE(var, NAME)	GET_OBJECT_ATBE(var, NAME);SET_OBJECT_ATBE(var, NAME);

#define GET_ITEM_ATBE(var, NAME)		static unsigned int GetItem_##NAME(ItemAttrib* ITEM)\
										{\
											if(TEST_FLAG_BOOL(ITEM->extra, EXTRA_MASK_ENCRYPT))\
											{\
												return ObjectValid::GetItem##NAME(ITEM);\
											}\
											else\
											{\
												return ITEM->var;\
											}\
										}
#define SET_ITEM_ATBE(var, NAME)		static void SetItem_##NAME(ItemAttrib* ITEM, unsigned int _var)\
										{\
											if(TEST_FLAG_BOOL(ITEM->extra, EXTRA_MASK_ENCRYPT))\
											{\
												ObjectValid::SetItem##NAME(ITEM, _var);\
											}\
											else\
											{\
												ITEM->var = _var;\
											}\
										}
#define GET_ITEM_ATBN(var, NAME)		static unsigned int GetItem_##NAME(ItemAttrib* ITEM)\
										{\
											return ITEM->var;\
										}
#define SET_ITEM_ATBN(var, NAME)		static void SetItem_##NAME(ItemAttrib* ITEM, unsigned int _var)\
										{\
											ITEM->var = _var;\
										}
#define SETGET_ITEM_ATBE(var, NAME)		GET_ITEM_ATBE(var, NAME);SET_ITEM_ATBE(var, NAME);
#define SETGET_ITEM_ATBN(var, NAME)		GET_ITEM_ATBN(var, NAME);SET_ITEM_ATBN(var, NAME);

#define GETITEMATB(ITEM, NAME)			ObjectValid::GetItem_##NAME(ITEM)
#define SETITEMATB(ITEM, NAME, VAR)		ObjectValid::SetItem_##NAME(ITEM, VAR)

#ifdef _DEBUG
#define VALID_MAX_ABILITY				999
#else
#define VALID_MAX_ABILITY				200
#endif
//////////////////////////////////////////////////////////////////////////
//	Encryption helper
class ObjectValid
{
public:
	ObjectValid();
	~ObjectValid();

public:
	inline void Attach(GameObject* _pObj)
	{
		m_pAttached = _pObj;
	}
	bool TestValid();

	void SetHP(int _hp);
	void SetMP(int _mp);
	void SetEXP(int _exp);
	void UpdateAllAttrib();
	void AddReceiveDamage(int _dag);
	inline int GetReceiveDamage()
	{
		return m_nReceiveDamage;
	}

	void IncHP(int _hp);
	void IncMP(int _mp);
	void IncEXP(int _exp);
	void DecHP(int _hp);
	void DecMP(int _mp);
	void DecEXP(int _exp);

public:
	static void EncryptAttrib(ItemAttrib* _pItem);
	static void DecryptAttrib(ItemAttrib* _pItem);

	static void GenerateEncryptTable();

	static unsigned int GetItemID(ItemAttrib* _pItem);
	static void GetItemName(ItemAttrib* _pItem, char* _pBuf);
	static unsigned int GetItemLucky(ItemAttrib* _pItem);
	static unsigned int GetItemCurse(ItemAttrib* _pItem);
	static unsigned int GetItemHide(ItemAttrib* _pItem);
	static unsigned int GetItemAccuracy(ItemAttrib* _pItem);
	static unsigned int GetItemAtkSpeed(ItemAttrib* _pItem);
	static unsigned int GetItemAtkPalsy(ItemAttrib* _pItem);
	static unsigned int GetItemAtkPois(ItemAttrib* _pItem);
	static unsigned int GetItemMoveSpeed(ItemAttrib* _pItem);
	static unsigned int GetItemWeight(ItemAttrib* _pItem);
	static unsigned int GetItemReqType(ItemAttrib* _pItem);
	static unsigned int GetItemReqValue(ItemAttrib* _pItem);
	static unsigned int GetItemSex(ItemAttrib* _pItem);
	static unsigned int GetItemType(ItemAttrib* _pItem);
	static unsigned int GetItemMaxDC(ItemAttrib* _pItem);
	static unsigned int GetItemDC(ItemAttrib* _pItem);
	static unsigned int GetItemMaxAC(ItemAttrib* _pItem);
	static unsigned int GetItemAC(ItemAttrib* _pItem);
	static unsigned int GetItemMaxMAC(ItemAttrib* _pItem);
	static unsigned int GetItemMAC(ItemAttrib* _pItem);
	static unsigned int GetItemMaxSC(ItemAttrib* _pItem);
	static unsigned int GetItemSC(ItemAttrib* _pItem);
	static unsigned int GetItemMaxMC(ItemAttrib* _pItem);
	static unsigned int GetItemMC(ItemAttrib* _pItem);
	static unsigned int GetItemMaxHP(ItemAttrib* _pItem);
	static unsigned int GetItemHP(ItemAttrib* _pItem);
	static unsigned int GetItemMaxMP(ItemAttrib* _pItem);
	static unsigned int GetItemMP(ItemAttrib* _pItem);
	static unsigned int GetItemMaxExpr(ItemAttrib* _pItem);
	static unsigned int GetItemExpr(ItemAttrib* _pItem);
	static unsigned int GetItemLevel(ItemAttrib* _pItem);
	static unsigned int GetItemTex(ItemAttrib* _pItem);

	static void SetItemID(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemName(ItemAttrib* _pItem, const char* _pszName);
	static void SetItemLucky(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemCurse(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemHide(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemAccuracy(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemAtkSpeed(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemAtkPalsy(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemAtkPois(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMoveSpeed(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemWeight(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemReqType(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemReqValue(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemSex(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemType(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMaxDC(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemDC(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMaxAC(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemAC(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMaxMAC(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMAC(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMaxSC(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemSC(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMaxMC(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMC(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMaxHP(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemHP(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMaxMP(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMP(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemMaxExpr(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemExpr(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemLevel(ItemAttrib* _pItem, unsigned int _value);
	static void SetItemTex(ItemAttrib* _pItem, unsigned int _value);

	SETGET_ITEM_ATBE(id, ID);
	SETGET_ITEM_ATBE(lucky, Lucky);
	SETGET_ITEM_ATBE(curse, Curse);
	SETGET_ITEM_ATBE(hide, Hide);
	SETGET_ITEM_ATBE(accuracy, Accuracy);
	SETGET_ITEM_ATBE(atkSpeed, AtkSpeed);
	SETGET_ITEM_ATBE(atkPalsy, AtkPalsy);
	SETGET_ITEM_ATBE(atkPois, AtkPois);
	SETGET_ITEM_ATBE(moveSpeed, MoveSpeed);
	SETGET_ITEM_ATBE(weight, Weight);
	SETGET_ITEM_ATBE(reqType, ReqType);
	SETGET_ITEM_ATBE(reqValue, ReqValue);
	SETGET_ITEM_ATBE(sex, Sex);
	SETGET_ITEM_ATBE(type, Type);
	SETGET_ITEM_ATBE(maxDC, MaxDC);
	SETGET_ITEM_ATBE(DC, DC);
	SETGET_ITEM_ATBE(maxAC, MaxAC);
	SETGET_ITEM_ATBE(AC, AC);
	SETGET_ITEM_ATBE(maxMAC, MaxMAC);
	SETGET_ITEM_ATBE(MAC, MAC);
	SETGET_ITEM_ATBE(maxSC, MaxSC);
	SETGET_ITEM_ATBE(SC, SC);
	SETGET_ITEM_ATBE(maxMC, MaxMC);
	SETGET_ITEM_ATBE(MC, MC);
	SETGET_ITEM_ATBE(maxHP, MaxHP);
	SETGET_ITEM_ATBE(HP, HP);
	SETGET_ITEM_ATBE(maxMP, MaxMP);
	SETGET_ITEM_ATBE(MP, MP);
	SETGET_ITEM_ATBE(maxEXPR, MaxExpr);
	SETGET_ITEM_ATBE(EXPR, Expr);
	SETGET_ITEM_ATBE(level, Level);
	SETGET_ITEM_ATBE(tex, Tex);

private:
	int m_nHP;
	int m_nMP;
	int m_nEXP;
	char m_cLevel;
	int m_nReceiveDamage;

	GameObject* m_pAttached;
};

//////////////////////////////////////////////////////////////////////////
#endif