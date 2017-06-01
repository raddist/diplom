// 2 Simplest & slowest version of arithmetic codec (almost demo version)
#include "utils.h"

#pragma warning(disable: 4996)
FILE *in, *out;

#define WAVE_STEPS	4
#define ETALON_Q	20		// my research was provided for this value



class TestManager
{
public:
	TestManager()
	{}

	void PrintNumOfTests()
	{
		printf("\n%d\n", 5);

	}

	void processLimits(double& l1, double& l2, double& l3, double i_q)
	{
		double shift = ETALON_Q / i_q;
		l1 *= shift;
		l2 *= shift;
		l3 *= shift;
	}

	void DoTest(int i_test, double i_q = 1.0,
				double l1 = 0.1, double l2 = 1.1, double l3 = 3.5,
				double s1 = -1.5, double s2 = 1.5)
	{
		//processLimits(l1, l2, l3, i_q);
		switch (i_test)
		{
		case 0:
		{
			//do all tests
			test0(i_q, WAVE_STEPS, l1, l2, l3, s1, s2);
			break;
		}
		case 1:
		{
			test1(i_q, WAVE_STEPS);
			break;
		}
		case 2:
		{
			test2(i_q, WAVE_STEPS);
			break;
		}
		case 3:
		{
			test3(i_q, WAVE_STEPS, l1, l2, l3);
			break;
		}
		case 4:
		{
			test4(i_q, WAVE_STEPS);
			break;
		}
		case 5:
		{
			test5(i_q, WAVE_STEPS, l1, l2, l3, s1, s2);
			break;
		}
		}
	}


	~TestManager()
	{}

private:

	// @brief compare all arcoders
	void test0(double i_q, int steps, double l1, double l2, double l3,
				double s1, double s2)
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();
		SubbandMap map(img.GetWidth(), img.GetHeight(), 4);

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilizationByMap(transformed, quanted, img_size, i_q, map);

		// preapare for encode/decode
		Limits limits;
		limits.push_back(l1);
		limits.push_back(l2);
		limits.push_back(l3);

		Limits signLimits;
		signLimits.push_back(s1);
		signLimits.push_back(s2);

		double ctemp[] = {	0.2, 0.4, 0.0 ,  
							0.0, 0.0, 0.0 ,  
							0.0, 0.0, 0.0 };
		double ctemp1[] = { 0.4, 0.2, 0.4 ,  
							0.1, 0.0, 0.0 ,  
							0.0, 0.0, 0.0 };
		Context3x3 context;
		context.maskD = ctemp1;
		context.maskH = ctemp;
		context.maskV = ctemp;

		double ctempH[] = { 0.2, 0.4, 0.0 ,
							0.4, 0.0, 0.0 ,
							0.0, 0.0, 0.0 };
		double ctempV[] = { 0.2, 0.4, 0.0 ,
							0.4, 0.0, 0.0 ,
							0.0, 0.0, 0.0 };
		double ctempD[] = { 0.4, 0.2, 0.4 ,
							0.0, 0.0, 0.0 ,
							0.0, 0.0, 0.0 };
		Context3x3 context_2;
		context_2.maskD = ctempD;
		context_2.maskH = ctempH;
		context_2.maskV = ctempV;
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
		ContextArcoder Carcoder(qStruct, context, limits);
		int8_t* encoded_3 = new int8_t[img_size];
		int out_size_3 = 0;
		Carcoder.encode(quanted, encoded_3, map, img_size, out_size_3);

		// decode 3
		int* decoded_3 = new int[img_size];
		int foundedSize_3 = 0;
		Carcoder.decode(encoded_3, decoded_3, map, foundedSize_3);
		///////////////////////////////////////////////////////////////////////////////////////

		// encode 4
		SignContextArcoder sCarcoder(qStruct, context_2, limits, signLimits);
		int8_t* encoded_4 = new int8_t[img_size];
		int out_size_4 = 0;
		sCarcoder.encode(quanted, encoded_4, map, img_size, out_size_4);

