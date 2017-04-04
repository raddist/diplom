#pragma once

#include "arcoder.h"

#define NO_OF_MODELS 5
#define LIMITS_SIZE 3

struct Context3x3 {
	double* maskV;
	double* maskD;
	double* maskH;
};

typedef std::vector<double> Limits;

class ContextArcoder : public Arcoder
{
public:

	ContextArcoder();

	ContextArcoder(Context3x3 i_context);

	ContextArcoder(Context3x3 i_context, Limits i_limits);

	void reset_model();

	double calcP(int index, uint8_t* decoded_data);

	int findModelByP(double p);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// encode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// @brief кодирование информации
	void encode(uint8_t* in, uint8_t* out, SubbandMap map, int size_in, int &size_out);

	//
	void encodeSubband(SubbandRect rect);

	void encodeTopLeftSubbandSymbol(int index);

	void encodeSymbolByContext(int index);

	void encodeTopRow(int startIndex, int endIndex);

	void encodeLeftColumn(int startIndex, int endIndex);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// decode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// @brief декодирование информации
	void decode(uint8_t* in, uint8_t* out, SubbandMap map, int &size_out);

	//
	void decodeSubband(SubbandRect rect);

	void decodeTopLeftSubbandSymbol(int index);

	void decodeSymbolByContext(int index);

	void decodeTopRow(int startIndex, int endIndex);

	void decodeLeftColumn(int startIndex, int endIndex);
	

private:
	Context3x3 m_context;
	Limits limits;
	int m_subbandType;
};
