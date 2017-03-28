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
	double min = 0;
	double max = 0;
	findMinMax(output, img_size, min, max);
	double q = findQ(min, max);

	uint8_t* post = quantArray(q, static_cast<double*>(output), img_size);

	// encode
	uint8_t* encoded = new uint8_t[img_size];
	int out_size = 0;
	encode(post, encoded, img_size, out_size);

	// mapped encode
	uint8_t* m_encoded = new uint8_t[img_size];
	int m_out_size = 0;
	CoderMap map(img.GetWidth(), img.GetHeight(), 4);
	mappedEncode(post, m_encoded, img_size, m_out_size, map);

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
	decode(encoded, decoded, out_size, foundedSize);

	// decode
	uint8_t* m_decoded = new uint8_t[img_size];
	int m_foundedSize = 0;
	mappedDecode(m_encoded, m_decoded, m_out_size, m_foundedSize, map);

	for (int i = 0; i < img_size; ++i)
	{
		if (encoded[i] != post[i])
		{
			int temp = 1;
		}
	}

	// dequant
	deQuantArray(q, m_decoded, output, foundedSize);

	// invert
	wvlt.invert2d(output, input, img.GetWidth(), img.GetHeight(), 4);

	img.SetDataFromDouble(static_cast<double*>(input));
	uint8_t umin = 0;
	uint8_t umax = 0;
	findMinMax(img.GetUint8tData(), img_size, umin, umax);

	img.WriteBmp(out);
}

void _cdecl main(int argc, char **argv)
{
	printf("\nAlpha version of arithmetic Codec 2\n");
	if (argc != 4 || argv[1][0] != 'e' && argv[1][0] != 'd')
		printf("\nUsage: arcode e|d infile outfile \n");
	else if ((in = fopen(argv[2], "r+b")) == NULL)
		printf("\nIncorrect input file\n");
	else if ((out = fopen(argv[3], "w+b")) == NULL)
		printf("\nIncorrect output file\n");
	else
	{
		int a = 0;
		if (argv[1][0] == 'e') encode();
		fclose(in);
		fclose(out);
	}
}