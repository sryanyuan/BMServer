#ifndef _INC_LUAITEMHELPER_
#define _INC_LUAITEMHELPER_
//////////////////////////////////////////////////////////////////////////
struct ItemAttrib;

class LuaItemHelper
{
public:
	static int GetItemType(ItemAttrib* _pItem);
	static int GetItemAtkSpeed(ItemAttrib* _pItem);
	static int GetItemUpgrade(ItemAttrib* _pItem);
	static int GetItemAtkPalsy(ItemAttrib* _pItem);
	static int GetItemLucky(ItemAttrib* _pItem);
	static int GetItemTag(ItemAttrib* _pItem);
	static int GetItemMP(ItemAttrib* _pItem);

	static void EncryptItem(ItemAttrib* _pItem);
	static void DecryptItem(ItemAttrib* _pItem);
	static bool IsEncrypt(ItemAttrib* _pItem);
};

// bit operations
unsigned int GetLoWord(unsigned int _nValue);

unsigned int GetHiWord(unsigned int _nValue);

unsigned int MakeLong(unsigned int _nLow, unsigned int _nHigh);
//////////////////////////////////////////////////////////////////////////
#endif