#include <stdio.h>
#include <process.h>
#include "arcoder.h"

#include <vector>

#pragma warning(disable: 4996)

/////////////////////////////////////////////////////////////////////////////////
Model::Model(bool isEOFneeded /*= true*/)
{
	m_numOfChars = NO_OF_CHARS;
	m_numOfSymbols = m_numOfChars + (isEOFneeded ? 1 : 0);
	cum_freq = new uint[m_numOfSymbols + 1];
}

/////////////////////////////////////////////////////////////////////////////////
Model::Model(int i_numOfChars, bool isEOFneeded /*= false*/)
{
	m_numOfChars = i_numOfChars;
	m_numOfSymbols = m_numOfChars + (isEOFneeded ? 1 : 0);
	cum_freq = new uint[m_numOfSymbols + 1];
}

/////////////////////////////////////////////////////////////////////////////////
void Model::StartModel()
{
	// ������� ��� ������� � ��������� ������� ��������������� 
	for (int i = 0; i <= m_numOfSymbols; i++)
	{
		cum_freq[i] = i;
	}
}

/////////////////////////////////////////////////////////////////////////////////
inline void Model::UpdateModel(int i_symbol)
{
	if (cum_freq[m_numOfSymbols] == MAX_FREQUENCY)
	{	//	������������ ��������� ���������, �������� �� � 2 ����
		int cum = 0;
		for (int i = 0; i < m_numOfSymbols; i++)
		{
			int fr = (cum_freq[i + 1] - cum_freq[i] + 1) >> 1;
			cum_freq[i] = cum;
			cum += fr;
		}
		cum_freq[m_numOfSymbols] = cum;
	}
	// ���������� ���������� ������
	for (int i = i_symbol + 1; i <= m_numOfSymbols; i++)
	{
		cum_freq[i]++;
	}
}

/////////////////////////////////////////////////////////////////////////////////
unsigned int Model::GetFreq(int i_symbol)
{
	return cum_freq[i_symbol];
}

/////////////////////////////////////////////////////////////////////////////////
unsigned int Model::GetLastFreq()
{
	return cum_freq[m_numOfSymbols];
}

/////////////////////////////////////////////////////////////////////////////////
int Model::GetEof()
{
	return m_numOfChars;
}

/////////////////////////////////////////////////////////////////////////////////
void Converter::Initialize(int i_numOfChars /*= 256*/,
		  int i_startValue /*= -128*/)
{
	m_startValue = i_startValue;
}

/////////////////////////////////////////////////////////////////////////////////
uint Converter::ConvertToUnsigned(int i_symbol)
{
	return static_cast<uint>(i_symbol - m_startValue);
}

/////////////////////////////////////////////////////////////////////////////////
int Converter::ConvertToSigned(int i_symbol)
{
	return static_cast<int>(i_symbol + m_startValue);
}

/////////////////////////////////////////////////////////////////////////////////
Arcoder::Arcoder():
	m_numOfModelsNeeded(0)
{
}

/////////////////////////////////////////////////////////////////////////////////
Arcoder::Arcoder(qMinCap i_qMinCap, int i_memoryLen)
{
	m_numOfModelsNeeded = (i_memoryLen == 0) ? 1 :i_memoryLen *i_qMinCap.arrCapacity;

	for (int i = 0; i < m_numOfModelsNeeded; ++i)
	{
		m_model.emplace_back(i_qMinCap.arrCapacity, true);
	}

	conv.Initialize(i_qMinCap.arrCapacity, i_qMinCap.minValue);
}

/////////////////////////////////////////////////////////////////////////////////
// @brief ������������� ������� ������
void Arcoder::start_model(void)
{
	for (int k = 0; k < m_numOfModelsNeeded; ++k)
	{
		m_model[k].StartModel();
	}
	m_currentModel = 0;
}

/////////////////////////////////////////////////////////////////////////////////
inline void Arcoder::update_model(int symbol)
{
	m_model[m_currentModel].UpdateModel(symbol);

	// update previous symbol

	if (m_numOfModelsNeeded != 1 && m_subbandType == 0)
	{
		m_currentModel = symbol;
	}
}

