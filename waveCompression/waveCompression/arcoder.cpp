#include <stdio.h>
#include <process.h>
#include "arcoder.h"

#include <vector>

#pragma warning(disable: 4996)

Arcoder::Arcoder(uint8_t* in):
	data_in(in)
{
}

// @brief ������������� ������� ������
void Arcoder::start_model(void)
{
	for (int k = 0; k < NO_OF_CHARS; ++k)
	{
		// ������� ��� ������� � ��������� ������� ��������������� 
		for (int i = 0; i <= NO_OF_SYMBOLS; i++)	cum_freq[k][i] = i;
	}
}

// @brief ���������� ������� ������
// @param symbol - in, ����������� ������
inline void Arcoder::update_model(int symbol)
{
	if (cum_freq[prev_symbol][NO_OF_SYMBOLS] == MAX_FREQUENCY)
	{	//	������������ ��������� ���������, �������� �� � 2 ����
		int cum = 0;
		for (int i = 0; i < NO_OF_SYMBOLS; i++)
		{
			int fr = (cum_freq[prev_symbol][i + 1] - cum_freq[prev_symbol][i] + 1) >> 1;
			cum_freq[prev_symbol][i] = cum;
			cum += fr;
		}
		cum_freq[prev_symbol][NO_OF_SYMBOLS] = cum;
	}
	// ���������� ���������� ������
	for (int i = symbol + 1; i <= NO_OF_SYMBOLS; i++) cum_freq[prev_symbol][i]++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////
///// encode
/////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// @brief ������������� ���������� ����������
void Arcoder::start_encoding(void)
{
	bits_to_go = 8;				// �������� ��� � ������� ������ ������
	bits_to_follow = 0;				// ����� ���, ����� ������� �������
	low = 0;				// ������ ������� ���������
	high = TOP_VALUE;		// ������� ������� ���������
}

// @brief ����� ������ ���� � ������ ����
inline void Arcoder::output_bit(int bit)
{
	buffer = (buffer >> 1) + (bit << 7);	// � ������� ����� (���� ����)
	bits_to_go--;
	if (bits_to_go == 0)				// ������� ����� ��������, ����� ������
	{
		*data_out = buffer;
		data_out++;
		sizeOut++;

		bits_to_go = 8;
	}
}

// @brief ����� ������ ���������� ���� � ���, ������� ���� ��������
inline void Arcoder::output_bit_plus_follow(int bit)
{
	output_bit(bit);
	while (bits_to_follow>0)
	{
		output_bit(!bit);
		bits_to_follow--;
	}
}

// @brief ���������� �����������
void Arcoder::done_encoding(void)
{
	bits_to_follow++;
	if (low < FIRST_QTR) output_bit_plus_follow(0);
	else output_bit_plus_follow(1);
	*data_out = buffer >> bits_to_go;				// �������� ������������� �����
	sizeOut++;
}

// @brief ����������� �������
// @param symbol - in, ����������� ������
void Arcoder::encode_symbol(int symbol)
{
	// �������� ������ ���������
	unsigned long range;
	range = high - low + 1;
	high = low + range*cum_freq[prev_symbol][symbol + 1] / cum_freq[prev_symbol][NO_OF_SYMBOLS] - 1;
	low = low + range*cum_freq[prev_symbol][symbol] / cum_freq[prev_symbol][NO_OF_SYMBOLS];
	// ����� ��� ������������� - ����� ���� ��� ���� �� ������������
	for (;;)
	{			// ���������: ������ low<high
		if (high<HALF) // ������� ���� low � high - ������� (���)
			output_bit_plus_follow(0); //����� ������������ �������� ����
		else if (low >= HALF) // ������� ���� low � high - ���������
		{
			output_bit_plus_follow(1);	//����� �������� ����
			low -= HALF;					//����� �������� ���� � 0
			high -= HALF;					//����� �������� ���� � 0
		}
		else if (low >= FIRST_QTR && high < THIRD_QTR)
		{		/* �������� ������������, �.�.
				HALF<=high<THIRD_QTR,	i.e. high=10...
				FIRST_QTR<=low<HALF,	i.e. low =01...
				����������� ������ �� ����������� ���	*/
			high -= FIRST_QTR;		// high	=01...
			low -= FIRST_QTR;			// low	=00...
			bits_to_follow++;		//����������� ����� (���) ������ ����
									// ������� ��� ����� ������ �����
		}
		else break;		// ��������� ����� ��� ���� 
						//	������� ��� � low � high �������, ��������� ����� ��� � ������� ������ 
		low <<= 1;			// ��������� 0
		(high <<= 1)++;		// ��������� 1
	}
}

// @brief ����������� ����������
void Arcoder::encode(uint8_t* in, uint8_t* out, int size_in, int &size_out)
{
	data_in = in;
	data_out = out;

	uint8_t symbol;
	int prev_symbol = 0;
	start_model();
	start_encoding();

	int size = 0;

	// read data
	for (int i = 0; i < size_in; ++i)
	{
		symbol = data_in[i];
		encode_symbol(symbol);
		update_model(symbol);

		prev_symbol = symbol;
	}

	encode_symbol(EOF_SYMBOL);
	done_encoding();

	sizeIn = 0;
	size_out = sizeOut;
	sizeOut = 0;
}

//
void Arcoder::encodeSubband(SubbandRect rect)
{
	int horizontalFrom = rect.left;
	int horizontalTo = rect.right - 1;	// include last pixel in [horizontalFrom horizontalTo]
	int step = 1;

	for (int j = rect.top; j < rect.bot; ++j)
	{

		for (int i = horizontalFrom; i*(step) <= horizontalTo*(step); i += step)
		{
			int index = j*imgWidth + i;
			uint8_t symbol = data_in[index];

			encode_symbol(symbol);
			update_model(symbol);

			if (!m_isOneModel)
			{
				prev_symbol = symbol;
			}
		}

		// change direction
		int temp = horizontalFrom;
		horizontalFrom = horizontalTo;
		horizontalTo = temp;
		step *= -1;
	}
}

// @brief ����������� ���������� � ���������� �������
void Arcoder::mappedEncode(uint8_t* in, uint8_t* out, CoderMap map, int &size_out)
{
	data_in = in;
	data_out = out;
	prev_symbol = 0;

	int hLeftIndex = map.m_hSize[0];
	int vTopIndex = map.m_vSize[0];
	int hRightIndex = map.m_hSize[1];
	int vBotIndex = map.m_vSize[1];
	imgWidth = map.m_hSize[map.steps];

	start_model();
	start_encoding();

	int size = 0;

	// read minimal left top subband
	SubbandRect rect(0, hLeftIndex, 0, vTopIndex);
	encodeSubband(rect);

	// read other data
	for (int k = 0; k < map.steps; ++k)
	{
		// right top
		SubbandRect rightTop(hLeftIndex, hRightIndex, 0, vTopIndex);
		encodeSubband(rightTop);


		//left bot
		SubbandRect leftBot(0, hLeftIndex, vTopIndex, vBotIndex);
		encodeSubband(leftBot);

		//right bot
		SubbandRect rightBot(hLeftIndex, hRightIndex, vTopIndex, vBotIndex);
		encodeSubband(rightBot);

		hLeftIndex = map.m_hSize[k + 1];
		vTopIndex = map.m_vSize[k + 1];
		hRightIndex = map.m_hSize[(k + 2) % 5];
		vBotIndex = map.m_vSize[(k + 2) % 5];
	}

	encode_symbol(EOF_SYMBOL);
	done_encoding();

	sizeIn = 0;
	size_out = sizeOut;
	sizeOut = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////
///// decode
/////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// @brief ���� 1 ���� �� ������� �����
inline int Arcoder::input_bit(void)
{
	if (bits_to_go == 0)
	{
		buffer = *data_in;	// ��������� ����� �������� �����
		uint8_t temp = data_in[7212 - 1];
		data_in++;
		sizeIn++;
		temp = data_in[7212 - 1 - sizeIn];
		if (buffer == EOF)	// ������� ����� ������ ������ �������� !!!
		{
			// ������� ������� ����������� ������: ��������� 
			// ������������ �������� ������ ���� EOF_SYMBOL,
			// �� ������� �� ���� ���� �� ����� � ����� ���������� 
			// � ����������� �������������, �������� ����� ���� 
			// (��. ���� for(;;) � ��������� decode_symbol). ��� 
			// ���� - "�����", ������� �� ����� ������� 
			// ���������� � �� ����� ������ ������
			garbage_bits++;
			if (garbage_bits>BITS_IN_REGISTER - 2)
			{	// ������ ����������� ���������� ����� �������� �����
				printf("ERROR IN COMPRESSED FILE !!! \n");
				exit(-1);
			}
			bits_to_go = 1;
		}
		else bits_to_go = 8;
	}
	int t = buffer & 1;
	buffer >>= 1;
	bits_to_go--;
	return t;
}

// @brief ������������� ���������� ����������
void Arcoder::start_decoding(void)
{
	bits_to_go = 0;				// �������� ��� � ������� ������ �����
	garbage_bits = 0;				// �������� ����� "��������" ��� � ����� ������� �����
	low = 0;				// ������ ������� ���������
	high = TOP_VALUE;		// ������� ������� ���������
	value = 0;				// "�����"
	for (int i = 0; i<BITS_IN_REGISTER; i++) value = (value << 1) + input_bit();
}

// @brief ������������� �������
int Arcoder::decode_symbol()
{
	unsigned long range, cum;
	int symbol;
	range = high - low + 1;
	// ����� cum - ��� ����� value, ������������� �� ���������
	// low..high � �������� 0..CUM_FREQUENCY[NO_OF_SYMBOLS]
	cum = ((value - low + 1)*cum_freq[prev_symbol][NO_OF_SYMBOLS] - 1) / range;
	// ����� ���������, ���������������� ����� cum
	for (symbol = 0; cum_freq[prev_symbol][symbol + 1] <= cum; symbol++);
	// �������� ������
	high = low + range*cum_freq[prev_symbol][symbol + 1] / cum_freq[prev_symbol][NO_OF_SYMBOLS] - 1;
	low = low + range*cum_freq[prev_symbol][symbol] / cum_freq[prev_symbol][NO_OF_SYMBOLS];
	for (;;)
	{		// ���������� � ������������� ��������� ��������
		if (high<HALF) {/* ������� ���� low � high - ������� */ }
		else if (low >= HALF)
		{		// ������� ���� low � high - ���������, ����������
			value -= HALF;
			low -= HALF;
			high -= HALF;
		}
		else if (low >= FIRST_QTR && high<THIRD_QTR)
		{		// ��������� ��� ��, ��� ��� �����������
			value -= FIRST_QTR;
			low -= FIRST_QTR;
			high -= FIRST_QTR;
		}
		else break;	// ��������� ����� ��� ���� 
		low <<= 1;							// ��������� ����� ��� 0
		(high <<= 1)++;						// ��������� ����� ��� 1
		value = (value << 1) + input_bit();		// ��������� ����� ��� ����������
	}
	return symbol;
}

// @brief ������������� ����������
void Arcoder::decode(uint8_t* in, uint8_t* out, int size_in, int &size_out)
{
	sizeIn = 0;
	data_in = in;
	data_out = out;

	int symbol;
	int prev_symbol = 0;
	start_model();
	start_decoding();

	while ((symbol = decode_symbol()) != EOF_SYMBOL)
	{
		update_model(symbol);
		*data_out = symbol;
		data_out++;
		sizeOut++;

		if (m_isOneModel)
		{
			prev_symbol = symbol;
		}
	}

	sizeIn = 0;
	size_out = sizeOut;
	sizeOut = 0;
}

// 
void Arcoder::decodeSubband(SubbandRect rect)
{
	int horizontalFrom = rect.left;
	int horizontalTo = rect.right - 1;	// include last pixel in [horizontalFrom horizontalTo]
	int step = 1;

	for (int j = rect.top; j < rect.bot; ++j)
	{
		// *(step) just to move from => to <= and back
		for (int i = horizontalFrom; i*(step) <= horizontalTo*(step); i += step)
		{
			int index = j*imgWidth + i;

			int symbol = decode_symbol();
			update_model(symbol);
			data_out[index] = symbol;
			sizeOut++;

			if (!m_isOneModel)
			{
				prev_symbol = symbol;
			}
		}

		// change direction
		int temp = horizontalFrom;
		horizontalFrom = horizontalTo;
		horizontalTo = temp;
		step *= -1;
	}
}

// @brief ������������� ���������� � ���������� �������
void Arcoder::mappedDecode(uint8_t* in, uint8_t* out, CoderMap map, int &size_out)
{
	data_in = in;
	data_out = out;
	int symbol;
	prev_symbol = 0;
	start_model();
	start_decoding();

	int hLeftIndex = map.m_hSize[0];
	int vTopIndex = map.m_vSize[0];
	int hRightIndex = map.m_hSize[1];
	int vBotIndex = map.m_vSize[1];
	int imgWidth = map.m_hSize[map.steps];

	// read minimal left top subband
	SubbandRect rect(0, hLeftIndex, 0, vTopIndex);
	decodeSubband(rect);

	// read other data
	for (int k = 0; k < map.steps; ++k)
	{
		// right top
		SubbandRect rightTop(hLeftIndex, hRightIndex, 0, vTopIndex);
		decodeSubband(rightTop);


		//left bot
		SubbandRect leftBot(0, hLeftIndex, vTopIndex, vBotIndex);
		decodeSubband(leftBot);

		//right bot
		SubbandRect rightBot(hLeftIndex, hRightIndex, vTopIndex, vBotIndex);
		decodeSubband(rightBot);

		hLeftIndex = map.m_hSize[k + 1];
		vTopIndex = map.m_vSize[k + 1];
		hRightIndex = map.m_hSize[(k + 2) % 5];
		vBotIndex = map.m_vSize[(k + 2) % 5];
	}

	sizeIn = 0;
	size_out = sizeOut;
	sizeOut = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
/////////////////
/////////////////	one model encoding -- decoding
/////////////////
/////////////////////////////////////////////////////////////////////////////////////////

// @brief ����������� ���������� � ���������� �������
void Arcoder::oneModelMappedEncode(uint8_t* in, uint8_t* out, CoderMap i_map, int &size_out)
{
	// let's prev_symbol be always 1
	m_isOneModel = true;
	mappedEncode(in, out, i_map, size_out);
	m_isOneModel = false;
}

// @brief ������������� ���������� � ���������� �������
void Arcoder::oneModelMappedDecode(uint8_t* in,uint8_t* out, CoderMap i_map, int &size_out)
{
	// let's prev_symbol be always 1
	m_isOneModel = true;
	mappedDecode(in, out, i_map, size_out);
	m_isOneModel = false;
}