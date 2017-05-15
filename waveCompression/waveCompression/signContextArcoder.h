#pragma once

#include "contextArcoder.h"

class SignContextArcoder : public ContextArcoder
{
public:

	SignContextArcoder(qMinCap qStruct, Context3x3 i_context, bool i__isContextForSignNeeded = false);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// encode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void basicEncode(int i_index);

	virtual void encodeSymbolByContext(int index, bool i_isOnTheBord = false);

	virtual void encodeSubband(SubbandRect rect);

	void encodeTopLeftSubband(SubbandRect rect);

	void encodeHorizontalSubband(SubbandRect rect);

	void encodeVerticalSubband(SubbandRect rect);

	void encodeDiagonalSubband(SubbandRect rect);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// decode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void basicDecode(int i_index);

	virtual void decodeSymbolByContext(int index, bool i_isOnTheBord = false);

	virtual void decodeSubband(SubbandRect rect);

	void decodeTopLeftSubband(SubbandRect rect);

	void decodeHorizontalSubband(SubbandRect rect);

	void decodeVerticalSubband(SubbandRect rect);

	void decodeDiagonalSubband(SubbandRect rect);

	// @brief find model for sign encoding
	// @param index - in, index of coef to encode
	int FindSignModel(int i_index, int *decoded_data);

private:
	bool m_isContextForSignNeeded;

	std::vector<double> m_signLimits;
};

