#include <stdio.h>
#include <process.h>
#include "contextArcoder.h"

#include <vector>

#pragma warning(disable: 4996)

///////////////////////////////////////////////////////////////////////
ContextArcoder::ContextArcoder()
{
}

///////////////////////////////////////////////////////////////////////
ContextArcoder::ContextArcoder(qMinCap i_qMinCap, Context3x3 i_context) :
	m_qStruct(i_qMinCap),
	m_context(i_context)
{
	limits.push_back(0.2);
	limits.push_back(1);
	limits.push_back(10);
}

///////////////////////////////////////////////////////////////////////
ContextArcoder::ContextArcoder(qMinCap i_qMinCap,
								Context3x3 i_context,
								Limits i_limits) :
	m_qStruct(i_qMinCap),
	m_context(i_context),
	limits(i_limits)
{
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::initialize_model(void)
{
	// first group of models for right top subband
	m_numOfModelsNeeded = m_qStruct.mainCapacity;
	m_model.clear();

	for (int i = 0; i < m_numOfModelsNeeded; ++i)
	{
		m_model.emplace_back(m_qStruct.mainCapacity);
	}
	conv.Initialize(m_qStruct.mainCapacity, m_qStruct.mainMin);

	start_model();
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::reset_model(void)
{
	// reset models to work with extra subbands
	m_model.clear();
	m_numOfModelsNeeded = limits.size() + 1;

	for (int i = 0; i < m_numOfModelsNeeded; ++i)
	{
		m_model.emplace_back(m_qStruct.extraCapacity);
	}
	conv.Initialize(m_qStruct.extraCapacity, m_qStruct.extraMin);

	start_model();
}

///////////////////////////////////////////////////////////////////////
double ContextArcoder::calcP(int index, int *decoded_data, bool i_isOnTheBord)
{
	double sum = 0;
	int lim = i_isOnTheBord ? 2 : 3;
	for (int j = 0; j < lim; ++j)
	{
		for (int i = 0; i < lim; ++i)
		{
			int temp_index = index + (-1 + j)*imgWidth + (-1 + i);
			switch (m_subbandType)
			{
			case 1: sum += m_context.maskV[3 * j + i] * abs(decoded_data[temp_index]);
				break;
			case 2: sum += m_context.maskH[3 * j + i] * abs(decoded_data[temp_index]);
				break;
			case 3: sum += m_context.maskD[3 * j + i] * abs(decoded_data[temp_index]);
				break;
			}
		}
	}
	return sum;
}

///////////////////////////////////////////////////////////////////////
int ContextArcoder::findModelByP(double p)
{
	for (int i = 0; i < static_cast<int>(limits.size()); ++i)
	{
		if (p < limits.at(i))
		{
			return i;
		}
	}
	return limits.size();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////
///// encode
/////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
void ContextArcoder::basicEncode(int i_index)
{
	int symbol = m_decodedData[i_index];

	uint uSymbol = conv.ConvertToUnsigned(symbol);
	Arcoder::encode_symbol(uSymbol);
	update_model(uSymbol);
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::encodeSymbolByContext(int index, bool i_isOnTheBord)
{
	double p = calcP(index, m_decodedData, i_isOnTheBord);
	m_currentModel = findModelByP(p);
	int symbol = m_decodedData[index];

	uint uSymbol = conv.ConvertToUnsigned(symbol);
	Arcoder::encode_symbol(uSymbol);
	update_model(uSymbol);
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::encode(int* in, int8_t* out, SubbandMap map, int size_in, int &size_out)
{
	m_decodedData = in;
	m_encodedData = out;

	int hLeftIndex = map.m_hSize[0];
	int vTopIndex = map.m_vSize[0];
	int hRightIndex = map.m_hSize[1];
	int vBotIndex = map.m_vSize[1];
	imgWidth = map.m_hSize[map.steps];
	imgHeight = map.m_vSize[map.steps];

	initialize_model();
	start_encoding();

	// read minimal left top subband
	SubbandRect rect(0, hLeftIndex, 0, vTopIndex);
	encodeSubband(rect);
	reset_model();

	// encode top row and left col
	m_subbandType = 1;
	encodeTopRow(0, hLeftIndex);
	encodeTopRow(1, hLeftIndex);

	m_subbandType = 2;
	encodeLeftColumn(0, vTopIndex);
	encodeLeftColumn(1, vTopIndex);
	//reset_model();

	// read other data
	for (int k = 0; k < map.steps; ++k)
	{
		// right top
		m_subbandType = 1;
		SubbandRect rightTop(hLeftIndex, hRightIndex, 0, vTopIndex);
		encodeSubband(rightTop);


		//left bot
		m_subbandType = 2;
		SubbandRect leftBot(0, hLeftIndex, vTopIndex, vBotIndex);
		encodeSubband(leftBot);

		//right bot
		m_subbandType = 3;
		SubbandRect rightBot(hLeftIndex, hRightIndex, vTopIndex, vBotIndex);
		encodeSubband(rightBot);

		hLeftIndex = map.m_hSize[k + 1];
		vTopIndex = map.m_vSize[k + 1];
		hRightIndex = map.m_hSize[(k + 2) % 5];
		vBotIndex = map.m_vSize[(k + 2) % 5];
	}

	done_encoding();

	m_subbandType = 0;
	size_out = sizeOut;
	sizeIn = 0;
	sizeOut = 0;
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::encodeSubband(SubbandRect rect)
{
	int horizontalFrom = rect.left;
	int horizontalTo = rect.right - 1;	// include last pixel in [horizontalFrom horizontalTo]

	for (int j = rect.top; j < rect.bot; ++j)
	{
		for (int i = horizontalFrom; i <= horizontalTo; ++i)
		{
			int index = j*imgWidth + i;
			m_curSymbolIndex = index;

			if (rect.top == 0 && rect.left == 0)
			{
				basicEncode(index);
			}
			else if (j > 1 && i > 1)
			{
				if (j != rect.bot-1 && i != horizontalTo)
				{
					encodeSymbolByContext(index);
				}
				else
				{
					encodeSymbolByContext(index, true);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::encodeTopRow(int i_row, int i_startIndex)
{
	for (int col = i_startIndex; col < imgWidth; ++col)
	{
		int index = i_row * imgWidth + col;
		basicEncode(index);
	}
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::encodeLeftColumn(int i_col, int i_startIndex)
{
	for (int row = i_startIndex; row < imgHeight; ++row)
	{
		int index = row * imgWidth + i_col;
		basicEncode(index);
		m_currentModel = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////
///// decode
/////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
void ContextArcoder::basicDecode(int i_index)
{
	int uSymbol = Arcoder::decode_symbol();
	update_model(uSymbol);
	int symbol = conv.ConvertToSigned(uSymbol);

	m_decodedData[i_index] = symbol;
	sizeOut++;
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::decodeSymbolByContext(int index, bool i_isOnTheBord)
{
	double p = calcP(index, m_decodedData, i_isOnTheBord);
	m_currentModel = findModelByP(p);

	int uSymbol = Arcoder::decode_symbol();
	update_model(uSymbol);
	int symbol = conv.ConvertToSigned(uSymbol);

	m_decodedData[index] = symbol;
	sizeOut++;
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::decode(int8_t* in, int* out, SubbandMap map, int &size_out)
{
	m_encodedData = in;
	m_decodedData = out;

	int hLeftIndex = map.m_hSize[0];
	int vTopIndex = map.m_vSize[0];
	int hRightIndex = map.m_hSize[1];
	int vBotIndex = map.m_vSize[1];
	imgWidth = map.m_hSize[map.steps];
	imgHeight = map.m_vSize[map.steps];

	initialize_model();
	start_decoding();

	// read minimal left top subband
	SubbandRect rect(0, hLeftIndex, 0, vTopIndex);
	decodeSubband(rect);
	reset_model();

	// decode top row and left col
	m_subbandType = 1;
	decodeTopRow(0, hLeftIndex);
	decodeTopRow(1, hLeftIndex);

	m_subbandType = 2;
	decodeLeftColumn(0, vTopIndex);
	decodeLeftColumn(1, vTopIndex);
	//reset_model();

	// read other data
	for (int k = 0; k < map.steps; ++k)
	{
		// right top
		m_subbandType = 1;
		SubbandRect rightTop(hLeftIndex, hRightIndex, 0, vTopIndex);
		decodeSubband(rightTop);

		//left bot
		m_subbandType = 2;
		SubbandRect leftBot(0, hLeftIndex, vTopIndex, vBotIndex);
		decodeSubband(leftBot);

		//right bot
		m_subbandType = 3;
		SubbandRect rightBot(hLeftIndex, hRightIndex, vTopIndex, vBotIndex);
		decodeSubband(rightBot);

		hLeftIndex = map.m_hSize[k + 1];
		vTopIndex = map.m_vSize[k + 1];
		hRightIndex = map.m_hSize[(k + 2) % 5];
		vBotIndex = map.m_vSize[(k + 2) % 5];
	}

	m_subbandType = 0;
	size_out = sizeOut;
	sizeIn = 0;
	sizeOut = 0;
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::decodeSubband(SubbandRect rect)
{
	int horizontalFrom = rect.left;
	int horizontalTo = rect.right - 1;	// include last pixel in [horizontalFrom horizontalTo]

	for (int j = rect.top; j < rect.bot; ++j)
	{
		for (int i = horizontalFrom; i <= horizontalTo; ++i)
		{
			int index = j*imgWidth + i;
			m_curSymbolIndex = index;

			if (rect.top == 0 && rect.left == 0)
			{
				basicDecode(index);
			}
			else if (j > 1 && i > 1)
			{
				if (j != rect.bot - 1 && i != horizontalTo)
				{
					decodeSymbolByContext(index);
				}
				else
				{
					decodeSymbolByContext(index, true);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::decodeTopRow(int i_row, int i_startIndex)
{
	for (int col = i_startIndex; col < imgWidth; ++col)
	{
		int index = i_row * imgWidth + col;
		basicDecode(index);
	}
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::decodeLeftColumn(int i_col, int i_startIndex)
{
	for (int row = i_startIndex; row < imgHeight; ++row)
	{
		int index = row * imgWidth + i_col;
		basicDecode(index);
		m_currentModel = 0;
	}
}

