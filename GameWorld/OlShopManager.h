#ifndef _INC_OLSHOPMANAGER_
#define _INC_OLSHOPMANAGER_
//////////////////////////////////////////////////////////////////////////
#include <map>
#include <list>
#include <vector>
//////////////////////////////////////////////////////////////////////////
struct OlShopBuyQueryInfo
{
	int nQueryID;
};
//////////////////////////////////////////////////////////////////////////
typedef std::map<int, OlShopBuyQueryInfo> OlShopBuyQueryInfoMap;
typedef std::list<int> OlShopItemList;
typedef std::vector<int> OlShopItemVector;
//////////////////////////////////////////////////////////////////////////
class OlShopManager
{
public:
	static OlShopManager* GetInstance()
	{
		static OlShopManager s_ins;
		return &s_ins;
	}
public:
	~OlShopManager();

protected:
	OlShopManager();

public:
	//	检测玩家是否正在等待购买验证
	bool IsWaitForQuery(int _nUid);
	//	设置玩家正在等待购买验证
	int SetWaitForQuery(int _nUid, bool _bSet);
	//	得到玩家的查询凭证
	int GetQueryID(int _nUid);

	//	添加商品
	void AddShopItem(int _nID);
	void RemoveShopItem(int _nID);
	void ClearShopItems();
	void GetShopItems(OlShopItemVector& _refItems);

private:
	int m_nQuerySeed;
	OlShopBuyQueryInfoMap m_xOlShopBuyQueryInfoMap;
	OlShopItemList m_xOlShopItemList;
};
//////////////////////////////////////////////////////////////////////////
#endif