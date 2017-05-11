#pragma once

#include <math.h>
#include <cstdint>
#include <vector>

#define y 0.375

template <typename T1> int sgn(T1 val) 
{
	return (T1(0) < val) - (val < T1(0));
}

template <typename T2> T2 maximum(T2 left, T2 right) 
{
	return (left > right) ? left : right;
}

template <typename T3> T3 abs(T3 value)
{
	return (value > 0) ? value : -value;
}

/////////////////////////////////////////////////////////////////////////////////////////
class SubbandMap
{
public:
	SubbandMap(int hSize, int vSize, int i_steps)
	{
		int levels = i_steps + 1;

		while (levels--)
		{
			m_hSize.insert(m_hSize.begin(), hSize);
			m_vSize.insert(m_vSize.begin(), vSize);

			hSize = (hSize + 1) / 2;
			vSize = (vSize + 1) / 2;
		}
		steps = i_steps;
	}

	~SubbandMap()
	{};

public:

	std::vector<int> m_hSize;
	std::vector<int> m_vSize;
	int steps;
};

typedef struct qMinCap {
	double q;
	int minValue;
	int arrCapacity;

	int mainMin;
	int mainCapacity;

	int extraMin;
	int extraCapacity;
} qMinCap;

typedef struct {
	double qPos;
	double qNeg;
	uint8_t negativeShift;
} qPair;

class Quantor
{
public:

	Quantor()
	{
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	qMinCap quantArray(double* input, int* output, int size, double i_q);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	qMinCap quantArrayByMap(double* input, int* output, int size, double i_q, SubbandMap i_map);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void deQuantArray(int* input, double* output, int size, double i_q);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	int8_t* myQuantArray(qPair q, double* input, int size);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void myDeQuantArray(qPair q, int8_t* input, double* output, int size);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	void findMinMax(T* input, int size, T &min, T &max);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	double findQ(double min, double max);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	qPair myFindQ(double min, double max);
};
