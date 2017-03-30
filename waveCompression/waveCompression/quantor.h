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

typedef struct {
	double qPos;
	double qNeg;
	uint8_t negativeShift;
} qPair;

class Quantor
{
public:

	Quantor(double i_q)
	{
		m_q = i_q;
	}
	template <typename T>
	Quantor(T* input, int size)
	{
		T min;
		T max;
		findMinMax(input, size, min, max);
		m_q = findQ(min, max);
	}

	uint8_t* quantArray(double* input, int size);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void deQuantArray(uint8_t* input, double* output, int size);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	uint8_t* myQuantArray(qPair q, double* input, int size);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void myDeQuantArray(qPair q, uint8_t* input, double* output, int size);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void findMinMax(double* input, int size, double &min, double &max);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	void findMinMax(T* input, int size, T &min, T &max);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	double findQ(double min, double max);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	qPair myFindQ(double min, double max);

private:
	double m_q = 1;
};