		// decode 4
		int* decoded_4 = new int[img_size];
		int foundedSize_4 = 0;
		sCarcoder.decode(encoded_4, decoded_4, map, foundedSize_4);
		///////////////////////////////////////////////////////////////////////////////////////

		// encode 5
		SignContextArcoder Sarcoder(qStruct, context_2, limits, signLimits, true);
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
		compareArraysThroughMap(quanted, decoded_4, img_size, map);

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

		// write model data
		FILE* model_file = fopen("model.bin", "w+b");
		Sarcoder.exportModelInformation(model_file);
		fclose(model_file);

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
	void test1(double i_q, int steps)
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), steps);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilization(transformed, quanted, img_size, i_q);

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
		deQuantilization(decoded, deQuanted, img_size, i_q);

		// using wavelet invert
		double* inverted = new double[img_size];
		waveletInvert2D(deQuanted, inverted, img.GetWidth(), img.GetHeight(), steps);

		img.SetDataFromDouble(static_cast<double*>(inverted));
		img.WriteBmp(out);

		// tests

		// write quanted data
		FILE* quanted_file = fopen("quanted.bin", "w+b");
		int* temp = new int[2];
		temp[0] = img.GetHeight();
		temp[1] = img.GetWidth();
		fwrite(temp, sizeof(*temp), 2, quanted_file);
		fwrite(quanted, sizeof(*quanted), img_size, quanted_file);
		fclose(quanted_file);


		// realloc
		delete[] transformed;
		delete[] quanted;
		delete[] encoded;
		delete[] decoded;
		delete[] deQuanted;
		delete[] inverted;
	}

	// @brief test mapped encode/decode with memory
	void test2(double i_q, int steps)
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilization(transformed, quanted, img_size, i_q);

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
		deQuantilization(decoded, deQuanted, img_size, i_q);

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
	void test3(double i_q, int steps, double l1, double l2, double l3)
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();
		SubbandMap map(img.GetWidth(), img.GetHeight(), 4);

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilizationByMap(transformed, quanted, img_size, i_q, map);

		// encode
		double ctemp[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		double ctemp1[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		Context3x3 context;
		context.maskD = ctemp;
		context.maskH = ctemp1;
		context.maskV = ctemp1;
		Limits limits;
		limits.push_back(l1);
		limits.push_back(l2);
		limits.push_back(l3);
		ContextArcoder Carcoder(qStruct, context, limits);
		int8_t* encoded = new int8_t[img_size];
		int c_out_size = 0;
		Carcoder.encode(quanted, encoded, map, img_size, c_out_size);

		// decode
		int* decoded = new int[img_size];
		int foundedSize = 0;
		Carcoder.decode(encoded, decoded, map, foundedSize);

		// quantilization
		double* deQuanted = new double[img_size];
		deQuantilization(decoded, deQuanted, img_size, i_q);

		// using wavelet invert
		double* inverted = new double[img_size];
		waveletInvert2D(deQuanted, inverted, img.GetWidth(), img.GetHeight(), 4);

		img.SetDataFromDouble(static_cast<double*>(inverted));
		img.WriteBmp(out);

		// write model data
		FILE* model_file = fopen("model.bin", "w+b");
		Carcoder.exportModelInformation(model_file);
		fclose(model_file);

		// tests
		compareArrays(quanted, decoded, img_size);

		// realloc
		delete[] transformed;
		delete[] quanted;
		delete[] encoded;
		delete[] decoded;
		delete[] deQuanted;
		delete[] inverted;

		printf("%d", c_out_size);
	}

	// @brief test context encode/decode with sign
	void test4(double i_q, int steps)
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();
		SubbandMap map(img.GetWidth(), img.GetHeight(), 4);

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilizationByMap(transformed, quanted, img_size, i_q, map);

		// encode
		double ctempH[] = { 0.2, 0.0, 0.0 ,  0.4, 0.0, 0.0 ,  0.2, 0.0, 0.0 };
		double ctempV[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		double ctempD[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		Context3x3 context;
		context.maskD = ctempD;
		context.maskH = ctempH;
		context.maskV = ctempV;
		SignContextArcoder Carcoder(qStruct, context);
		int8_t* encoded = new int8_t[img_size];
		int c_out_size = 0;
		Carcoder.encode(quanted, encoded, map, img_size, c_out_size);

		// decode
		int* decoded = new int[img_size];
		int foundedSize = 0;
		Carcoder.decode(encoded, decoded, map, foundedSize);

		// quantilization
		double* deQuanted = new double[img_size];
		deQuantilization(decoded, deQuanted, img_size, i_q);

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
	void test5(double i_q, int steps, double l1, double l2, double l3,
				double s1, double s2)
	{
		BmpImage img(in, 1);
		int img_size = img.GetWidth() * img.GetHeight();
		SubbandMap map(img.GetWidth(), img.GetHeight(), 4);

		// using wavelet
		double* transformed = new double[img_size];
		waveletTransform2D(img.GetDoubleData(), transformed, img.GetWidth(), img.GetHeight(), 4);

		// quantilization
		int* quanted = new int[img_size];
		qMinCap qStruct = quantilizationByMap(transformed, quanted, img_size, i_q, map);

		// prepare limits
		Limits limits;
		limits.push_back(l1);
		limits.push_back(l2);
		limits.push_back(l3);

		Limits signLimits;
		signLimits.push_back(s1);
		signLimits.push_back(s2);

		// encode
		double ctempH[] = { 0.2, 0.0, 0.0 ,  0.4, 0.0, 0.0 ,  0.2, 0.0, 0.0 };
		double ctempV[] = { 0.4, 0.2, 0.4 ,  0.0, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		double ctempD[] = { 0.2, 0.4, 0.0 ,  0.4, 0.0, 0.0 ,  0.0, 0.0, 0.0 };
		Context3x3 context;
		context.maskD = ctempD;
		context.maskH = ctempH;
		context.maskV = ctempV;

		SignContextArcoder Carcoder(qStruct, context, limits, signLimits, true);
		int8_t* encoded = new int8_t[img_size];
		int c_out_size = 0;
		Carcoder.encode(quanted, encoded, map, img_size, c_out_size);

		// decode
		int* decoded = new int[img_size];
		int foundedSize = 0;
		Carcoder.decode(encoded, decoded, map, foundedSize);

		// quantilization
		double* deQuanted = new double[img_size];
		deQuantilization(decoded, deQuanted, img_size, i_q);

		// using wavelet invert
		double* inverted = new double[img_size];
		waveletInvert2D(deQuanted, inverted, img.GetWidth(), img.GetHeight(), 4);

		img.SetDataFromDouble(static_cast<double*>(inverted));
		img.WriteBmp(out);

		// write model data
		FILE* model_file = fopen("model.bin", "w+b");
		Carcoder.exportModelInformation(model_file);
		fclose(model_file);

		// realloc
		delete[] transformed;
		delete[] quanted;
		delete[] encoded;
		delete[] decoded;
		delete[] deQuanted;
		delete[] inverted;

		printf("%d", c_out_size);
	}
};

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
			int testNumber = argv[1][0] - '0';
			if (testNumber > 0 && testNumber <= 5)
			{
				if (argc > 8)
				{
					testMgr.DoTest(testNumber, atof(argv[4]),
						atof(argv[5]), atof(argv[6]), atof(argv[7]),
						atof(argv[8]), atof(argv[9]));
				}
				else if (argc > 5)
				{
					testMgr.DoTest(testNumber, atof(argv[4]),
								   atof(argv[5]), atof(argv[6]), atof(argv[7]));
				}
				else
				{
					testMgr.DoTest(testNumber, atof(argv[4]));
				}
			}

			if (argv[1][0] == '0')
			{
				// let's do all the tests
				testMgr.DoTest(0, atof(argv[4]));
			}

			fclose(in);
			fclose(out);
		}
	}
}