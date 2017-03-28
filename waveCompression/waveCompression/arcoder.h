#pragma once

// 2 Simplest & slowest version of arithmetic codec (almost demo version)
#include <stdio.h>
#include <process.h>
#include <vector>

#pragma warning(disable: 4996)

//для избежания переполнения:	MAX_FREQUENCY * (TOP_VALUE+1) < ULONG_MAX 
//число MAX_FREQUENCY должно быть не менее, чем в 4 раза меньше TOP_VALUE 
//число символов NO_OF_CHARS должно быть много меньше MAX_FREQUENCY 
#define BITS_IN_REGISTER			17	
#define TOP_VALUE					(((unsigned long)1<<BITS_IN_REGISTER)-1)	// 1111...1
#define FIRST_QTR					((TOP_VALUE>>2) +1)							// 0100...0
#define HALF						(2*FIRST_QTR)								// 1000...0
#define THIRD_QTR					(3*FIRST_QTR)								// 1100...0
#define MAX_FREQUENCY				((unsigned)1<<15)
#define NO_OF_CHARS					256
#define EOF_SYMBOL					NO_OF_CHARS			// char-коды: 0..NO_OF_CHARS-1 
#define NO_OF_SYMBOLS				(NO_OF_CHARS+1)		// + EOF_SYMBOL

unsigned long						low, high, value;
uint8_t								buffer, bits_to_go;
int									garbage_bits, bits_to_follow;
unsigned int						cum_freq[NO_OF_CHARS][NO_OF_SYMBOLS + 1];	//интервалы частот символов
																				// относительная частота появления символа s (оценка вероятности его появления)
																				// определяется как p(s)=(cum_freq[s+1]-cum_freq[s])/cum_freq[NO_OF_SYMBOLS]
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

// @brief инициализация массива частот
void start_model(void)
{
	for (int k = 0; k < NO_OF_CHARS; ++k)
	{
		// исходно все символы в сообщении считаем равновероятными 
		for (int i = 0; i <= NO_OF_SYMBOLS; i++)	cum_freq[k][i] = i;
	}
}

// @brief обновление массива частот
// @param symbol - in, поступивший символ
inline void update_model(int symbol, int prev_symbol)
{
	if (cum_freq[prev_symbol][NO_OF_SYMBOLS] == MAX_FREQUENCY)
	{	//	масштабируем частотные интервалы, уменьшая их в 2 раза
		int cum = 0;
		for (int i = 0; i < NO_OF_SYMBOLS; i++)
		{
			int fr = (cum_freq[prev_symbol][i + 1] - cum_freq[prev_symbol][i] + 1) >> 1;
			cum_freq[prev_symbol][i] = cum;
			cum += fr;
		}
		cum_freq[prev_symbol][NO_OF_SYMBOLS] = cum;
	}
	// обновление интервалов частот
	for (int i = symbol + 1; i <= NO_OF_SYMBOLS; i++) cum_freq[prev_symbol][i]++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////
///// encode
/////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// @brief инициализация глобальных переменных
void start_encoding(void)
{
	bits_to_go = 8;				// свободно бит в битовом буфере вывода
	bits_to_follow = 0;				// число бит, вывод которых отложен
	low = 0;				// нижняя граница интервала
	high = TOP_VALUE;		// верхняя граница интервала
}

// @brief вывод одного бита в сжатый файл
inline void output_bit(int bit)
{
	buffer = (buffer >> 1) + (bit << 7);	// в битовый буфер (один байт)
	bits_to_go--;
	if (bits_to_go == 0)				// битовый буфер заполнен, сброс буфера
	{
		*data_out = buffer;
		data_out++;
		sizeOut++;

		bits_to_go = 8;
	}
}

// @brief вывод одного очередного бита и тех, которые были отложены
inline void output_bit_plus_follow(int bit)
{
	output_bit(bit);
	while (bits_to_follow>0)
	{
		output_bit(!bit);
		bits_to_follow--;
	}
}

// @brief завершение кодирования
void done_encoding(void)
{
	bits_to_follow++;
	if (low < FIRST_QTR) output_bit_plus_follow(0);
	else output_bit_plus_follow(1);
	*data_out = buffer >> bits_to_go;				// записать незаполненный буфер
	sizeOut++;
}

// @brief кодирование символа
// @param symbol - in, поступивший символ
void encode_symbol(int symbol, int prev_symbol = 0)
{
	// пересчет границ интервала
	unsigned long range;
	range = high - low + 1;
	high = low + range*cum_freq[prev_symbol][symbol + 1] / cum_freq[prev_symbol][NO_OF_SYMBOLS] - 1;
	low = low + range*cum_freq[prev_symbol][symbol] / cum_freq[prev_symbol][NO_OF_SYMBOLS];
	// далее при необходимости - вывод бита или меры от зацикливания
	for (;;)
	{			// Замечание: всегда low<high
		if (high<HALF) // Старшие биты low и high - нулевые (оба)
			output_bit_plus_follow(0); //вывод совпадающего старшего бита
		else if (low >= HALF) // Старшие биты low и high - единичные
		{
			output_bit_plus_follow(1);	//вывод старшего бита
			low -= HALF;					//сброс старшего бита в 0
			high -= HALF;					//сброс старшего бита в 0
		}
		else if (low >= FIRST_QTR && high < THIRD_QTR)
		{		/* возможно зацикливание, т.к.
				HALF<=high<THIRD_QTR,	i.e. high=10...
				FIRST_QTR<=low<HALF,	i.e. low =01...
				выбрасываем второй по старшинству бит	*/
			high -= FIRST_QTR;		// high	=01...
			low -= FIRST_QTR;			// low	=00...
			bits_to_follow++;		//откладываем вывод (еще) одного бита
									// младший бит будет втянут далее
		}
		else break;		// втягивать новый бит рано 
						//	старший бит в low и high нулевой, втягиваем новый бит в младший разряд 
		low <<= 1;			// втягиваем 0
		(high <<= 1)++;		// втягиваем 1
	}
}

// @brief кодирование информации
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
	for (int j = rect.top; j < rect.bot; ++j)
	{
		for (int i = rect.left; i < rect.right; ++i)
		{
			int index = j*imgWidth + i;
			uint8_t symbol = data_in[index];

			encode_symbol(symbol, prev_symbol);
			update_model(symbol, prev_symbol);
			prev_symbol = symbol;


		}
		/*if (j < vMidIndex)
		{
		for (int i = hMidIndex-1; i>=0; --i)
		{
		int index = j*imgWidth + i;

		symbol = data_in[index];
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
		j++;
		}*/
	}
}

