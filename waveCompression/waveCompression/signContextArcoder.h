#pragma once

#include "contextArcoder.h"

class SignContextArcoder : public ContextArcoder
{
public:

	SignContextArcoder(Context3x3 i_context);

	// @brief ����������� �������
	// @param symbol - in, ����������� ������
	virtual void encode_symbol(int symbol);

	// @brief ������������� �������
	virtual int decode_symbol();
};

