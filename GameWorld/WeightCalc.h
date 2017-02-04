#ifndef _INC_WEIGHTCALC_
#define _INC_WEIGHTCALC_
//////////////////////////////////////////////////////////////////////////
#define DEFAULT_WEIGHTCALC_SIZE	32
//////////////////////////////////////////////////////////////////////////
#include <vector>
//////////////////////////////////////////////////////////////////////////
struct WeightCalcItem
{
	int nWeight;
	int nValue;
	// private fields
	int nID;
};
typedef std::vector<WeightCalcItem> WeightCalcItemVector;
//////////////////////////////////////////////////////////////////////////
class WeightCalc
{
public:
	WeightCalc();
	~WeightCalc();

public:
	int AppendWeight(int _nWeight, int _nValue);
	void RemoveWeight(int _nID);
	void SetWeight(int _nID, int _nWeight);
	void SetWeightByValue(int _nValue, int _nWeight);
	void Reset();

	int Get() const;
	const WeightCalcItemVector& GetWeightCalcItems()		{return m_xItems;}
	bool Calculable()										{return m_nTotalWeight != 0;}

private:
	int m_nTotalWeight;
	WeightCalcItemVector m_xItems;
};
//////////////////////////////////////////////////////////////////////////
#endif