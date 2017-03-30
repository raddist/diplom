// 2 Simplest & slowest version of arithmetic codec (almost demo version)
#include <stdio.h>
#include <process.h>
#include <cstdint>
#include "wavelet.h"
#include "bmpStream.h"
#include "quantor.h"
#include "arcoder.h"

#pragma warning(disable: 4996)
FILE *in, *out;

// @brief кодирование информации
void encode(void)
{
	BmpImage img(in, 1);
	int img_size = img.GetHeight() * img.GetHeight();

	Real* input = static_cast<Real*>(img.GetDoubleData());

	Real *output = new Real[img_size];

	//using wavelet
	Wavelet wvlt(&Antonini);
	wvlt.transform2d(input, output, 512, 512, 4);

	// quant
	Quantor quant(static_cast<double*>(output), img_size);

	uint8_t* post = quant.quantArray(static_cast<double*>(output), img_size);

	// encode
	uint8_t* encoded = new uint8_t[img_size];
	int out_size = 0;
	Arcoder coder(post);
	coder.encode(post, encoded, img_size, out_size);

	// mapped encode
	uint8_t* m_encoded = new uint8_t[img_size];
	int m_out_size = 0;
	CoderMap map(img.GetWidth(), img.GetHeight(), 4);
	coder.mappedEncode(post, m_encoded, map, m_out_size);

	// write encoded data in file
	FILE* temp = fopen("encoded.bin", "w+b");
	fwrite(encoded, 1, out_size, temp);
	fclose(temp);

	// write map encoded data in file
	FILE* m_temp = fopen("map_encoded.bin", "w+b");
	fwrite(m_encoded, 1, m_out_size, m_temp);
	fclose(m_temp);

	// decode
	uint8_t* decoded = new uint8_t[img_size];
	int foundedSize = 0;
	coder.decode(encoded, decoded, out_size, foundedSize);

	// decode
	uint8_t* m_decoded = new uint8_t[img_size];
	int m_foundedSize = 0;
	coder.mappedDecode(m_encoded, m_decoded, map, m_foundedSize);

	// dequant
	quant.deQuantArray(m_decoded, output, m_foundedSize);

	// invert
	wvlt.invert2d(output, input, img.GetWidth(), img.GetHeight(), 4);

	img.SetDataFromDouble(static_cast<double*>(input));

	img.WriteBmp(out);
}

////////////////////////////////////////////////////////////////////////////////////////////
void qSchedule(double i_qConst)
{
	BmpImage img(in, 1);
	int img_size = img.GetHeight() * img.GetHeight();

	Real* input = static_cast<Real*>(img.GetDoubleData());
	Real *output = new Real[img_size];

	//using wavelet
	Wavelet wvlt(&Antonini);
	wvlt.transform2d(input, output, 512, 512, 4);

	// quant
	Quantor quant(i_qConst);
	uint8_t* post = quant.quantArray(static_cast<double*>(output), img_size);

	// encode
	uint8_t* encoded = new uint8_t[img_size];
	int out_size = 0;
	Arcoder coder(post);
	coder.encode(post, encoded, img_size, out_size);

	// mapped encode
	uint8_t* m_encoded = new uint8_t[img_size];
	int m_out_size = 0;
	CoderMap map(img.GetWidth(), img.GetHeight(), 4);
	coder.mappedEncode(post, m_encoded, map, m_out_size);

	// write encoded data in file
	FILE* temp = fopen("encoded.bin", "w+b");
	fwrite(encoded, 1, out_size, temp);
	fclose(temp);

	// write map encoded data in file
	FILE* m_temp = fopen("map_encoded.bin", "w+b");
	fwrite(m_encoded, 1, m_out_size, m_temp);
	fclose(m_temp);

	// decode
	uint8_t* decoded = new uint8_t[img_size];
	int foundedSize = 0;
	coder.decode(encoded, decoded, out_size, foundedSize);

	// mapped decode
	uint8_t* m_decoded = new uint8_t[img_size];
	int m_foundedSize = 0;
	coder.mappedDecode(m_encoded, m_decoded, map, m_foundedSize);

	// dequant
	quant.deQuantArray(m_decoded, output, m_foundedSize);

	// invert
	wvlt.invert2d(output, input, img.GetWidth(), img.GetHeight(), 4);
	img.SetDataFromDouble(static_cast<double*>(input));
	img.WriteBmp(out);
}

void _cdecl main(int argc, char **argv)
{
	printf("\nAlpha version of arithmetic Codec 2\n");
	if ((in = fopen(argv[2], "r+b")) == NULL)
		printf("\nIncorrect input file\n");
	else if ((out = fopen(argv[3], "w+b")) == NULL)
		printf("\nIncorrect output file\n");
	else
	{
		int a = 0;
		if (argv[1][0] == 'e') encode();
		if (argv[1][0] == 'q') qSchedule(atof(argv[4]));
		fclose(in);
		fclose(out);
	}
}