#pragma once

#ifndef  __MATH_H
#include <math.h>
#endif

#define y 0.375

typedef struct {
	double qPos;
	double qNeg;
	uint8_t negativeShift;
} qPair;

/////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t* quantArray(double q, double* input, int size)
{
	uint8_t* output = new uint8_t[size];

	for (int i = 0; i < size; ++i)
	{
		output[i] = static_cast<uint8_t>(floor(abs(input[i] / q)) *sign(input[i]) + 128);
	}

	return output;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void deQuantArray(double q, uint8_t* input, double* output, int size)
{
	for (int i = 0; i < size; ++i)
	{
		int8_t temp = static_cast<int8_t>(input[i] - 128);
		output[i] = q * (temp + sign(temp) * y);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t* myQuantArray(qPair q, double* input, int size)
{
	uint8_t* output = new uint8_t[size];

	for (int i = 0; i < size; ++i)
	{
		if (input[i] > 0)
		{
			output[i] = static_cast<uint8_t>(floor(abs(input[i] / q.qPos)) + q.negativeShift);
		}
		else
		{
			output[i] = static_cast<uint8_t>(floor(abs(input[i] / q.qNeg)) * (-1) + q.negativeShift);
		}
	}

	return output;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void myDeQuantArray(qPair q, uint8_t* input, double* output, int size)
{
	for (int i = 0; i < size; ++i)
	{
		int temp = static_cast<int>(input[i] - q.negativeShift);
		if (temp > 0)
		{
			output[i] = q.qPos * (temp + sign(temp) * y);
		}
		else
		{
			output[i] = q.qNeg * (temp + sign(temp) * y);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void findMinMax(double* input, int size, double &min, double &max)
{
	min = max = input[0];

	for (int i = 1; i < size; ++i)
	{
		if (input[i] > max)
		{
			max = input[i];
		}
		if (input[i] < min)
		{
			min = input[i];
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void findMinMax(uint8_t* input, int size, uint8_t &min, uint8_t &max)
{
	min = max = input[0];

	for (int i = 1; i < size; ++i)
	{
		if (input[i] > max)
		{
			max = input[i];
		}
		if (input[i] < min)
		{
			min = input[i];
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
double findQ(double min, double max)
{
	double q = 0;
	q = max(abs(max) / 127.0, abs(min) / 128.0);
	return q;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
qPair myFindQ(double min, double max)
{
	double negativePart = abs(min) / (abs(min) + abs(max));
	int negativePartOf256 = static_cast<int>(floor(256 * negativePart));
	int positivePartOf256 = (256 - 1 - negativePartOf256);
	qPair q;
	q.qNeg = abs(min) / negativePartOf256;
	q.qPos = abs(max) / positivePartOf256;
	q.negativeShift = negativePartOf256;
	return q;
}
