#pragma once

// 2 Simplest & slowest version of arithmetic codec (almost demo version)
#include <stdio.h>
#include <process.h>
#include <vector>

#pragma warning(disable: 4996)

//��� ��������� ������������:	MAX_FREQUENCY * (TOP_VALUE+1) < ULONG_MAX 
//����� MAX_FREQUENCY ������ ���� �� �����, ��� � 4 ���� ������ TOP_VALUE 
//����� �������� NO_OF_CHARS ������ ���� ����� ������ MAX_FREQUENCY 
#define BITS_IN_REGISTER			17	
#define TOP_VALUE					(((unsigned long)1<<BITS_IN_REGISTER)-1)	// 1111...1
#define FIRST_QTR					((TOP_VALUE>>2) +1)							// 0100...0
#define HALF						(2*FIRST_QTR)								// 1000...0
#define THIRD_QTR					(3*FIRST_QTR)								// 1100...0
#define MAX_FREQUENCY				((unsigned)1<<15)
#define NO_OF_CHARS					256
#define EOF_SYMBOL					NO_OF_CHARS			// char-����: 0..NO_OF_CHARS-1 
#define NO_OF_SYMBOLS				(NO_OF_CHARS+1)		// + EOF_SYMBOL

unsigned long						low, high, value;
uint8_t								buffer, bits_to_go;
int									garbage_bits, bits_to_follow;
unsigned int						cum_freq[NO_OF_CHARS][NO_OF_SYMBOLS + 1];	//��������� ������ ��������
																				// ������������� ������� ��������� ������� s (������ ����������� ��� ���������)
																				// ������������ ��� p(s)=(cum_freq[s+1]-cum_freq[s])/cum_freq[NO_OF_SYMBOLS]
uint8_t *data_in, *data_out;
int sizeOut, sizeIn = 0;
uint8_t prev_symbol;
int imgWidth;

class CoderMap
{
public:
	CoderMap(int hSize, int vSize, int i_steps)
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

	~CoderMap()
	{};

public:

	std::vector<int> m_hSize;
	std::vector<int> m_vSize;
	int steps;
};

class SubbandRect
{
public:
	SubbandRect(int leftSide, int rightSide, int topSide, int botSide)
	{
		left = leftSide;
		right = rightSide;
		top = topSide;
		bot = botSide;
	}

	int left;
	int right;
	int top;
	int bot;
};

// @brief ������������� ������� ������
void start_model(void)
{
	for (int k = 0; k < NO_OF_CHARS; ++k)
	{
		// ������� ��� ������� � ��������� ������� ��������������� 
		for (int i = 0; i <= NO_OF_SYMBOLS; i++)	cum_freq[k][i] = i;
	}
}

// @brief ���������� ������� ������
// @param symbol - in, ����������� ������
inline void update_model(int symbol, int prev_symbol)
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
void start_encoding(void)
{
	bits_to_go = 8;				// �������� ��� � ������� ������ ������
	bits_to_follow = 0;				// ����� ���, ����� ������� �������
	low = 0;				// ������ ������� ���������
	high = TOP_VALUE;		// ������� ������� ���������
}

// @brief ����� ������ ���� � ������ ����
inline void output_bit(int bit)
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
inline void output_bit_plus_follow(int bit)
{
	output_bit(bit);
	while (bits_to_follow>0)
	{
		output_bit(!bit);
		bits_to_follow--;
	}
}

// @brief ���������� �����������
void done_encoding(void)
{
	bits_to_follow++;
	if (low < FIRST_QTR) output_bit_plus_follow(0);
	else output_bit_plus_follow(1);
	*data_out = buffer >> bits_to_go;				// �������� ������������� �����
	sizeOut++;
}