/////////////////////////////////////////////////////////////////////////////////
void Arcoder::exportModelInformation(FILE* i_file)
{
	unsigned int* output = new unsigned int[m_model.size() * (m_model[0].GetFreqSize())];
	int index = 0;

	output[index] = m_model.size();
	index++;
	for (int i = 0; i < m_model.size(); ++i)
	{
		output[index] = m_model[i].GetFreqSize();
		index++;

		int freqSize = m_model[i].GetFreqSize();
		unsigned int* tempFreq = m_model[i].GetFreqData();
		for (int j = 0; j < freqSize; ++j)
		{
			output[index] = tempFreq[j];
			++index;
		}
	}

	fwrite(output, sizeof(*output), index, i_file);
	delete[] output;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////
///// encode
/////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// @brief ������������� ���������� ����������
void Arcoder::start_encoding(void)
{
	bits_to_go = BUFFER_SIZE;				// �������� ��� � ������� ������ ������
	bits_to_follow = 0;				// ����� ���, ����� ������� �������
	low = 0;				// ������ ������� ���������
	high = TOP_VALUE;		// ������� ������� ���������

	m_currentModel = 0;
}

/////////////////////////////////////////////////////////////////////////////////
inline void Arcoder::output_bit(int bit)
{
	buffer = (buffer >> 1) + (bit << 7);	// � ������� ����� (���� ����)
	bits_to_go--;
	if (bits_to_go == 0)				// ������� ����� ��������, ����� ������
	{
		*m_encodedData = buffer;
		m_encodedData++;
		sizeOut++;

		bits_to_go = BUFFER_SIZE;
	}
}

/////////////////////////////////////////////////////////////////////////////////
inline void Arcoder::output_bit_plus_follow(int bit)
{
	output_bit(bit);
	while (bits_to_follow>0)
	{
		output_bit(!bit);
		bits_to_follow--;
	}
}

/////////////////////////////////////////////////////////////////////////////////
void Arcoder::done_encoding(void)
{
	bits_to_follow++;
	if (low < FIRST_QTR) output_bit_plus_follow(0);
	else output_bit_plus_follow(1);
	*m_encodedData = buffer >> bits_to_go;				// �������� ������������� �����
	sizeOut++;
}

/////////////////////////////////////////////////////////////////////////////////
void Arcoder::encode_symbol(int symbol)
{
	// �������� ������ ���������
	unsigned long range;
	range = high - low + 1;
	high = low + range * m_model[m_currentModel].GetFreq(symbol + 1) / m_model[m_currentModel].GetLastFreq() - 1;
	low = low + range * m_model[m_currentModel].GetFreq(symbol) / m_model[m_currentModel].GetLastFreq();
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

/////////////////////////////////////////////////////////////////////////////////
void Arcoder::encode(int* in, int8_t* out, int size_in, int &size_out)
{
	m_decodedData = in;
	m_encodedData = out;

	start_model();
	start_encoding();

	int size = 0;

	// read data
	for (int i = 0; i < size_in; ++i)
	{
		uint uSymbol= conv.ConvertToUnsigned(m_decodedData[i]);
		encode_symbol(uSymbol);
		update_model(uSymbol);
	}

	encode_symbol(m_model[m_currentModel].GetEof());
	done_encoding();

	sizeIn = 0;
	size_out = sizeOut;
	sizeOut = 0;
}

/////////////////////////////////////////////////////////////////////////////////
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

			int uSymbol = conv.ConvertToUnsigned(m_decodedData[index]);

			encode_symbol(uSymbol);
			update_model(uSymbol);
		}

		// change direction
		int temp = horizontalFrom;
		horizontalFrom = horizontalTo;
		horizontalTo = temp;
		step *= -1;
	}
}

/////////////////////////////////////////////////////////////////////////////////
void Arcoder::mappedEncode(int* in, int8_t* out, SubbandMap map, int &size_out)
{
	m_decodedData = in;
	m_encodedData = out;

	int hLeftIndex = map.m_hSize[0];
	int vTopIndex = map.m_vSize[0];
	int hRightIndex = map.m_hSize[1];
	int vBotIndex = map.m_vSize[1];
	imgWidth = map.m_hSize[map.steps];

	start_model();
	start_encoding();

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
	done_encoding();

	size_out = sizeOut;
	sizeIn = 0;
	sizeOut = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////
///// decode
/////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline int Arcoder::input_bit(void)
{
	if (bits_to_go == 0)
	{
		buffer = *m_encodedData;	// ��������� ����� �������� �����
		m_encodedData++;
		sizeIn++;
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
		else bits_to_go = BUFFER_SIZE;
	}
	int t = buffer & 1;
	buffer >>= 1;
	bits_to_go--;
	return t;
}

/////////////////////////////////////////////////////////////////////////////////
void Arcoder::start_decoding(void)
{
	bits_to_go = 0;				// �������� ��� � ������� ������ �����
	garbage_bits = 0;				// �������� ����� "��������" ��� � ����� ������� �����
	low = 0;				// ������ ������� ���������
	high = TOP_VALUE;		// ������� ������� ���������
	value = 0;				// "�����"
	for (int i = 0; i<BITS_IN_REGISTER; i++) value = (value << 1) + input_bit();

	m_currentModel = 0;
}

/////////////////////////////////////////////////////////////////////////////////
int Arcoder::decode_symbol()
{
	unsigned long range, cum;
	int symbol;
	range = high - low + 1;

	// ����� cum - ��� ����� value, ������������� �� ���������
	// low..high � �������� 0..CUM_FREQUENCY[NO_OF_SYMBOLS]
	cum = ((value - low + 1)*m_model[m_currentModel].GetLastFreq() - 1) / range;

	// ����� ���������, ���������������� ����� cum
	symbol = m_model[m_currentModel].findInterval(cum);

	// �������� ������
	high = low + range*m_model[m_currentModel].GetFreq(symbol + 1) / m_model[m_currentModel].GetLastFreq() - 1;
	low = low + range*m_model[m_currentModel].GetFreq(symbol) / m_model[m_currentModel].GetLastFreq();

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

/////////////////////////////////////////////////////////////////////////////////
void Arcoder::decode(int8_t* in, int* out, int size_in, int &size_out)
{
	m_encodedData = in;
	m_decodedData = out;

	int uSymbol;
	start_model();
	start_decoding();
	int eofSymbol = m_model[m_currentModel].GetEof();

	while ((uSymbol = decode_symbol()) != eofSymbol)
	{
		update_model(uSymbol);

		int symbol = conv.ConvertToSigned(uSymbol);
		*m_decodedData = symbol;
		m_decodedData++;
		sizeOut++;
	}

	sizeIn = 0;
	size_out = sizeOut;
	sizeOut = 0;
}

/////////////////////////////////////////////////////////////////////////////////
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
			int uSymbol = decode_symbol();
			update_model(uSymbol);

			int symbol = conv.ConvertToSigned(uSymbol);
			m_decodedData[index] = symbol;
			sizeOut++;
		}

		// change direction
		int temp = horizontalFrom;
		horizontalFrom = horizontalTo;
		horizontalTo = temp;
		step *= -1;
	}
}

/////////////////////////////////////////////////////////////////////////////////
void Arcoder::mappedDecode(int8_t* in, int* out, SubbandMap map, int &size_out)
{
	m_encodedData = in;
	m_decodedData = out;
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
