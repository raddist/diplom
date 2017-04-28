#include <iostream>
#include <stdio.h>

#pragma once

#define BYTES_IN_HEADER 54

typedef unsigned __int16 WORD;

typedef struct {
	WORD   bfType;         // 0x4d42 | 0x4349 | 0x5450
	int    bfSize;         // ������ �����
	WORD   bfReserved;     // 0
	WORD   bfOffBits;      // �������� �� ���� ������,
						   // ������ 54 = 16 + biSize
	int    biSize;         // ������ �������� � ������:
						   // 40(BITMAPINFOHEADER) ��� 108(BITMAPV4HEADER)
						   // ��� 124(BITMAPV5HEADER)
	int    biWidth;        // ������ � ������
	int    biHeight;       // ������ � ������
	WORD   biPlanes;       // ������ ������ ���� 1
	WORD   biBitCount;     // 0 | 1 | 4 | 8 | 16 | 24 | 32
	int    biCompression;  // BI_RGB | BI_RLE8 | BI_RLE4 |
						   // BI_BITFIELDS | BI_JPEG | BI_PNG
						   // ������� ������������ ���� BI_RGB
	int    biSizeImage;    // ���������� ���� � ���� ������
						   // ������ ��������������� � 0
	int    biXPelsPerMeter;// �������������� ����������, ����� �� ����
	int    biYPelsPerMeter;// ������������ ����������, ����� �� ����
	int    biClrUsed;      // ���������� ������������ ������
						   // (���� ���� ������� ������)
	int    biClrImportant; // ���������� ������������ ������.
						   // ����� �������, ������ 0
} BMPheader;

class BmpImage
{
public:

	BmpImage(FILE* input_file, int channels);

	void WriteBmp(FILE* output_file);

	int GetHeight();
	int GetWidth();

	uint8_t* GetUint8tData();
	double* GetDoubleData();

	void SetDataFromDouble(double* input);
	void SetDataFromInt(int* input);

private:
	void intToHeader(int number, int positionInHeader);

	unsigned char header[BYTES_IN_HEADER];
	int width;
	int height;
	int numOfChannels;

	unsigned char* data;	// inverted bmp data
};

// @brief function to read drom .bmp file
// @param f - in, file descriptor
// @param width - in/out, width of read image
// @param height - in/out, height of read image
unsigned char* readBMP(FILE* f, int &width, int &height);

double* uint8ToDouble(uint8_t* input, int size);

int *loadBMP(FILE* f, int &mx, int &my);

int saveBMP(FILE* f, uint8_t *v, int width, int height);
