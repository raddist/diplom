#include <process.h>
#include "signContextArcoder.h"

SignContextArcoder::SignContextArcoder(Context3x3 i_context, 
										bool i__isContextForSignNeeded):
	m_isContextForSignNeeded(i__isContextForSignNeeded)
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

	if (i__isContextForSignNeeded)
	{
		m_numOfModelsNeeded += 4;
		for (int i = 0; i < 4; ++i)
		{
			m_model.emplace_back(2);
		}
	}

	Model signModel(2);

	// add new sign model
	m_numOfModelsNeeded++;
	m_model.emplace_back(2);

	// add limits for sign encoding
	m_signLimits.push_back(-1);
	m_signLimits.push_back(0);
	m_signLimits.push_back(1);
}

///////////////////////////////////////////////////////////////
/*void SignContextArcoder::encode_symbol(int symbol)
{
	Arcoder::encode_symbol(abs(symbol));
	update_model(abs(symbol));
	int storageCurModelNumber = m_currentModel;

	// encode sign bit
	if (!m_isContextForSignNeeded)
	{
		m_currentModel = m_numOfModelsNeeded - 1;
	}
	else
	{
		FindSignModel(data_in);
	}

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
}*/

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::basicEncode(int i_index)
{
	int8_t symbol = data_in[i_index];

	Arcoder::encode_symbol(abs(symbol));
	update_model(abs(symbol));
	int storageCurModelNumber = m_currentModel;

	// encode sign bit
	m_currentModel = m_numOfModelsNeeded - 1;

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

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeSymbolByContext(int index, bool i_isOnTheBord)
{
	double p = calcP(index, data_in, i_isOnTheBord);
	m_currentModel = findModelByP(p);
	int8_t symbol = data_in[index];

	Arcoder::encode_symbol(abs(symbol));
	update_model(abs(symbol));
	int storageCurModelNumber = m_currentModel;

	// encode sign bit
	if (!m_isContextForSignNeeded && !i_isOnTheBord)
	{
		m_currentModel = m_numOfModelsNeeded - 1;
	}
	else
	{
		int reverseModelNum = FindSignModel(index, data_in);
		m_currentModel = m_numOfModelsNeeded - 1 - reverseModelNum;
	}

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

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeSubband(SubbandRect rect)
{
	int horizontalFrom = rect.left;
	int horizontalTo = rect.right - 1;	// include last pixel in [horizontalFrom horizontalTo]

	for (int j = rect.top; j < rect.bot; ++j)
	{
		for (int i = horizontalFrom; i <= horizontalTo; ++i)
		{
			int index = j*imgWidth + i;
			m_curSymbolIndex = index;

			if (rect.top == 0 && rect.left == 0)
			{
				basicEncode(index);
			}
			else if (j > 1  && i != 0)
			{
				if (j != rect.bot - 1 && i != horizontalTo)
				{
					encodeSymbolByContext(index);
				}
				else
				{
					encodeSymbolByContext(index, true);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////
/*int SignContextArcoder::decode_symbol()
{
	int uSymbol = Arcoder::decode_symbol();
	update_model(uSymbol);
	int storageCurModelNumber = m_currentModel;

	if (uSymbol == 0)
	{
		return 0;
	}

	// decode sign bit
	if (!m_isContextForSignNeeded)
	{
		m_currentModel = m_numOfModelsNeeded - 1;
	}
	else
	{
		FindSignModel(data_out);
	}

	int signBit = Arcoder::decode_symbol();

	m_currentModel = storageCurModelNumber;

	return (signBit == 1) ? uSymbol: -uSymbol;
}*/

void SignContextArcoder::basicDecode(int i_index)
{
	int uSymbol = Arcoder::decode_symbol();
	update_model(uSymbol);
	int storageCurModelNumber = m_currentModel;

	if (uSymbol == 0)
	{
		data_out[i_index] = 0;
	}
	else
	{
		// decode sign bit
		m_currentModel = m_numOfModelsNeeded - 1;

		int signBit = Arcoder::decode_symbol();

		m_currentModel = storageCurModelNumber;
		data_out[i_index] = (signBit == 1) ? uSymbol : -uSymbol;
	}

	sizeOut++;
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeSymbolByContext(int i_index, bool i_isOnTheBord)
{
	double p = calcP(i_index, data_out, i_isOnTheBord);
	m_currentModel = findModelByP(p);
	int uSymbol = Arcoder::decode_symbol();
	update_model(uSymbol);

	int storageCurModelNumber = m_currentModel;

	if (uSymbol == 0)
	{
		data_out[i_index] = 0;
	}
	else
	{
		// decode sign bit
		if (!m_isContextForSignNeeded && !i_isOnTheBord)
		{
			m_currentModel = m_numOfModelsNeeded - 1;
		}
		else
		{
			int reverseModelNum = FindSignModel(i_index, data_out);
			m_currentModel = m_numOfModelsNeeded - 1 - reverseModelNum;
		}

		int signBit = Arcoder::decode_symbol();

		m_currentModel = storageCurModelNumber;
		data_out[i_index] = (signBit == 1) ? uSymbol : -uSymbol;
	}

	sizeOut++;
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeSubband(SubbandRect rect)
{
	int horizontalFrom = rect.left;
	int horizontalTo = rect.right - 1;	// include last pixel in [horizontalFrom horizontalTo]

	for (int j = rect.top; j < rect.bot; ++j)
	{
		for (int i = horizontalFrom; i <= horizontalTo; ++i)
		{
			int index = j*imgWidth + i;
			m_curSymbolIndex = index;

			if (rect.top == 0 && rect.left == 0)
			{
				basicDecode(index);
			}
			else if (j > 1 && i != 0)
			{
				if (j != rect.bot - 1 && i != horizontalTo)
				{
					decodeSymbolByContext(index);
				}
				else
				{
					decodeSymbolByContext(index, true);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeTopRow(int startIndex, int endIndex)
{
	ContextArcoder::encodeTopRow(startIndex, endIndex);

	if (m_isContextForSignNeeded)
	{
		// encode one more top row
		ContextArcoder::encodeTopRow(startIndex + imgWidth, endIndex + imgWidth);
	}
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeTopRow(int startIndex, int endIndex)
{
	ContextArcoder::decodeTopRow(startIndex, endIndex);

	if (m_isContextForSignNeeded)
	{
		// decode one more top row
		ContextArcoder::decodeTopRow(startIndex + imgWidth, endIndex + imgWidth);
	}
}

///////////////////////////////////////////////////////////////
int SignContextArcoder::FindSignModel(int i_index, int8_t *decoded_data)
{
	double sum = 0;
	
	sum = -0.5 * (decoded_data[i_index - 2*imgWidth - 1] + decoded_data[i_index - 2 * imgWidth + 1] + 
		decoded_data[i_index - imgWidth - 1] + decoded_data[i_index - imgWidth + 1]) +
		(decoded_data[i_index - imgWidth] + decoded_data[i_index - 2*imgWidth]);

	for (int i = 0; i < static_cast<int>(m_signLimits.size() - 1); ++i)
	{
		if (sum < m_signLimits.at(i))
		{
			return i + 1;
		}
	}
	return limits.size() + 1;
}


