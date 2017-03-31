#pragma once

// 2 Simplest & slowest version of arithmetic codec (almost demo version)
#include <vector>


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

class SubbandMap
{
public:
	SubbandMap(int hSize, int vSize, int i_steps)
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

	~SubbandMap()
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

class Arcoder
{
public:

	Arcoder();

	// @brief ������������� ������� ������
	void start_model(void);

	// @brief ���������� ������� ������
	// @param symbol - in, ����������� ������
	inline void update_model(int symbol);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// encode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// @brief ������������� ���������� ����������
	void start_encoding(void);

	// @brief ����� ������ ���� � ������ ����
	inline void output_bit(int bit);

	// @brief ����� ������ ���������� ���� � ���, ������� ���� ��������
	inline void output_bit_plus_follow(int bit);

	// @brief ���������� �����������
	void done_encoding(void);

	// @brief ����������� �������
	// @param symbol - in, ����������� ������
	void encode_symbol(int symbol);

	// @brief ����������� ����������
	void encode(uint8_t* in, uint8_t* out, int size_in, int &size_out);

	//
	void encodeSubband(SubbandRect rect);

	// @brief ����������� ���������� � ���������� �������
	void mappedEncode(uint8_t* in, uint8_t* out, SubbandMap map, int &size_out);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// decode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// @brief ���� 1 ���� �� ������� �����
	inline int input_bit(void);

	// @brief ������������� ���������� ����������
	void start_decoding(void);

	// @brief ������������� �������
	int decode_symbol();

	// @brief ������������� ����������
	void decode(uint8_t* in, uint8_t* out, int size_in, int &size_out);

	// 
	void decodeSubband(SubbandRect rect);

	// @brief ������������� ���������� � ���������� �������
	void mappedDecode(uint8_t* in, uint8_t* out, SubbandMap map, int &size_out);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// one-model encoding/decoding
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// @brief ����������� ���������� � ���������� �������
	void oneModelMappedEncode(uint8_t* in, uint8_t* out, SubbandMap i_map, int &size_out);

	// @brief ������������� ���������� � ���������� �������
	void oneModelMappedDecode(uint8_t* in, uint8_t* out, SubbandMap i_map, int &size_out);

protected:
	bool m_isOneModel = false;

	unsigned long						low, high, value;
	uint8_t								buffer, bits_to_go;
	int									garbage_bits, bits_to_follow;
	
	uint8_t *data_in, *data_out;
	int sizeOut, sizeIn = 0;
	uint8_t prev_symbol;
	int imgWidth;

private:
	unsigned int						cum_freq[NO_OF_CHARS][NO_OF_SYMBOLS + 1];	//��������� ������ ��������
																					// ������������� ������� ��������� ������� s (������ ����������� ��� ���������)
																					// ������������ ��� p(s)=(cum_freq[s+1]-cum_freq[s])/cum_freq[NO_OF_SYMBOLS]
};