// @brief ����������� �������
// @param symbol - in, ����������� ������
void encode_symbol(int symbol, int prev_symbol = 0)
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
void encode(uint8_t* in, uint8_t* out, int size_in, int &size_out)
{
	data_in = in;
	data_out = out;

	uint8_t symbol;
	int prev_symbol = -1;
	start_model();
	start_encoding();

	int size = 0;

	// read data
	for (int i = 0; i < size_in; ++i)
	{
		symbol = data_in[i];
		if (prev_symbol != -1)
		{
			encode_symbol(symbol, prev_symbol);
			update_model(symbol, prev_symbol);
		}
		else
		{
			encode_symbol(symbol);
		}
		prev_symbol = symbol;
	}

	encode_symbol(EOF_SYMBOL, prev_symbol);
	done_encoding();

	size_out = sizeOut;
	sizeOut = 0;
}

//
void encodeSubband(SubbandRect rect)
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

			encode_symbol(symbol, prev_symbol);
			update_model(symbol, prev_symbol);
			prev_symbol = symbol;
		}
		
		// change direction
		int temp = horizontalFrom;
		horizontalFrom = horizontalTo;
		horizontalTo = temp;
		step *= -1;
	}
}

// @brief ����������� ���������� � ���������� �������
void mappedEncode(uint8_t* in, uint8_t* out, int size_in, int &size_out, CoderMap map)
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

	encode_symbol(EOF_SYMBOL, prev_symbol);
	done_encoding();

	size_out = sizeOut;
	sizeOut = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////
///// decode
/////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// @brief ���� 1 ���� �� ������� �����
inline int input_bit(void)
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
void start_decoding(void)
{
	bits_to_go = 0;				// �������� ��� � ������� ������ �����
	garbage_bits = 0;				// �������� ����� "��������" ��� � ����� ������� �����
	low = 0;				// ������ ������� ���������
	high = TOP_VALUE;		// ������� ������� ���������
	value = 0;				// "�����"
	for (int i = 0; i<BITS_IN_REGISTER; i++) value = (value << 1) + input_bit();
}

// @brief ������������� �������
int decode_symbol(int prev_symbol = 0)
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
void decode(uint8_t* in, uint8_t* out, int size_in, int &size_out)
{
	sizeIn = 0;
	data_out = out;
	data_in = in;

	int symbol;
	int prev_symbol = 0;
	start_model();
	start_decoding();

	while ((symbol = decode_symbol(prev_symbol)) != EOF_SYMBOL)
	{
		if (prev_symbol != -1)
		{
			update_model(symbol, prev_symbol);
			*data_out = symbol;
			data_out++;
			sizeOut++;
			if (sizeOut == size_out - 1)
			{
				int temp = 1;
			}
		}
		else
		{
			*data_out = symbol;
			data_out++;
			sizeOut++;
		}
		prev_symbol = symbol;
	}
	size_out = sizeOut;
}

// 
void decodeSubband(SubbandRect rect)
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

			int symbol = decode_symbol(prev_symbol);
			update_model(symbol, prev_symbol);
			data_out[index] = symbol;
			sizeOut++;

			prev_symbol = symbol;
		}

		// change direction
		int temp = horizontalFrom;
		horizontalFrom = horizontalTo;
		horizontalTo = temp;
		step *= -1;
	}
}
// @brief ������������� ���������� � ���������� �������
void mappedDecode(uint8_t* in, uint8_t* out, int size_in, int &size_out, CoderMap map)
{
	sizeOut = 0;
	sizeIn = 0;
	data_out = out;
	data_in = in;

	int symbol;
	prev_symbol = 0;
	start_model();
	start_decoding();

	int hLeftIndex = map.m_hSize[0];
	int vTopIndex = map.m_vSize[0];
	int hRightIndex = map.m_hSize[1];
	int vBotIndex = map.m_vSize[1];
	int imgWidth = map.m_hSize[map.steps];

	//while ((symbol = decode_symbol(prev_symbol)) != EOF_SYMBOL)

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

	size_out = sizeOut;
}
