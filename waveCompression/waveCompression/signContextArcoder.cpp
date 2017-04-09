#include <process.h>
#include "signContextArcoder.h"

SignContextArcoder::SignContextArcoder(Context3x3 i_context)
{
	m_context = i_context;
	m_subbandType = 0;

	limits.push_back(0.2);
	limits.push_back(1);
	limits.push_back(10);

	m_numOfModelsNeeded = NO_OF_CHARS;

	for (int i = 0; i < m_numOfModelsNeeded; ++i)
	{
		m_model.emplace_back(128);
	}
	conv.Initialize();

	Model signModel(2);

	// add new sign model
	m_numOfModelsNeeded++;
	m_model.push_back(signModel);
}

void SignContextArcoder::encode_symbol(int symbol)
{
	Arcoder::encode_symbol(abs(symbol));
	update_model(abs(symbol));
	int storageCurModelNumber = m_currentModel;

	// encode sign bit
	m_currentModel = m_numOfModelsNeeded-1;
	if (symbol > 0)
	{
		Arcoder::encode_symbol(1);
	}
	else if (symbol < 0)
	{
		Arcoder::encode_symbol(0);
	}

	// back to symbol model
	m_currentModel = storageCurModelNumber;
}

int SignContextArcoder::decode_symbol()
{
	int uSymbol = Arcoder::decode_symbol();
	update_model(uSymbol);
	int storageCurModelNumber = m_currentModel;

	if (uSymbol == 0)
	{
		return 0;
	}

	// decode sign bit
	m_currentModel = m_numOfModelsNeeded-1;
	int signBit = Arcoder::decode_symbol();

	m_currentModel = storageCurModelNumber;

	return (signBit == 1) ? uSymbol: -uSymbol;
}

