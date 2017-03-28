#include "bmpStream.h"
#include <intrin.h>


#pragma warning(disable: 4996)

/////////////////////////////////////////////////////////////////////////////////////////////////
BmpImage::BmpImage(FILE* input_file, int channels)
{
	fread(header, sizeof(unsigned char), BYTES_IN_HEADER, input_file);	// read the 54-byte header
	int shift = *(int*)&header[10];				// shift of image data
	int garbageBytes = shift - BYTES_IN_HEADER;
	fseek(input_file, shift - BYTES_IN_HEADER, SEEK_CUR);					// skip garbage bytes

																			// extract image height and width from header
	width = *(int*)&header[18];
	height = *(int*)&header[22];

	int hSize = *(int*)&header[14];
	intToHeader(54, 10);

	int fSize = *(int*)&header[2];
	intToHeader(fSize - garbageBytes, 2);
	fSize = *(int*)&header[2];

	if (channels == 1)
	{
		int size = width * height;
		unsigned char* temp_data = new unsigned char[size]; // allocate 1 byte per pixel
		fread(temp_data, sizeof(unsigned char), size, input_file);	// read the rest of the data at once

		data = new unsigned char[width * height];

		// reverse data
		for (int y = height - 1; y >= 0; y--)
		{
			for (int x = 0; x < width; x++)
			{
				data[width * (height - 1 - y) + x] = temp_data[width*y + x];
			}
		}
		delete[] temp_data;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void BmpImage::WriteBmp(FILE* output_file)
{
	fwrite(&header, 1, BYTES_IN_HEADER, output_file);

	// приготовим временный буфер
	unsigned char *temp_data = new unsigned char[width*height];

	// reverse data
	for (int y = height - 1; y >= 0; y--)
	{
		for (int x = 0; x < width; x++)
		{
			temp_data[width * (height - 1 - y) + x] = data[width*y + x];
		}
	}

	// сбросим в файл
	fwrite(temp_data, 1, width*height, output_file);
	delete[]temp_data;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
int BmpImage::GetHeight()
{
	return height;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int BmpImage::GetWidth()
{
	return width;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t* BmpImage::GetUint8tData()
{
	return static_cast<uint8_t*>(data);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
double* BmpImage::GetDoubleData()
{
	double* output = new double[width * height];
	for (int i = 0; i < width * height; ++i)
	{
		output[i] = static_cast<double>(data[i]);
	}
	return output;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void BmpImage::SetDataFromDouble(double* input)
{
	for (int i = 0; i < width * height; ++i)
	{
		data[i] = static_cast<unsigned char>(input[i]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void BmpImage::intToHeader(int number, int positionInHeader)
{
	int* temp = (int*)&header[positionInHeader];
	(*temp) = number;
}

