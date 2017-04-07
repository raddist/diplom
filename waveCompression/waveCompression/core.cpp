// 2 Simplest & slowest version of arithmetic codec (almost demo version)
#include <stdio.h>
#include <process.h>
#include <cstdint>
#include "wavelet.h"
#include "bmpStream.h"
#include "quantor.h"
#include "arcoder.h"
#include "contextArcoder.h"

#pragma warning(disable: 4996)
FILE *in, *out;

// @brief кодирование информации
void encode()
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

	int8_t* post = quant.quantArray(static_cast<double*>(output), img_size);

	// encode
	int8_t* encoded = new int8_t[img_size];
	int out_size = 0;
	Arcoder coder;
	Arcoder Mcoder(1);
	coder.encode(post, encoded, img_size, out_size);

	// mapped encode
	int8_t* m_encoded = new int8_t[img_size];
	int m_out_size = 0;
	SubbandMap map(img.GetWidth(), img.GetHeight(), 4);
	Mcoder.mappedEncode(post, m_encoded, map, m_out_size);

	// context encode
	double ctemp[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
	double ctemp1[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
	Context3x3 context;
	context.maskD = ctemp;
	context.maskH = ctemp1;
	context.maskV = ctemp1;
	ContextArcoder Carcoder(context);
	int8_t* c_encoded = new int8_t[img_size];
	int c_out_size = 0;
	Carcoder.encode(post, c_encoded, map, img_size, c_out_size);

	// write encoded data in file
	FILE* temp = fopen("encoded.bin", "w+b");
	fwrite(encoded, 1, out_size, temp);
	fclose(temp);

	// write map encoded data in file
	FILE* m_temp = fopen("map_encoded.bin", "w+b");
	fwrite(m_encoded, 1, m_out_size, m_temp);
	fclose(m_temp);

	// decode
	int8_t* decoded = new int8_t[img_size];
	int foundedSize = 0;
	coder.decode(encoded, decoded, out_size, foundedSize);

	// decode
	int8_t* m_decoded = new int8_t[img_size];
	int m_foundedSize = 0;
	Mcoder.mappedDecode(m_encoded, m_decoded, map, m_foundedSize);

	// context decode
	int8_t* c_decoded = new int8_t[img_size];
	int c_foundedSize = 0;
	Carcoder.decode(c_encoded, c_decoded, map, c_foundedSize);

	// dequant
	quant.deQuantArray(c_decoded, output, m_foundedSize);

	// invert
	wvlt.invert2d(output, input, img.GetWidth(), img.GetHeight(), 4);

	img.SetDataFromDouble(static_cast<double*>(input));

	img.WriteBmp(out);

	delete[] m_decoded;
	delete[] m_encoded;
	delete[] post;
	delete[] encoded;
	delete[] decoded;
	delete[] input;
	delete[] output;
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
	int8_t* post = quant.quantArray(static_cast<double*>(output), img_size);

	// encode
	int8_t* encoded = new int8_t[img_size];
	int out_size = 0;
	Arcoder coder(0);
	Arcoder Mcoder(1);
	coder.encode(post, encoded, img_size, out_size);

	// mapped encode
	int8_t* m_encoded = new int8_t[img_size];
	int m_out_size = 0;
	SubbandMap map(img.GetWidth(), img.GetHeight(), 4);
	Mcoder.mappedEncode(post, m_encoded, map, m_out_size);

	// context encode
	double ctemp[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
	double ctemp1[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
	Context3x3 context;
	context.maskD = ctemp;
	context.maskH = ctemp1;
	context.maskV = ctemp1;
	ContextArcoder Carcoder(context);
	int8_t* c_encoded = new int8_t[img_size];
	int c_out_size = 0;
	Carcoder.encode(post, c_encoded, map, img_size, c_out_size);

	// write encoded data in file
	FILE* temp = fopen("encoded.bin", "w+b");
	fwrite(encoded, 1, out_size, temp);
	fclose(temp);

	// write map encoded data in file
	FILE* m_temp = fopen("map_encoded.bin", "w+b");
	fwrite(m_encoded, 1, m_out_size, m_temp);
	fclose(m_temp);

	// write context encoded data in file
	FILE* c_temp = fopen("con_encoded.bin", "w+b");
	fwrite(c_encoded, 1, c_out_size, c_temp);
	fclose(c_temp);

	// decode
	int8_t* decoded = new int8_t[img_size];
	int foundedSize = 0;
	coder.decode(encoded, decoded, out_size, foundedSize);

	// mapped decode
	int8_t* m_decoded = new int8_t[img_size];
	int m_foundedSize = 0;
	Mcoder.mappedDecode(m_encoded, m_decoded, map, m_foundedSize);

	// context decode
	int8_t* c_decoded = new int8_t[img_size];
	int c_foundedSize = 0;
	Carcoder.decode(c_encoded, c_decoded, map, c_foundedSize);

	// dequant
	quant.deQuantArray(m_decoded, output, m_foundedSize);

	// invert
	wvlt.invert2d(output, input, img.GetWidth(), img.GetHeight(), 4);
	img.SetDataFromDouble(static_cast<double*>(input));
	img.WriteBmp(out);
}

/////////////////////////////////////////////////////////////////////////////
int test3(char **argv)
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
	int8_t* post = quant.quantArray(static_cast<double*>(output), img_size);


	SubbandMap map(img.GetWidth(), img.GetHeight(), 4);
	// context encode
	double ctemp[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
	double ctemp1[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
	Context3x3 context;
	context.maskD = ctemp;
	context.maskH = ctemp1;
	context.maskV = ctemp1;
	//
	Limits limits;
	limits.push_back(atof(argv[4]));
	limits.push_back(atof(argv[5]));
	limits.push_back(atof(argv[6]));
	//
	ContextArcoder Carcoder(context, limits);
	int8_t* c_encoded = new int8_t[img_size];
	int c_out_size = 0;
	Carcoder.encode(post, c_encoded, map, img_size, c_out_size);

	// write encoded data in file
	FILE* temp = fopen("encoded.bin", "w+b");
	fwrite(c_encoded, 1, c_out_size, temp);
	fclose(temp);

	// context decode
	int8_t* c_decoded = new int8_t[img_size];
	int c_foundedSize = 0;
	Carcoder.decode(c_encoded, c_decoded, map, c_foundedSize);

	for (int i = 0; i < img_size; ++i)
	{
		if (post[i] != c_decoded[i])
		{
			int tempdas = 0;
		}
	}

	// dequant
	quant.deQuantArray(c_decoded, output, c_foundedSize);

	// invert
	wvlt.invert2d(output, input, img.GetWidth(), img.GetHeight(), 4);

	img.SetDataFromDouble(static_cast<double*>(input));

	img.WriteBmp(out);

	delete[] post;
	delete[] c_encoded;
	delete[] c_decoded;
	delete[] input;
	delete[] output;

	printf("\n%d\n", c_out_size);

	return c_out_size;
}


void _cdecl main(int argc, char **argv)
{
	if ((in = fopen(argv[2], "r+b")) == NULL)
		printf("\nIncorrect input file\n");
	else if ((out = fopen(argv[3], "w+b")) == NULL)
		printf("\nIncorrect output file\n");
	else
	{
		int a = 0;
		if (argv[1][0] == '1') encode();
		if (argv[1][0] == '2') qSchedule(atof(argv[4]));
		if (argv[1][0] == '3') test3(argv);
		fclose(in);
		fclose(out);
	}
}