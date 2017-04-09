#pragma once

#include "contextArcoder.h"

class SignContextArcoder : public ContextArcoder
{
public:

	SignContextArcoder(Context3x3 i_context);

	// @brief кодирование символа
	// @param symbol - in, поступивший символ
	virtual void encode_symbol(int symbol);

	// @brief декодирование символа
	virtual int decode_symbol();
};

