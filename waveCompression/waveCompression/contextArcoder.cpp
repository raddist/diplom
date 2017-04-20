#include <stdio.h>
#include <process.h>
#include "contextArcoder.h"

#include <vector>

#pragma warning(disable: 4996)

///////////////////////////////////////////////////////////////////////
ContextArcoder::ContextArcoder() :
	m_subbandType(0)
{
}

///////////////////////////////////////////////////////////////////////
ContextArcoder::ContextArcoder(Context3x3 i_context) :
	m_context(i_context),
	m_subbandType(0)
{
	limits.push_back(0.2);
	limits.push_back(1);
	limits.push_back(10);

	m_numOfModelsNeeded = NO_OF_CHARS;

	for (int i = 0; i < m_numOfModelsNeeded; ++i)
	{
		m_model.emplace_back();
	}
	conv.Initialize();
}

///////////////////////////////////////////////////////////////////////
ContextArcoder::ContextArcoder(Context3x3 i_context,
								Limits i_limits) :
	m_context(i_context),
	m_subbandType(0)
{
	limits = i_limits;

	m_numOfModelsNeeded = NO_OF_CHARS;

	for (int i = 0; i < m_numOfModelsNeeded; ++i)
	{
		m_model.emplace_back();
	}
	conv.Initialize();
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::reset_model(void)
{
	// just reset models
	start_model();
}

///////////////////////////////////////////////////////////////////////
double ContextArcoder::calcP(int index, int8_t *decoded_data, bool i_isOnTheBord)
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
	for (int i = 0; i < static_cast<int>(limits.size() - 1); ++i)
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
	int8_t symbol = data_in[i_index];

	uint8_t uSymbol = conv.ConvertToUnsigned(symbol);
	Arcoder::encode_symbol(uSymbol);
	update_model(uSymbol);
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::encodeSymbolByContext(int index, bool i_isOnTheBord)
{
	double p = calcP(index, data_in, i_isOnTheBord);
	m_currentModel = findModelByP(p);
	int8_t symbol = data_in[index];

	uint8_t uSymbol = conv.ConvertToUnsigned(symbol);
	Arcoder::encode_symbol(uSymbol);
	update_model(uSymbol);
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::encode(int8_t* in, int8_t* out, SubbandMap map, int size_in, int &size_out)
{
	data_in = in;
	data_out = out;

	int hLeftIndex = map.m_hSize[0];
	int vTopIndex = map.m_vSize[0];
	int hRightIndex = map.m_hSize[1];
	int vBotIndex = map.m_vSize[1];
	imgWidth = map.m_hSize[map.steps];
	imgHeight = map.m_vSize[map.steps];

	start_model();
	start_encoding();

	// read minimal left top subband
	SubbandRect rect(0, hLeftIndex, 0, vTopIndex);
	encodeSubband(rect);
	reset_model();

	// encode top row and left col
	encodeTopRow(hLeftIndex, imgWidth);
	reset_model();

	encodeLeftColumn(vTopIndex, imgHeight);
	reset_model();

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

	encode_symbol(EOF_SYMBOL);
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
			else if (j != 0 && i != 0)
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
void ContextArcoder::encodeTopRow(int startIndex, int endIndex)
{
	for (int i = startIndex; i < endIndex; ++i)
	{
		basicEncode(i);
	}
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::encodeLeftColumn(int startIndex, int endIndex)
{
	for (int j = startIndex; j < endIndex; ++j)
	{
		int index = j*imgWidth;
		basicEncode(index);
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
	int8_t symbol = conv.ConvertToSigned(uSymbol);

	data_out[i_index] = symbol;
	sizeOut++;
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::decodeSymbolByContext(int index, bool i_isOnTheBord)
{
	double p = calcP(index, data_out, i_isOnTheBord);
	m_currentModel = findModelByP(p);

	int uSymbol = Arcoder::decode_symbol();
	update_model(uSymbol);
	int8_t symbol = conv.ConvertToSigned(uSymbol);

	data_out[index] = symbol;
	sizeOut++;
}

///////////////////////////////////////////////////////////////////////
void ContextArcoder::decode(int8_t* in, int8_t* out, SubbandMap map, int &size_out)
{
	data_in = in;
	data_out = out;

	int hLeftIndex = map.m_hSize[0];
	int vTopIndex = map.m_vSize[0];
	int hRightIndex = map.m_hSize[1];
	int vBotIndex = map.m_vSize[1];
	imgWidth = map.m_hSize[map.steps];
	imgHeight = map.m_vSize[map.steps];

	start_model();
	start_decoding();

	// read minimal left top subband
	SubbandRect rect(0, hLeftIndex, 0, vTopIndex);
	decodeSubband(rect);
	reset_model();

	// decode top row and left col
	decodeTopRow(hLeftIndex, imgWidth);
	reset_model();

	decodeLeftColumn(vTopIndex, imgHeight);
	reset_model();

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
			else if (j != 0 && i != 0)
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
void ContextArcoder::decodeTopRow(int startIndex, int endIndex)
{
	for (int i = startIndex; i < endIndex; ++i)
	{
		basicDecode(i);
	}
}

// TODO change row and col logic
///////////////////////////////////////////////////////////////////////
void ContextArcoder::decodeLeftColumn(int startIndex, int endIndex)
{
	for (int j = 0; j < endIndex - startIndex; ++j)
	{
		int index = startIndex + j*imgWidth;
		basicDecode(index);
	}
}

