// 2 Simplest & slowest version of arithmetic codec (almost demo version)
#include "utils.h"

#pragma warning(disable: 4996)
FILE *in, *out;



class TestManager
{
public:
	TestManager()
	{}

	void PrintNumOfTests()
	{
		printf("\n%d\n", 5);

	}

	void DoTest(int i_test, int i_q = 0)
	{
		switch (i_test)
		{
		case 0:
		{
			//do all tests
			test0(i_q);
			break;
		}
		case 1:
		{
			test1();
			break;
		}
		case 2:
		{
			test2();
			break;
		}
		case 3:
		{
			test3();
			break;
		}
		case 4:
		{
			test4();
			break;
		}
		case 5:
		{
			test5();
			break;
		}
		}
	}


	~TestManager()
	{}

private:

	// @brief compare all arcoders
	void test0(double i_q)
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilization(transformed, quanted, img_size, i_q);

		// preapare for encode/decode
		double ctemp[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		double ctemp1[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		Context3x3 context;
		context.maskD = ctemp;
		context.maskH = ctemp1;
		context.maskV = ctemp1;

		double ctempH[] = { 0.2, 0.0, 0.0 ,  0.4, 0.0, 0.0 ,  0.2, 0.0, 0.0 };
		double ctempV[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		double ctempD[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		Context3x3 context_2;
		context_2.maskD = ctempD;
		context_2.maskH = ctempH;
		context_2.maskV = ctempV;

		SubbandMap map(img.GetWidth(), img.GetHeight(), 4);
		///////////////////////////////////////////////////////////////////////////////////////

		// encode 1
		int8_t* encoded_1 = new int8_t[img_size];
		int out_size_1 = 0;
		Arcoder coder(qStruct, 0);
		coder.encode(quanted, encoded_1, img_size, out_size_1);

		// decode 1
		int* decoded_1 = new int[img_size];
		int foundedSize = 0;
		coder.decode(encoded_1, decoded_1, out_size_1, foundedSize);
		///////////////////////////////////////////////////////////////////////////////////////

		// encode 2
		int8_t* encoded_2 = new int8_t[img_size];
		int out_size_2 = 0;
		Arcoder Mcoder(qStruct, 1);
		Mcoder.mappedEncode(quanted, encoded_2, map, out_size_2);

		// decode 2
		int* decoded_2 = new int[img_size];
		int foundedSize_2 = 0;
		Mcoder.mappedDecode(encoded_2, decoded_2, map, foundedSize_2);
		///////////////////////////////////////////////////////////////////////////////////////

		// encode 3
		ContextArcoder Carcoder(qStruct, context);
		int8_t* encoded_3 = new int8_t[img_size];
		int out_size_3 = 0;
		Carcoder.encode(quanted, encoded_3, map, img_size, out_size_3);

		// decode 3
		int* decoded_3 = new int[img_size];
		int foundedSize_3 = 0;
		Carcoder.decode(encoded_3, decoded_3, map, foundedSize_3);
		///////////////////////////////////////////////////////////////////////////////////////

		// encode 4
		SignContextArcoder sCarcoder(qStruct, context_2);
		int8_t* encoded_4 = new int8_t[img_size];
		int out_size_4 = 0;
		sCarcoder.encode(quanted, encoded_4, map, img_size, out_size_4);

		// decode 4
		int* decoded_4 = new int[img_size];
		int foundedSize_4 = 0;
		sCarcoder.decode(encoded_4, decoded_4, map, foundedSize_4);
		///////////////////////////////////////////////////////////////////////////////////////

		// encode 5
		SignContextArcoder Sarcoder(qStruct, context_2, true);
		int8_t* encoded_5 = new int8_t[img_size];
		int out_size_5 = 0;
		Sarcoder.encode(quanted, encoded_5, map, img_size, out_size_5);

		// decode 5
		int* decoded_5 = new int[img_size];
		int foundedSize_5 = 0;
		Sarcoder.decode(encoded_5, decoded_5, map, foundedSize_5);
		///////////////////////////////////////////////////////////////////////////////////////

		// quantilization
		double* deQuanted = new double[img_size];
		deQuantilization(decoded_1, deQuanted, img_size, i_q);

		// using wavelet invert
		double* inverted = new double[img_size];
		waveletInvert2D(deQuanted, inverted, img.GetWidth(), img.GetHeight(), 4);

		img.SetDataFromDouble(static_cast<double*>(inverted));
		img.WriteBmp(out);

		// tests
		compareArraysThroughMap(quanted, decoded_5, img_size, map);

		// write files
		// write encoded data in file
		FILE* temp = fopen("encoded.bin", "w+b");
		fwrite(encoded_1, 1, out_size_1, temp);
		fclose(temp);

		// write map encoded data in file
		FILE* m_temp = fopen("map_encoded.bin", "w+b");
		fwrite(encoded_2, 1, out_size_2, m_temp);
		fclose(m_temp);

		// write context encoded data in file
		FILE* c_temp = fopen("con_encoded.bin", "w+b");
		fwrite(encoded_3, 1, out_size_3, c_temp);
		fclose(c_temp);

		// write sign context encoded data in file
		FILE* s_temp = fopen("s_encoded.bin", "w+b");
		fwrite(encoded_4, 1, out_size_4, s_temp);
		fclose(s_temp);

		// write sign context encoded data in file
		FILE* sc_temp = fopen("sc_encoded.bin", "w+b");
		fwrite(encoded_5, 1, out_size_5, sc_temp);
		fclose(sc_temp);

		// realloc
		delete[] transformed;
		delete[] quanted;
		delete[] encoded_1;
		delete[] decoded_1;
		delete[] encoded_2;
		delete[] decoded_2;
		delete[] encoded_3;
		delete[] decoded_3;
		delete[] encoded_4;
		delete[] decoded_4;
		delete[] encoded_5;
		delete[] decoded_5;
		delete[] deQuanted;
		delete[] inverted;
	}

	// @brief test standart encode/decode
	void test1()
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilization(transformed, quanted, img_size, 27);

		// encode
		int8_t* encoded = new int8_t[img_size];
		int out_size = 0;
		Arcoder coder(qStruct, 0);
		coder.encode(quanted, encoded, img_size, out_size);

		// decode
		int* decoded = new int[img_size];
		int foundedSize = 0;
		coder.decode(encoded, decoded, out_size, foundedSize);

		// quantilization
		double* deQuanted = new double[img_size];
		deQuantilization(decoded, deQuanted, img_size, 27);

		// using wavelet invert
		double* inverted = new double[img_size];
		waveletInvert2D(deQuanted, inverted, img.GetWidth(), img.GetHeight(), 4);

		img.SetDataFromDouble(static_cast<double*>(inverted));
		img.WriteBmp(out);

		// tests

		// realloc
		delete[] transformed;
		delete[] quanted;
		delete[] encoded;
		delete[] decoded;
		delete[] deQuanted;
		delete[] inverted;
	}

	// @brief test mapped encode/decode with memory
	void test2()
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilization(transformed, quanted, img_size, 27);

		// encode
		int8_t* encoded = new int8_t[img_size];
		int out_size = 0;
		Arcoder Mcoder(qStruct, 1);
		SubbandMap map(img.GetWidth(), img.GetHeight(), 4);
		Mcoder.mappedEncode(quanted, encoded, map, out_size);

		// decode
		int* decoded = new int[img_size];
		int foundedSize = 0;
		Mcoder.mappedDecode(encoded, decoded, map, foundedSize);

		// quantilization
		double* deQuanted = new double[img_size];
		deQuantilization(decoded, deQuanted, img_size, 27);

		// using wavelet invert
		double* inverted = new double[img_size];
		waveletInvert2D(deQuanted, inverted, img.GetWidth(), img.GetHeight(), 4);

		img.SetDataFromDouble(static_cast<double*>(inverted));
		img.WriteBmp(out);

		// tests

		// realloc
		delete[] transformed;
		delete[] quanted;
		delete[] encoded;
		delete[] decoded;
		delete[] deQuanted;
		delete[] inverted;
	}

	// @brief test context enocode/decode
	void test3()
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilization(transformed, quanted, img_size, 27);

		// encode
		double ctemp[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		double ctemp1[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		Context3x3 context;
		context.maskD = ctemp;
		context.maskH = ctemp1;
		context.maskV = ctemp1;
		ContextArcoder Carcoder(qStruct, context);
		SubbandMap map(img.GetWidth(), img.GetHeight(), 4);
		int8_t* encoded = new int8_t[img_size];
		int c_out_size = 0;
		Carcoder.encode(quanted, encoded, map, img_size, c_out_size);

		// decode
		int* decoded = new int[img_size];
		int foundedSize = 0;
		Carcoder.decode(encoded, decoded, map, foundedSize);

		// quantilization
		double* deQuanted = new double[img_size];
		deQuantilization(decoded, deQuanted, img_size, 27);

		// using wavelet invert
		double* inverted = new double[img_size];
		waveletInvert2D(deQuanted, inverted, img.GetWidth(), img.GetHeight(), 4);

		img.SetDataFromDouble(static_cast<double*>(inverted));
		img.WriteBmp(out);

		// tests
		compareArrays(quanted, decoded, img_size);

		// realloc
		delete[] transformed;
		delete[] quanted;
		delete[] encoded;
		delete[] decoded;
		delete[] deQuanted;
		delete[] inverted;
	}

	// @brief test context encode/decode with sign
	void test4()
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilization(transformed, quanted, img_size, 27);

		// encode
		double ctempH[] = { 0.2, 0.0, 0.0 ,  0.4, 0.0, 0.0 ,  0.2, 0.0, 0.0 };
		double ctempV[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		double ctempD[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		Context3x3 context;
		context.maskD = ctempD;
		context.maskH = ctempH;
		context.maskV = ctempV;
		SignContextArcoder Carcoder(qStruct, context);
		SubbandMap map(img.GetWidth(), img.GetHeight(), 4);
		int8_t* encoded = new int8_t[img_size];
		int c_out_size = 0;
		Carcoder.encode(quanted, encoded, map, img_size, c_out_size);

		// decode
		int* decoded = new int[img_size];
		int foundedSize = 0;
		Carcoder.decode(encoded, decoded, map, foundedSize);

		// quantilization
		double* deQuanted = new double[img_size];
		deQuantilization(decoded, deQuanted, img_size, 27);

		// using wavelet invert
		double* inverted = new double[img_size];
		waveletInvert2D(deQuanted, inverted, img.GetWidth(), img.GetHeight(), 4);

		img.SetDataFromDouble(static_cast<double*>(inverted));
		img.WriteBmp(out);

		// tests
		compareArrays(quanted, decoded, img_size);

		// realloc
		delete[] transformed;
		delete[] quanted;
		delete[] encoded;
		delete[] decoded;
		delete[] deQuanted;
		delete[] inverted;
	}

	// @brief test sign context encode/decode
	void test5()
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilization(transformed, quanted, img_size, 27);

		// encode
		double ctempH[] = { 0.2, 0.0, 0.0 ,  0.4, 0.0, 0.0 ,  0.2, 0.0, 0.0 };
		double ctempV[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		double ctempD[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		Context3x3 context;
		context.maskD = ctempD;
		context.maskH = ctempH;
		context.maskV = ctempV;

		SignContextArcoder Carcoder(qStruct, context, true);
		SubbandMap map(img.GetWidth(), img.GetHeight(), 4);
		int8_t* encoded = new int8_t[img_size];
		int c_out_size = 0;
		Carcoder.encode(quanted, encoded, map, img_size, c_out_size);

		// decode
		int* decoded = new int[img_size];
		int foundedSize = 0;
		Carcoder.decode(encoded, decoded, map, foundedSize);

		// quantilization
		double* deQuanted = new double[img_size];
		deQuantilization(decoded, deQuanted, img_size, 27);

		// using wavelet invert
		double* inverted = new double[img_size];
		waveletInvert2D(deQuanted, inverted, img.GetWidth(), img.GetHeight(), 4);

		img.SetDataFromDouble(static_cast<double*>(inverted));
		img.WriteBmp(out);

		// tests
		compareArrays(quanted, decoded, img_size);

		// realloc
		delete[] transformed;
		delete[] quanted;
		delete[] encoded;
		delete[] decoded;
		delete[] deQuanted;
		delete[] inverted;
	}
};




// @brief кодирование информации
/*void encode()
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

	// sign context encode
	SignContextArcoder Sarcoder(context);
	int8_t* s_encoded = new int8_t[img_size];
	int s_out_size = 0;
	Sarcoder.encode(post, s_encoded, map, img_size, s_out_size);

	// context for sign
	SignContextArcoder SCarcoder(context, true);
	int8_t* sc_encoded = new int8_t[img_size];
	int sc_out_size = 0;
	SCarcoder.encode(post, sc_encoded, map, img_size, sc_out_size);

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

	// write sign context encoded data in file
	FILE* s_temp = fopen("s_encoded.bin", "w+b");
	fwrite(s_encoded, 1, s_out_size, s_temp);
	fclose(s_temp);

	// write sign context encoded data in file
	FILE* sc_temp = fopen("sc_encoded.bin", "w+b");
	fwrite(sc_encoded, 1, sc_out_size, sc_temp);
	fclose(sc_temp);

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

	// sign context decode
	int8_t* s_decoded = new int8_t[img_size];
	int s_foundedSize = 0;
	Sarcoder.decode(s_encoded, s_decoded, map, s_foundedSize);

	// context for signdecode
	int8_t* sc_decoded = new int8_t[img_size];
	int sc_foundedSize = 0;
	SCarcoder.decode(sc_encoded, sc_decoded, map, sc_foundedSize);

	// dequant
	quant.deQuantArray(decoded, output, sc_foundedSize);

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

/////////////////////////////////////////////////////////////////////////////
int test4()
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
	SignContextArcoder Sarcoder(context);
	int8_t* s_encoded = new int8_t[img_size];
	int s_out_size = 0;
	Sarcoder.encode(post, s_encoded, map, img_size, s_out_size);

	// write encoded data in file
	FILE* temp = fopen("encoded.bin", "w+b");
	fwrite(s_encoded, 1, s_out_size, temp);
	fclose(temp);

	// context decode
	int8_t* s_decoded = new int8_t[img_size];
	int s_foundedSize = 0;
	Sarcoder.decode(s_encoded, s_decoded, map, s_foundedSize);

	for (int i = 0; i < img_size; ++i)
	{
		if (post[i] != s_decoded[i])
		{
			int tempdas = 0;
		}
	}

	// dequant
	quant.deQuantArray(s_decoded, output, s_foundedSize);

	// invert
	wvlt.invert2d(output, input, img.GetWidth(), img.GetHeight(), 4);

	img.SetDataFromDouble(static_cast<double*>(input));

	img.WriteBmp(out);

	delete[] post;
	delete[] s_encoded;
	delete[] s_decoded;
	delete[] input;
	delete[] output;

	printf("\n%d\n", s_out_size);

	return s_out_size;
}

/////////////////////////////////////////////////////////////////////////////
int test5()
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
	SignContextArcoder SCarcoder(context, true);
	int8_t* sc_encoded = new int8_t[img_size];
	int sc_out_size = 0;
	SCarcoder.encode(post, sc_encoded, map, img_size, sc_out_size);

	// write encoded data in file
	FILE* temp = fopen("encoded.bin", "w+b");
	fwrite(sc_encoded, 1, sc_out_size, temp);
	fclose(temp);

	// context decode
	int8_t* sc_decoded = new int8_t[img_size];
	int sc_foundedSize = 0;
	SCarcoder.decode(sc_encoded, sc_decoded, map, sc_foundedSize);

	for (int i = 0; i < img_size; ++i)
	{
		if (post[i] != sc_decoded[i])
		{
			int tempdas = 0;
		}
	}

	// dequant
	quant.deQuantArray(sc_decoded, output, sc_foundedSize);

	// invert
	wvlt.invert2d(output, input, img.GetWidth(), img.GetHeight(), 4);

	img.SetDataFromDouble(static_cast<double*>(input));

	img.WriteBmp(out);

	delete[] post;
	delete[] sc_encoded;
	delete[] sc_decoded;
	delete[] input;
	delete[] output;

	printf("\n%d\n", sc_out_size);

	return sc_out_size;
}*/

void _cdecl main(int argc, char **argv)
{
	TestManager testMgr;
	char workMode = argv[1][0];

	if (workMode == 'c')
	{
		testMgr.PrintNumOfTests();
	}
	else
	{
		if ((in = fopen(argv[2], "r+b")) == NULL)
			printf("\nIncorrect input file\n");
		else if ((out = fopen(argv[3], "w+b")) == NULL)
			printf("\nIncorrect output file\n");
		else
		{
			if (argv[1][0] == '1') testMgr.DoTest(1);
			if (argv[1][0] == '2') testMgr.DoTest(2);
			if (argv[1][0] == '3') testMgr.DoTest(3);
			if (argv[1][0] == '4') testMgr.DoTest(4);
			if (argv[1][0] == '5') testMgr.DoTest(5);

			if (argv[1][0] == '0')
			{
				// let's do all the tests
				testMgr.DoTest(0, atoi(argv[4]));
			}

			fclose(in);
			fclose(out);
		}
	}
}