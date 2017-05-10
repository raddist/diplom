#pragma once

#include <math.h>
#include <cstdint>

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
