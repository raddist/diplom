#include <stdio.h>
#include <process.h>
#include "arcoder.h"

#include <vector>

#pragma warning(disable: 4996)

Arcoder::Arcoder()
{
}

Arcoder::Arcoder(int i_memoryLen)
{
	if (i_memoryLen != 0)
	{
		m_isOneModel = false;
	}
}

// @brief инициализация массива частот
void Arcoder::start_model(void)
{
	for (int k = 0; k < NO_OF_CHARS; ++k)
	{
		// исходно все символы в сообщении считаем равновероятными 
		for (int i = 0; i <= NO_OF_SYMBOLS; i++)	cum_freq[k][i] = i;
	}
	m_currentModel = 0;
}

// @brief обновление массива частот
// @param symbol - in, поступивший символ
inline void Arcoder::update_model(int symbol)
{
	if (cum_freq[m_currentModel][NO_OF_SYMBOLS] == MAX_FREQUENCY)
	{	//	масштабируем частотные интервалы, уменьшая их в 2 раза
		int cum = 0;
		for (int i = 0; i < NO_OF_SYMBOLS; i++)
		{
			int fr = (cum_freq[m_currentModel][i + 1] - cum_freq[m_currentModel][i] + 1) >> 1;
			cum_freq[m_currentModel][i] = cum;
			cum += fr;
		}
		cum_freq[m_currentModel][NO_OF_SYMBOLS] = cum;
	}
	// обновление интервалов частот
	for (int i = NEGATIVE_SHIFT + symbol + 1; i <= NO_OF_SYMBOLS; i++) cum_freq[m_currentModel][i]++;

	// update previous symbol

	if (!m_isOneModel)
	{
		m_currentModel = NEGATIVE_SHIFT + symbol;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////
///// encode
/////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// @brief инициализация глобальных переменных
void Arcoder::start_encoding(void)
{
	bits_to_go = 8;				// свободно бит в битовом буфере вывода
	bits_to_follow = 0;				// число бит, вывод которых отложен
	low = 0;				// нижняя граница интервала
	high = TOP_VALUE;		// верхняя граница интервала

	m_currentModel = 0;
}

// @brief вывод одного бита в сжатый файл
inline void Arcoder::output_bit(int bit)
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
inline void Arcoder::output_bit_plus_follow(int bit)
{
	output_bit(bit);
	while (bits_to_follow>0)
	{
		output_bit(!bit);
		bits_to_follow--;
	}
}

// @brief завершение кодирования
void Arcoder::done_encoding(void)
{
	bits_to_follow++;
	if (low < FIRST_QTR) output_bit_plus_follow(0);
	else output_bit_plus_follow(1);
	*data_out = buffer >> bits_to_go;				// записать незаполненный буфер
	sizeOut++;
}

// @brief кодирование символа
// @param symbol - in, поступивший символ
void Arcoder::encode_symbol(int symbol)
{
	// пересчет границ интервала
	unsigned long range;
	range = high - low + 1;
	high = low + range*cum_freq[m_currentModel][NEGATIVE_SHIFT + symbol + 1] / cum_freq[m_currentModel][NO_OF_SYMBOLS] - 1;
	low = low + range*cum_freq[m_currentModel][NEGATIVE_SHIFT + symbol] / cum_freq[m_currentModel][NO_OF_SYMBOLS];
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
void Arcoder::encode(int8_t* in, int8_t* out, int size_in, int &size_out)
{
	data_in = in;
	data_out = out;

	int8_t symbol;
	start_model();
	start_encoding();

	int size = 0;

	// read data
	for (int i = 0; i < size_in; ++i)
	{
		symbol = data_in[i];
		encode_symbol(symbol);
		update_model(symbol);
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
			int8_t symbol = data_in[index];

			encode_symbol(symbol);
			update_model(symbol);
		}

		// change direction
		int temp = horizontalFrom;
		horizontalFrom = horizontalTo;
		horizontalTo = temp;
		step *= -1;
	}
}

// @brief кодирование информации в нелинейном порядке
void Arcoder::mappedEncode(int8_t* in, int8_t* out, SubbandMap map, int &size_out)
{
	data_in = in;
	data_out = out;

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

	encode_symbol(EOF_SYMBOL);
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

// @brief ввод 1 бита из сжатого файла
inline int Arcoder::input_bit(void)
{
	if (bits_to_go == 0)
	{
		buffer = *data_in;	// заполняем буфер битового ввода
		data_in++;
		sizeIn++;
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
void Arcoder::start_decoding(void)
{
	bits_to_go = 0;				// свободно бит в битовом буфере ввода
	garbage_bits = 0;				// контроль числа "мусорных" бит в конце сжатого файла
	low = 0;				// нижняя граница интервала
	high = TOP_VALUE;		// верхняя граница интервала
	value = 0;				// "ЧИСЛО"
	for (int i = 0; i<BITS_IN_REGISTER; i++) value = (value << 1) + input_bit();

	m_currentModel = 0;
}

// @brief декодирование символа
int Arcoder::decode_symbol()
{
	unsigned long range, cum;
	int symbol;
	range = high - low + 1;
	// число cum - это число value, пересчитанное из интервала
	// low..high в интервал 0..CUM_FREQUENCY[NO_OF_SYMBOLS]
	cum = ((value - low + 1)*cum_freq[m_currentModel][NO_OF_SYMBOLS] - 1) / range;
	// поиск интервала, соответствующего числу cum
	for (symbol = -128; cum_freq[m_currentModel][NEGATIVE_SHIFT + symbol + 1] <= cum; symbol++);
	// пересчет границ
	high = low + range*cum_freq[m_currentModel][NEGATIVE_SHIFT + symbol + 1] / cum_freq[m_currentModel][NO_OF_SYMBOLS] - 1;
	low = low + range*cum_freq[m_currentModel][NEGATIVE_SHIFT + symbol] / cum_freq[m_currentModel][NO_OF_SYMBOLS];
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
void Arcoder::decode(int8_t* in, int8_t* out, int size_in, int &size_out)
{
	data_in = in;
	data_out = out;

	int symbol;
	start_model();
	start_decoding();

	while ((symbol = decode_symbol()) != EOF_SYMBOL)
	{
		update_model(symbol);
		*data_out = symbol;
		data_out++;
		sizeOut++;
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
		}

		// change direction
		int temp = horizontalFrom;
		horizontalFrom = horizontalTo;
		horizontalTo = temp;
		step *= -1;
	}
}

// @brief декодирование информации в нелинейном порядке
void Arcoder::mappedDecode(int8_t* in, int8_t* out, SubbandMap map, int &size_out)
{
	data_in = in;
	data_out = out;
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
