#pragma once

#include "contextArcoder.h"

class SignContextArcoder : public ContextArcoder
{
public:

	SignContextArcoder(Context3x3 i_context, bool i__isContextForSignNeeded = false);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// encode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void basicEncode(int i_index);

	virtual void encodeSymbolByContext(int index, bool i_isOnTheBord = false);

	virtual void encodeTopRow(int startIndex, int endIndex);

	virtual void encodeLeftColumn(int startIndex, int endIndex);

	virtual void encodeSubband(SubbandRect rect);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////
	///// decode
	/////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void basicDecode(int i_index);

	virtual void decodeSymbolByContext(int index, bool i_isOnTheBord = false);
	// @brief
	virtual void decodeTopRow(int startIndex, int endIndex) override;

	virtual void decodeLeftColumn(int startIndex, int endIndex);

	virtual void decodeSubband(SubbandRect rect);

	// @brief find model for sign encoding
	// @param index - in, index of coef to encode
	int FindSignModel(int i_index, int8_t *decoded_data);

private:
	bool m_isContextForSignNeeded;

	std::vector<double> m_signLimits;
};