// @brief кодирование информации в нелинейном порядке
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
		encodeSubband(rect);


		//left bot
		SubbandRect leftBot(0, hLeftIndex, vTopIndex, vBotIndex);
		encodeSubband(rect);

		//right bot
		SubbandRect rightBot(hLeftIndex, hRightIndex, vTopIndex, vBotIndex);
		encodeSubband(rect);

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

// @brief ввод 1 бита из сжатого файла
inline int input_bit(void)
{
	if (bits_to_go == 0)
	{
		buffer = *data_in;	// заполняем буфер битового ввода
		uint8_t temp = data_in[7212 - 1];
		data_in++;
		sizeIn++;
		temp = data_in[7212 - 1 - sizeIn];
		if (buffer == EOF)	// входной поток сжатых данных исчерпан !!!
		{
			// Причина попытки дальнейшего чтения: следующим 
			// декодируемым символом должен быть EOF_SYMBOL,
			// но декодер об этом пока не знает и может готовиться 
			// к дальнейшему декодированию, втягивая новые биты 
			// (см. цикл for(;;) в процедуре decode_symbol). Эти 
			// биты - "мусор", реально не несут никакой 
			// информации и их можно выдать любыми
			garbage_bits++;
			if (garbage_bits>BITS_IN_REGISTER - 2)
			{	// больше максимально возможного числа мусорных битов
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

// @brief инициализация глобальных переменных
void start_decoding(void)
{
	bits_to_go = 0;				// свободно бит в битовом буфере ввода
	garbage_bits = 0;				// контроль числа "мусорных" бит в конце сжатого файла
	low = 0;				// нижняя граница интервала
	high = TOP_VALUE;		// верхняя граница интервала
	value = 0;				// "ЧИСЛО"
	for (int i = 0; i<BITS_IN_REGISTER; i++) value = (value << 1) + input_bit();
}

// @brief декодирование символа
int decode_symbol(int prev_symbol = 0)
{
	unsigned long range, cum;
	int symbol;
	range = high - low + 1;
	// число cum - это число value, пересчитанное из интервала
	// low..high в интервал 0..CUM_FREQUENCY[NO_OF_SYMBOLS]
	cum = ((value - low + 1)*cum_freq[prev_symbol][NO_OF_SYMBOLS] - 1) / range;
	// поиск интервала, соответствующего числу cum
	for (symbol = 0; cum_freq[prev_symbol][symbol + 1] <= cum; symbol++);
	// пересчет границ
	high = low + range*cum_freq[prev_symbol][symbol + 1] / cum_freq[prev_symbol][NO_OF_SYMBOLS] - 1;
	low = low + range*cum_freq[prev_symbol][symbol] / cum_freq[prev_symbol][NO_OF_SYMBOLS];
	for (;;)
	{		// подготовка к декодированию следующих символов
		if (high<HALF) {/* Старшие биты low и high - нулевые */ }
		else if (low >= HALF)
		{		// Старшие биты low и high - единичные, сбрасываем
			value -= HALF;
			low -= HALF;
			high -= HALF;
		}
		else if (low >= FIRST_QTR && high<THIRD_QTR)
		{		// Поступаем так же, как при кодировании
			value -= FIRST_QTR;
			low -= FIRST_QTR;
			high -= FIRST_QTR;
		}
		else break;	// втягивать новый бит рано 
		low <<= 1;							// втягиваем новый бит 0
		(high <<= 1)++;						// втягиваем новый бит 1
		value = (value << 1) + input_bit();		// втягиваем новый бит информации
	}
	return symbol;
}

// @brief декодирование информации
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
int decodeAndUpdate(int index, int prevSymbol)
{
	int symbol = decode_symbol(prevSymbol);
	update_model(symbol, prevSymbol);
	data_out[index] = symbol;
	sizeOut++;

	return symbol;
}
// @brief декодирование информации в нелинейном порядке
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

	int hMidIndex = map.m_hSize[0];
	int vMidIndex = map.m_vSize[0];
	int hHighIndex = map.m_hSize[1];
	int vHighIndex = map.m_vSize[1];
	int imgWidth = map.m_hSize[map.steps];

	//while ((symbol = decode_symbol(prev_symbol)) != EOF_SYMBOL)

	// read minimal left top subband
	for (int j = 0; j < vMidIndex; ++j)
	{
		for (int i = 0; i < hMidIndex; ++i)
		{
			int index = j*imgWidth + i;

			int symbol = decode_symbol(prev_symbol);
			update_model(symbol, prev_symbol);
			data_out[index] = symbol;
			sizeOut++;

			prev_symbol = symbol;
		}
		/*if (j < vMidIndex)
		{
		for (int i = hMidIndex - 1; i >= 0; --i)
		{
		int index = j*imgWidth + i;

		symbol = decode_symbol(prev_symbol);
		update_model(symbol, prev_symbol);
		data_out[index] = symbol;
		sizeOut++;

		prev_symbol = symbol;
		}
		j++;
		}*/
	}

	// read other data
	for (int k = 0; k < map.steps; ++k)
	{
		// right top
		for (int j = 0; j < vMidIndex; ++j)
		{
			for (int i = hMidIndex; i < hHighIndex; ++i)
			{
				int index = j*imgWidth + i;

				symbol = decode_symbol(prev_symbol);
				update_model(symbol, prev_symbol);
				data_out[index] = symbol;
				sizeOut++;

				prev_symbol = symbol;
			}
			/*if (j < vMidIndex)
			{
			for (int i = hHighIndex - 1; i >= hMidIndex; --i)
			{
			int index = j*imgWidth + i;

			symbol = decode_symbol(prev_symbol);
			update_model(symbol, prev_symbol);
			data_out[index] = symbol;
			sizeOut++;

			prev_symbol = symbol;
			}
			j++;
			}*/
		}

		//left bot
		for (int j = vMidIndex; j < vHighIndex; ++j)
		{
			for (int i = 0; i < hMidIndex; ++i)
			{
				int index = j*imgWidth + i;

				symbol = decode_symbol(prev_symbol);
				update_model(symbol, prev_symbol);
				data_out[index] = symbol;
				sizeOut++;

				prev_symbol = symbol;
			}
			/*if (j < vHighIndex)
			{
			for (int i = hMidIndex - 1; i >= 0; --i)
			{
			int index = j*imgWidth + i;

			symbol = decode_symbol(prev_symbol);
			update_model(symbol, prev_symbol);
			data_out[index] = symbol;
			sizeOut++;

			prev_symbol = symbol;
			}
			j++;
			}*/
		}

		//right bot
		for (int j = vMidIndex; j < vHighIndex; ++j)
		{
			for (int i = hMidIndex; i < hHighIndex; ++i)
			{
				int index = j*imgWidth + i;

				symbol = decode_symbol(prev_symbol);
				update_model(symbol, prev_symbol);
				data_out[index] = symbol;
				sizeOut++;

				prev_symbol = symbol;
			}
			/*if (j < vHighIndex)
			{
			for (int i = hHighIndex - 1; i >= hMidIndex; --i)
			{
			int index = j*imgWidth + i;

			symbol = decode_symbol(prev_symbol);
			update_model(symbol, prev_symbol);
			data_out[index] = symbol;
			sizeOut++;

			prev_symbol = symbol;
			}
			j++;
			}*/
		}

		hMidIndex = map.m_hSize[k + 1];
		vMidIndex = map.m_vSize[k + 1];
		hHighIndex = map.m_hSize[(k + 2) % 5];
		vHighIndex = map.m_vSize[(k + 2) % 5];
	}

	size_out = sizeOut;
}
