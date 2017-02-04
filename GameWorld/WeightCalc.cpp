#include "WeightCalc.h"
//////////////////////////////////////////////////////////////////////////
static int s_nWeightCalcItemIDSeed = 0;
//////////////////////////////////////////////////////////////////////////
WeightCalc::WeightCalc()
{
	m_xItems.reserve(DEFAULT_WEIGHTCALC_SIZE);
	m_nTotalWeight = 0;
}

WeightCalc::~WeightCalc()
{

}


int WeightCalc::AppendWeight(int _nWeight, int _nValue)
{
	m_nTotalWeight += _nWeight;

	// first find zero item and set it
	for (size_t i = 0; i < m_xItems.size(); ++i)
	{
		if (m_xItems[i].nID == 0)
		{
			m_xItems[i].nWeight = _nWeight;
			m_xItems[i].nValue = _nValue;
			m_xItems[i].nID = ++s_nWeightCalcItemIDSeed;

			return m_xItems[i].nID;
		}
	}

	// new item and push_back
	WeightCalcItem item;
	item.nWeight = _nWeight;
	item.nValue = _nValue;
	item.nID = ++s_nWeightCalcItemIDSeed;
	m_xItems.push_back(item);

	return item.nID;
}

void WeightCalc::RemoveWeight(int _nID)
{
	for (size_t i = 0; i < m_xItems.size(); ++i)
	{
		if (m_xItems[i].nID == _nID)
		{
			m_xItems[i].nID = 0;
			m_nTotalWeight -= m_xItems[i].nWeight;
			return;
		}
	}
}

void WeightCalc::SetWeight(int _nID, int _nWeight)
{
	if (0 == _nID)
	{
		return;
	}

	for (size_t i = 0; i < m_xItems.size(); ++i)
	{
		if (m_xItems[i].nID == _nID)
		{
			// change total weight
			int nWeightDiff = _nWeight - m_xItems[i].nWeight;
			m_xItems[i].nWeight = _nWeight;
			m_nTotalWeight += nWeightDiff;
			return;
		}
	}
}

void WeightCalc::SetWeightByValue(int _nValue, int _nWeight)
{
	for (size_t i = 0; i < m_xItems.size(); ++i)
	{
		if (m_xItems[i].nValue == _nValue)
		{
			// change total weight
			int nWeightDiff = _nWeight - m_xItems[i].nWeight;
			m_xItems[i].nWeight = _nWeight;
			m_nTotalWeight += nWeightDiff;
			return;
		}
	}
}

void WeightCalc::Reset()
{
	for (size_t i = 0; i < m_xItems.size(); ++i)
	{
		m_xItems[i].nID = 0;
	}
	m_nTotalWeight = 0;
}

int WeightCalc::Get() const
{
	if (0 == m_nTotalWeight)
	{
		return 0;
	}

	int nRand = rand() % m_nTotalWeight;
	int nWeight = 0;

	for (size_t i = 0; i < m_xItems.size(); ++i)
	{
		if (0 == m_xItems[i].nID)
		{
			continue;
		}

		nWeight += m_xItems[i].nWeight;
		if (nRand < nWeight)
		{
			return m_xItems[i].nValue;
		}
	}
	
	return 0;
}