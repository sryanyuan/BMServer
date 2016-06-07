#include "LuaItemHelper.h"
#include "ObjectValid.h"
#include "../../CommonModule/ShareData.h"
//////////////////////////////////////////////////////////////////////////
int LuaItemHelper::GetItemType(ItemAttrib* _pItem)
{
	return (int)GETITEMATB(_pItem, Type);
}

int LuaItemHelper::GetItemAtkSpeed(ItemAttrib *_pItem)
{
	return (int)GETITEMATB(_pItem, AtkSpeed);
}

int LuaItemHelper::GetItemLucky(ItemAttrib *_pItem)
{
	return (int)GETITEMATB(_pItem, Lucky);
}

int LuaItemHelper::GetItemUpgrade(ItemAttrib* _pItem)
{
	return (int)Lua_GetItemUpgrade(GETITEMATB(_pItem, Level));
}

int LuaItemHelper::GetItemTag(ItemAttrib* _pItem)
{
	return _pItem->tag;
}

int LuaItemHelper::GetItemMP(ItemAttrib *_pItem)
{
	return (int)GETITEMATB(_pItem, MP);
}

void LuaItemHelper::EncryptItem(ItemAttrib *_pItem)
{
	ObjectValid::EncryptAttrib(_pItem);
}

void LuaItemHelper::DecryptItem(ItemAttrib *_pItem)
{
	ObjectValid::DecryptAttrib(_pItem);
}

bool LuaItemHelper::IsEncrypt(ItemAttrib *_pItem)
{
	return TEST_FLAG_BOOL(_pItem->extra, EXTRA_MASK_ENCRYPT);
}

int LuaItemHelper::GetItemAtkPalsy(ItemAttrib* _pItem)
{
	return (int)GETITEMATB(_pItem, AtkPalsy);
}