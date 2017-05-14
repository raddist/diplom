#include <process.h>
#include "signContextArcoder.h"

SignContextArcoder::SignContextArcoder(qMinCap qStruct,
										Context3x3 i_context, 
										bool i__isContextForSignNeeded):
	m_isContextForSignNeeded(i__isContextForSignNeeded)
{
	m_context = i_context;
	m_subbandType = 0;

	limits.push_back(0.2);
	limits.push_back(1);
	limits.push_back(10);

	m_numOfModelsNeeded = qStruct.arrCapacity;

	int modelSize = (abs(qStruct.minValue) > qStruct.arrCapacity + qStruct.minValue) ? abs(qStruct.minValue) + 1 : qStruct.arrCapacity + qStruct.minValue;
	for (int i = 0; i < m_numOfModelsNeeded; ++i)
	{
		m_model.emplace_back(modelSize);
	}
	conv.Initialize();

	// add limits for sign encoding
	m_signLimits.push_back(-15);
	m_signLimits.push_back(-5);
	m_signLimits.push_back(5);
	m_signLimits.push_back(15);

	if (i__isContextForSignNeeded)
	{
		m_numOfModelsNeeded += m_signLimits.size() + 1;
		for (int i = 0; i < m_signLimits.size() + 1; ++i)
		{
			m_model.emplace_back(2);
		}
	}

	// basic sign model
	Model signModel(2);

	// add new sign model
	m_numOfModelsNeeded++;
	m_model.emplace_back(2);
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::basicEncode(int i_index)
{
	int symbol = m_decodedData[i_index];

	Arcoder::encode_symbol(abs(symbol));
	update_model(abs(symbol));
	int storageCurModelNumber = m_currentModel;

	// encode sign bit
	m_currentModel = m_numOfModelsNeeded - 1;

	if (symbol > 0)
	{
		Arcoder::encode_symbol(1);
		update_model(1);
	}
	else if (symbol < 0)
	{
		Arcoder::encode_symbol(0);
		update_model(0);
	}

	// back to symbol model
	m_currentModel = storageCurModelNumber;
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeSymbolByContext(int index, bool i_isOnTheBord)
{
	double p = calcP(index, m_decodedData, i_isOnTheBord);
	m_currentModel = findModelByP(p);
	int symbol = m_decodedData[index];

	Arcoder::encode_symbol(abs(symbol));
	update_model(abs(symbol));
	int storageCurModelNumber = m_currentModel;

	// encode sign bit
	if (i_isOnTheBord || !m_isContextForSignNeeded)
	{
		m_currentModel = m_numOfModelsNeeded - 1;
	}
	else if(symbol != 0)
	{
		int reverseModelNum = FindSignModel(index, m_decodedData);
		m_currentModel = m_numOfModelsNeeded - 1 - reverseModelNum;
	}

	if (symbol > 0)
	{
		Arcoder::encode_symbol(1);
		update_model(1);
	}
	else if (symbol < 0)
	{
		Arcoder::encode_symbol(0);
		update_model(0);
	}
	// back to symbol model
	m_currentModel = storageCurModelNumber;
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeSubband(SubbandRect rect)
{
	switch (m_subbandType)
	{
	case 0:
		encodeTopLeftSubband(rect);
		break;
	case 1:
		encodeVerticalSubband(rect);
		break;
	case 2:	
		encodeHorizontalSubband(rect);
		break;
	case 3:
		encodeDiagonalSubband(rect);
		break;
	}
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeTopLeftSubband(SubbandRect rect)
{
	for (int row = rect.top; row < rect.bot; ++row)
	{
		for (int col = rect.left; col < rect.right; ++col)
		{
			int index = row*imgWidth + col;
			basicEncode(index);
		}
	}
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeHorizontalSubband(SubbandRect rect)
{
	int startLeftIndex = m_isContextForSignNeeded ? 2 : 1;
	for (int col = startLeftIndex; col < rect.right; ++col)
	{
		for (int row = rect.top; row < rect.bot; ++row)
		{
			int index = row*imgWidth + col;

			if (row != rect.bot - 1 && col != rect.right - 1)
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

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeVerticalSubband(SubbandRect rect) 
{
	int startTopIndex = m_isContextForSignNeeded ? 2 : 1;

	for (int row = startTopIndex; row < rect.bot; ++row)
	{
		for (int col = rect.left; col < rect.right; ++col)
		{
			int index = row*imgWidth + col;

			if (row != rect.bot - 1 && col != rect.right - 1)
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

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeDiagonalSubband(SubbandRect rect)
{
	for (int row = rect.top; row < rect.bot; ++row)	
	{
		for (int col = rect.left; col < rect.right; ++col)
		{
			int index = row*imgWidth + col;

			if (row != rect.bot - 1 && col < rect.right - 2)
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

void SignContextArcoder::basicDecode(int i_index)
{
	int uSymbol = Arcoder::decode_symbol();
	update_model(uSymbol);
	int storageCurModelNumber = m_currentModel;

	if (uSymbol == 0)
	{
		m_decodedData[i_index] = 0;
	}
	else
	{
		// decode sign bit
		m_currentModel = m_numOfModelsNeeded - 1;

		int signBit = Arcoder::decode_symbol();
		update_model(signBit);

		m_currentModel = storageCurModelNumber;
		m_decodedData[i_index] = (signBit == 1) ? uSymbol : -uSymbol;
	}

	sizeOut++;
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeSymbolByContext(int i_index, bool i_isOnTheBord)
{
	double p = calcP(i_index, m_decodedData, i_isOnTheBord);
	m_currentModel = findModelByP(p);
	int uSymbol = Arcoder::decode_symbol();
	update_model(uSymbol);

	int storageCurModelNumber = m_currentModel;

	if (uSymbol == 0)
	{
		m_decodedData[i_index] = 0;
	}
	else
	{
		// decode sign bit
		if (i_isOnTheBord || !m_isContextForSignNeeded)
		{
			m_currentModel = m_numOfModelsNeeded - 1;
		}
		else if (uSymbol != 0)
		{
			int reverseModelNum = FindSignModel(i_index, m_decodedData);
			m_currentModel = m_numOfModelsNeeded - 1 - reverseModelNum;
		}

		int signBit = Arcoder::decode_symbol();
		update_model(signBit);

		m_currentModel = storageCurModelNumber;
		m_decodedData[i_index] = (signBit == 1) ? uSymbol : -uSymbol;
	}

	sizeOut++;
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeSubband(SubbandRect rect)
{
	switch (m_subbandType)
	{
	case 0:
		decodeTopLeftSubband(rect);
		break;
	case 1:
		decodeVerticalSubband(rect);	
		break;
	case 2:
		decodeHorizontalSubband(rect);
		break;
	case 3:
		decodeDiagonalSubband(rect);
		break;
	}
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeTopLeftSubband(SubbandRect rect)
{
	for (int row = rect.top; row < rect.bot; ++row)
	{
		for (int col = rect.left; col < rect.right; ++col)
		{
			int index = row*imgWidth + col;
			basicDecode(index);
		}
	}
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeHorizontalSubband(SubbandRect rect)
{
	int startLeftIndex = m_isContextForSignNeeded ? 2 : 1;
	for (int col = startLeftIndex; col < rect.right; ++col)
	{
		for (int row = rect.top; row < rect.bot; ++row)
		{
			int index = row*imgWidth + col;

			if (row != rect.bot - 1 && col != rect.right - 1)
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

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeVerticalSubband(SubbandRect rect)
{
	int startTopIndex = m_isContextForSignNeeded ? 2 : 1;

	for (int row = startTopIndex; row < rect.bot; ++row)
	{
		for (int col = rect.left; col < rect.right; ++col)
		{
			int index = row*imgWidth + col;

			if (row != rect.bot - 1 && col != rect.right - 1)
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

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeDiagonalSubband(SubbandRect rect)
{
	for (int row = rect.top; row < rect.bot; ++row)
	{
		for (int col = rect.left; col < rect.right; ++col)
		{
			int index = row*imgWidth + col;

			if (row != rect.bot - 1 && col < rect.right - 2)
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

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeTopRow(int i_row, int i_startIndex)
{
	ContextArcoder::encodeTopRow(0, i_startIndex);

	if (m_isContextForSignNeeded)
	{
		// encode one more top row
		ContextArcoder::encodeTopRow(1, i_startIndex);
	}
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::encodeLeftColumn(int i_col, int i_startIndex)
{
	ContextArcoder::encodeLeftColumn(0, i_startIndex);

	if (m_isContextForSignNeeded)
	{
		// encode one more top row
		ContextArcoder::encodeLeftColumn(1, i_startIndex);
	}
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeTopRow(int i_row, int i_startIndex)
{
	ContextArcoder::decodeTopRow(0, i_startIndex);

	if (m_isContextForSignNeeded)
	{
		// decode one more top row
		ContextArcoder::decodeTopRow(1, i_startIndex);
	}
}

///////////////////////////////////////////////////////////////////////
void SignContextArcoder::decodeLeftColumn(int i_col, int i_startIndex)
{
	ContextArcoder::decodeLeftColumn(0, i_startIndex);

	if (m_isContextForSignNeeded)
	{
		// decode one more top row
		ContextArcoder::decodeLeftColumn(1, i_startIndex);
	}
}

///////////////////////////////////////////////////////////////
int SignContextArcoder::FindSignModel(int i_index, int *decoded_data)
{
	double sum = 0;
	
	switch (m_subbandType)
	{
	case 1:	// vertical
		sum = -0.5 * (decoded_data[i_index - 2 * imgWidth - 1] + decoded_data[i_index - 2 * imgWidth + 1] +
			decoded_data[i_index - imgWidth - 1] + decoded_data[i_index - imgWidth + 1]) +
			(decoded_data[i_index - imgWidth] + decoded_data[i_index - 2 * imgWidth]);
		break;

	case 2:	// horizontal
		sum = -0.5 * (decoded_data[i_index - imgWidth - 2] + decoded_data[i_index -  imgWidth - 1] +
			decoded_data[i_index + imgWidth - 2] + decoded_data[i_index + imgWidth - 1]) +
			(decoded_data[i_index -2] + decoded_data[i_index - 1]);
		break;
	case 3:
	{
		// diagonal
		double leftSum = -0.5 * (decoded_data[i_index - imgWidth - 2] + decoded_data[i_index - 1] +
			decoded_data[i_index - 2 * imgWidth - 1] + decoded_data[i_index - imgWidth ]) +
			(decoded_data[i_index - 2*imgWidth - 2] + decoded_data[i_index - imgWidth - 1]);

		double rightSum = -0.5 * (decoded_data[i_index - imgWidth + 2] + 
			decoded_data[i_index - 2 * imgWidth + 1] + decoded_data[i_index - imgWidth]) -
			1.0 * (decoded_data[i_index - imgWidth + 2]) +
			(decoded_data[i_index - 2 * imgWidth - 2] + decoded_data[i_index - imgWidth - 1]);
		sum = (abs(leftSum) > abs(rightSum)) ? leftSum : rightSum;
		break;
	}
	}

	for (int i = 0; i < static_cast<int>(m_signLimits.size()); ++i)
	{
		if (sum < m_signLimits.at(i))
		{
			return i + 1;
		}
	}
	return m_signLimits.size() + 1;
}


