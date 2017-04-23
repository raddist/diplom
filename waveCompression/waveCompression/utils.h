#include <stdio.h>
#include <process.h>
#include <cstdint>
#include "wavelet.h"
#include "bmpStream.h"
#include "quantor.h"
#include "arcoder.h"
#include "contextArcoder.h"
#include "signContextArcoder.h"

#pragma once


typedef std::pair<double, double> MinMax;

template<typename T>
void compareArrays(T* i_left, T* i_right, int i_size)
{
	for (int i = 0; i < i_size; ++i)
	{
		T left = i_left[i];
		T right = i_right[i];
		if (left != right)
		{
			int temp = 0;
		}
	}
}

template<typename T>
void compareArraysThroughMap(T* i_left, T* i_right, int i_size, SubbandMap map)
{
	int hLeftIndex = map.m_hSize[0];
	int vTopIndex = map.m_vSize[0];
	int hRightIndex = map.m_hSize[1];
	int vBotIndex = map.m_vSize[1];
	int imgWidth = map.m_hSize[map.steps];
	int imgHeight = map.m_vSize[map.steps];

	// read minimal left top subband
	SubbandRect rect(0, hLeftIndex, 0, vTopIndex);
	compareSubband(i_left, i_right, rect, imgWidth);

	// read other data
	for (int k = 0; k < map.steps; ++k)
	{
		SubbandRect rightTop(hLeftIndex, hRightIndex, 0, vTopIndex);
		compareSubband(i_left, i_right, rightTop, imgWidth);

		SubbandRect leftBot(0, hLeftIndex, vTopIndex, vBotIndex);
		compareSubband(i_left, i_right, leftBot, imgWidth);

		SubbandRect rightBot(hLeftIndex, hRightIndex, vTopIndex, vBotIndex);
		compareSubband(i_left, i_right, rightBot, imgWidth);

		hLeftIndex = map.m_hSize[k + 1];
		vTopIndex = map.m_vSize[k + 1];
		hRightIndex = map.m_hSize[(k + 2) % 5];
		vBotIndex = map.m_vSize[(k + 2) % 5];
	}
}

template<typename T>
void compareSubband(T* i_left, T* i_right, SubbandRect rect, int imgWidth)
{
	int horizontalFrom = rect.left;
	int horizontalTo = rect.right - 1;	// include last pixel in [horizontalFrom horizontalTo]

	for (int j = rect.top; j < rect.bot; ++j)
	{
		for (int i = horizontalFrom; i <= horizontalTo; ++i)
		{
			int index = j*imgWidth + i;

			T left = i_left[index];
			T right = i_right[index];
			if (left != right)
			{
				int temp = 0;
			}
		}
	}
}

template<typename T>
MinMax findMinMax(T* i_arr, int i_size)
{
	T min = i_arr[0];
	T max = i_arr[0];

	for (int i = 0; i < i_size; ++i)
	{
		if (min > i_arr[i])
		{
			min = i_arr[i];
		}
		if (max < i_arr[i])
		{
			max = i_arr[i];
		}
	}
	MinMax res;
	res.first = static_cast<double>(min);
	res.second = static_cast<double>(max);
	return res;
}

// @brief
void waveletTransform2D(double* i_input, double* o_output,
	int i_width, int i_height, int i_steps)
{
	Wavelet wvlt(&Antonini);
	wvlt.transform2d(static_cast<Real*>(i_input), static_cast<Real*>(o_output), i_width, i_height, i_steps);
}

// @brief
void waveletInvert2D(double* i_input, double* o_output,
	int i_width, int i_height, int i_steps)
{
	Wavelet wvlt(&Antonini);
	wvlt.invert2d(static_cast<Real*>(i_input), static_cast<Real*>(o_output), i_width, i_height, i_steps);
}

// @brief
qMinCap quantilization(double* i_input, int* o_output, int i_imgSize, double i_q = 0)
{
	Quantor quant;
	return quant.quantArray(i_input, o_output, i_imgSize, i_q);
}

// @brief
void deQuantilization(int* i_input, double* o_output, int i_imgSize, double i_q = 0)
{
	Quantor quant;
	quant.deQuantArray(i_input, o_output, i_imgSize, i_q);
}
