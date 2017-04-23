#include "quantor.h"




/////////////////////////////////////////////////////////////////////////////////////////////////
qMinCap Quantor::quantArray(double* input, int* output, int size, double i_q)
{
	for (int i = 0; i < size; ++i)
	{
		output[i] = static_cast<int>(floor(abs(input[i] / i_q)) *sgn(input[i]));
	}

	int min = 0;
	int max = 0;
	findMinMax(output, size, min, max);

	qMinCap res;
	res.q = i_q;
	res.minValue = min;
	res.arrCapacity = max - min + 1;

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void Quantor::deQuantArray(int* input, double* output, int size, double i_q)
{
	for (int i = 0; i < size; ++i)
	{
		int temp = static_cast<int>(input[i]);
		output[i] = i_q * (temp + sgn(temp) * y);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int8_t* Quantor::myQuantArray(qPair q, double* input, int size)
{
	int8_t* output = new int8_t[size];

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
void Quantor::myDeQuantArray(qPair q, int8_t* input, double* output, int size)
{
	for (int i = 0; i < size; ++i)
	{
		int temp = static_cast<int>(input[i] - q.negativeShift);
		if (temp > 0)
		{
			output[i] = q.qPos * (temp + sgn(temp) * y);
		}
		else
		{
			output[i] = q.qNeg * (temp + sgn(temp) * y);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
void Quantor::findMinMax(T* input, int size, T &min, T &max)
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
qPair Quantor::myFindQ(double min, double max)
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