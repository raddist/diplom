#include "quantor.h"




/////////////////////////////////////////////////////////////////////////////////////////////////
int8_t* Quantor::quantArray(double* input, int size)
{
	int8_t* output = new int8_t[size];

	for (int i = 0; i < size; ++i)
	{
		output[i] = static_cast<int8_t>(floor(abs(input[i] / m_q)) *sgn(input[i]));
	}

	return output;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void Quantor::deQuantArray(int8_t* input, double* output, int size)
{
	for (int i = 0; i < size; ++i)
	{
		int8_t temp = static_cast<int8_t>(input[i]);
		output[i] = m_q * (temp + sgn(temp) * y);
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
			output[i] = static_cast<int8_t>(floor(abs(input[i] / q.qPos)) + q.negativeShift);
		}
		else
		{
			output[i] = static_cast<int8_t>(floor(abs(input[i] / q.qNeg)) * (-1) + q.negativeShift);
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
void Quantor::findMinMax(double* input, int size, double &min, double &max)
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
double Quantor::findQ(double i_min, double i_max)
{
	double q = 0;
	q = maximum(abs(i_max) / 127.0, abs(i_min) / 128.0);
	return q;
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