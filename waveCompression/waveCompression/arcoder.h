#pragma once

// 2 Simplest & slowest version of arithmetic codec (almost demo version)
#include <vector>
#include "quantor.h"

typedef unsigned int uint;

//дл€ избежани€ переполнени€:	MAX_FREQUENCY * (TOP_VALUE+1) < ULONG_MAX 
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

#define BUFFER_SIZE					8

/////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////
class Model
{
public:

	// @brief default ctor
	explicit Model(bool isEOFneeded = true);

	// @brief ctor
	// @param i_len - [in], capacity of the model
	// @param i_startValue - [in], start of real input/output model interval
	explicit Model(int i_numOfChars, bool isEOFneeded = false);

	// @brief
	void StartModel();

	void UpdateModel(int i_symbol);

	// @brief
	unsigned int GetFreq(int i_symbol);

	// @brief
	unsigned int GetLastFreq();

	// @brief 
	unsigned int findInterval(unsigned long cum)
	{
		int symbol;
		for (symbol = 0; GetFreq(symbol + 1) <= cum; symbol++);

		return symbol;
	}

	int GetEof();
	int GetFreqSize() { return m_numOfSymbols + 1; }
	unsigned int* GetFreqData() { return cum_freq; }

private:
	int m_numOfChars;
	int m_numOfSymbols;
	unsigned int *cum_freq;
};

/////////////////////////////////////////////////////////////////////////////////////////
class Converter
{
public:

	Converter() {};

	void Initialize(int i_numOfChars = 256,
			   int i_startValue = -128);
	//@brief
	// note: all the model functions which gets i_symbol as input
	//       must convert it 
	//		 from	[m_startValue .. m_startValue + m_numOfChars - 1]
	//		 to		[0 .. m_numOfChars - 1]
	//		 -1 because of zero is counts as char too
	uint ConvertToUnsigned(int i_symbol);

	// @brief
	int ConvertToSigned(int i_symbol);

private:
	int m_startValue;
};

/////////////////////////////////////////////////////////////////////////////////////////
class Arcoder
{
public:
	Arcoder();

	// memory arcoder ctor
	Arcoder(qMinCap i_qMinCap, int i_memoryLen);

	// @brief инициализаци€ массива частот
	virtual void start_model(void);

	// @brief обновление массива частот
	// @param symbol - in, поступивший символ
	inline virtual void update_model(int symbol);

	// @brief export model info to file
	void exportModelInformation(FILE* i_file);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// encode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// @brief инициализаци€ глобальных переменных
	virtual void start_encoding(void);

	// @brief вывод одного бита в сжатый файл
	inline virtual  void output_bit(int bit);

	// @brief вывод одного очередного бита и тех, которые были отложены
	inline virtual  void output_bit_plus_follow(int bit);

	// @brief завершение кодировани€
	virtual void done_encoding(void);

	// @brief кодирование символа
	// @param symbol - in, поступивший символ
	virtual void encode_symbol(int symbol);

	// @brief кодирование информации
	virtual void encode(int* in, int8_t* out, int size_in, int &size_out);

	//
	virtual void encodeSubband(SubbandRect rect);

	// @brief кодирование информации в нелинейном пор€дке
	virtual void mappedEncode(int* in, int8_t* out, SubbandMap map, int &size_out);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// decode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// @brief ввод 1 бита из сжатого файла
	inline virtual  int input_bit(void);

	// @brief инициализаци€ глобальных переменных
	virtual void start_decoding(void);

	// @brief декодирование символа
	virtual int decode_symbol();

	// @brief декодирование информации
	virtual void decode(int8_t* in, int* out, int size_in, int &size_out);

	// 
	virtual void decodeSubband(SubbandRect rect);

	// @brief декодирование информации в нелинейном пор€дке
	virtual void mappedDecode(int8_t* in, int* out, SubbandMap map, int &size_out);

protected:
	unsigned long						low, high, value;
	uint8_t								buffer, bits_to_go;
	int									garbage_bits, bits_to_follow;
	
	int* m_decodedData;
	int8_t* m_encodedData;
	int sizeOut, sizeIn = 0;
	int imgWidth, imgHeight;
	int m_currentModel;				//< model to encode/decode next symbol

	typedef std::vector<Model> Models;
	Models m_model;					//< array of models for encoding/decoding symbols
	int m_numOfModelsNeeded;
	Converter conv;
};
