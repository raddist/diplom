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

	ContextArcoder(qMinCap i_qMinCap, Context3x3 i_context);

	ContextArcoder(qMinCap i_qMinCap, Context3x3 i_context, Limits i_limits);

	void reset_model();

	double calcP(int index, int* decoded_data, bool i_isOnTheBord);

	int findModelByP(double p);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// encode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void basicEncode(int i_index);

	virtual void encodeSymbolByContext(int index, bool i_isOnTheBord = false);

	// @brief кодирование информации
	void encode(int* in, int8_t* out, SubbandMap map, int size_in, int &size_out);

	//
	virtual void encodeSubband(SubbandRect rect);

	virtual void encodeTopRow(int i_row, int i_startIndex);

	virtual void encodeLeftColumn(int i_col, int i_startIndex);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// decode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void basicDecode(int i_index);

	virtual void decodeSymbolByContext(int index, bool i_isOnTheBord = false);

	// @brief декодирование информации
	void decode(int8_t* in, int* out, SubbandMap map, int &size_out);

	//
	virtual void decodeSubband(SubbandRect rect);

	virtual void decodeTopRow(int i_row, int i_startIndex);

	virtual void decodeLeftColumn(int i_col, int i_startIndex);
	

protected:
	Context3x3 m_context;
	Limits limits;
	int m_subbandType;
	int m_curSymbolIndex;
};
