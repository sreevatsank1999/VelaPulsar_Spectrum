// VelaPulsar_Spectrum.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

#include <sys/types.h>
#include <math.h>

#include"../lib/fftw-3.3.5-dll64/include/fftw3.h"
//#include "../lib/mathgl-2.4.2-mingw.win64/include/mgl_cf.h"
#include "stdafx.h"

using namespace std;

enum DataMode { BIN, ASCII };

void Roundto2p(int &N);							// round(N) to closest power of 2

int Calc_Optm_BuffNum(const int64_t DataSize, const DataMode InpMode, const int64_t BuffSize, const int64_t MAX_TotSize);							// TotSize = BuffNum*BuffSize;			// BuffSize, MAX_TotSize in bytes, 

// File Tools

int64_t getFile_Size(ifstream &File);						// Return Size in bytes

int Read_Ascii_to_float(float *Ex, float *Ey, ifstream &InpFile, const int Buff_len, const int Buff_num, const int64_t MAX_TotSize = 512 * 1024 * 1024);    
int Read_Bin_to_float(float *Ex, float *Ey, ifstream &InpFile, const int Buff_len, const int Buff_num, const int64_t MAX_TotSize = 512 * 1024 * 1024);			//Buff_num == (No of Buff_len Arrays to fill)/Channel	MAX_TotSize == Max Temp Buffer Size			

// String Tools

int isValid_overwrite_YesNo(string overwrite_YesNo);
bool StringToBool_overwrite_YesNo(string overwrite_YesNo);
//int PLot();
int main()								// Spectrogram cpp
{
//	PLot(); exit(0);												// argv analysis
	string IN_PATH__, OUT_PATH__; 
	DataMode InpMode, OutMode;
	float SampleRate;		// in MHz
	float TimeRes;			// in seconds

	/*			Debug		*/
	int argc = 7;

	//argv = (char**) malloc(argc * 1024);
	const char argv[][1024] = { "VelaPulsar_Spectrum.exe", "D:\\Documents\\SWAN_RRI\\Vega_Pulsar\\ch00_B0833-45_20150612_191438_010_1", "ASCII", "D:\\Documents\\SWAN_RRI\\Vega_Pulsar\\FFT\\" , "BIN", "1.024", "0.100" };

	if (string(argv[1]) == "-h" || string(argv[1]) == "--help") {
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";
		exit(0);
	}

	cout << "argc: " << argc << endl;

	if (argc < 7) {
		cout << "Too few arguements.................\n ";
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";
		exit(-1);
	}
	else if (argc == 7) {
		IN_PATH__ = string(argv[1]);
		OUT_PATH__ = string(argv[3]);
		
		SampleRate = strtof(argv[5],NULL);
		TimeRes = strtof(argv[6], NULL);
		
		if (string(argv[2]) == string("ASCII"))		InpMode = ASCII;
		else						InpMode = BIN;				// default

		if (string(argv[4]) == string("ASCII"))		OutMode = ASCII;
		else						OutMode = BIN;				// default
		
																						// Print Read data
		cout << "IN_PATH__: " << IN_PATH__ << "\t" << "InMode: " << (InpMode ? "BIN" : "ASCII") << "\n";
		cout << "OUT_PATH__: " << OUT_PATH__ << "\t" << "OutMode: " << (OutMode ? "BIN" : "ASCII") << "\n";
		cout << "Sample Rate: " << SampleRate << " Mhz\n";
		cout << "TimeRes: " << TimeRes << " Seconds\n"; 
		cout << endl;


	}
	else if (argc > 7) {

		cout << "Too many arguements.................\n ";
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";
		exit(1);
	}
	
//	ios_base::sync_with_stdio(false);					// Fast I\O C ++
//	cin.tie(NULL);


	ifstream DataIn; // DataIn.rdbuf()->pubsetbuf(nullptr, 0); DataIn.tie(NULL);

	DataIn.open(IN_PATH__, ios::in | (InpMode ? 0 : ios::binary));											// Opening Data File

	if (!DataIn.is_open()) {
		cout << " Error Opening the file, please recheck the Input Filepath and Try Again \n";
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";
		
		DataIn.close();
		exit(2);
	}
	DataIn.seekg(ios::beg);

	const int64_t DataIn_Size = getFile_Size(DataIn);

	const string FILE_NAME__ = IN_PATH__.substr(IN_PATH__.find_last_of('\\'));

	
	ifstream OutFileTemp;
	
	OutFileTemp.open(OUT_PATH__ + FILE_NAME__ + "_Ex_" + (OutMode ? ".bin" : ".txt"), ios::in | (OutMode ? 0 : ios::binary));

	if (OutFileTemp.is_open()) {
		cout << " Output file already exists, Do you want to overwrite? (y/n): ";
		string YesNo;			 cin >> YesNo;

		if (StringToBool_overwrite_YesNo(YesNo) == true) {

			OutFileTemp.close();

			const int result = remove((OUT_PATH__ + FILE_NAME__ + "_Ex_" + (OutMode ? ".bin" : ".txt")).c_str());				// remove the existing file
			if (result != 0) { cout << "Error deleting file \n"; 	exit(3); }
		}
		else
		{
			cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";

			OutFileTemp.close();
			exit(3);
		}
	}
	OutFileTemp.close();

	OutFileTemp.open(OUT_PATH__ + FILE_NAME__ + "_Ey_" + (OutMode ? ".bin" : ".txt"), ios::in | (OutMode ? 0 : ios::binary));

	if (OutFileTemp.is_open()) {
		cout << " Output file already exists, Do you want to overwrite? (y/n): ";
		string YesNo;			 cin >> YesNo;

		if (StringToBool_overwrite_YesNo(YesNo) == true) { ; }					// No need to remove the file here as Later anyhow the output file is opened in truc mode which deletes the content of the file
		else
		{
			cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";

			OutFileTemp.close();
			exit(3);
		}
	}
	OutFileTemp.close(); 

	ofstream Ex_FFT_Out;	Ex_FFT_Out.rdbuf()->pubsetbuf(nullptr, 0);   Ex_FFT_Out.tie(NULL);
    ofstream Ey_FFT_Out;	Ey_FFT_Out.rdbuf()->pubsetbuf(nullptr, 0);	 Ey_FFT_Out.tie(NULL);
					
/*																							//Todo: check folder is empty     
	if (!Ex_FFT_Out.is_open()) {
		cout << " Error writting to file, please recheck the Output Filepath and Try Again \n";
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";

		Ex_FFT_Out.close();
		exit(2);
	}
	Ex_FFT_Out.seekp(ios::beg);

*/

	int FFT_len = (int)(SampleRate*1000000)* TimeRes;				// Length of Input to FFT, incase of r2c FFT No of FFT bins = FFT_len/2 + 1 ;

	Roundto2p(FFT_len);

	TimeRes = (FFT_len / (SampleRate * 1000000));		// in seconds

	cout << "Time_Resolution set to " << TimeRes << "(in seconds)" << endl;


	const int64_t MAX_Avail_RAM = (int64_t)2 * 1024 * 1024 * 1024;

	int N_FFTs = Calc_Optm_BuffNum(DataIn_Size, InpMode, (FFT_len / 2 + 1) * sizeof(fftwf_complex), MAX_Avail_RAM);
	N_FFTs = (N_FFTs % 2) ? N_FFTs : (N_FFTs - 1);																				// Assuming N_FFT != 0 or 1, N_FFT >=2,		Todo: Handle 0 & 1 Cases, Add Multi-channel Capability
	 
	fftwf_complex *FFT_Ex; FFT_Ex = (fftwf_complex *)fftwf_malloc((N_FFTs/2)*(FFT_len/2 +1)* sizeof(fftwf_complex));
	fftwf_complex *FFT_Ey; FFT_Ey = (fftwf_complex *)fftwf_malloc((N_FFTs/2)*(FFT_len/2 +1)* sizeof(fftwf_complex));
																															//FFTW Advacned Interface 
	fftwf_plan plan_x = fftwf_plan_many_dft_r2c(1, &FFT_len, N_FFTs, (float *)FFT_Ex, &FFT_len, 1, (FFT_len / 2 + 1) * sizeof(fftwf_complex), FFT_Ex, &FFT_len, 1, (FFT_len / 2 + 1) * sizeof(fftwf_complex), FFTW_PATIENT);
	fftwf_plan plan_y = fftwf_plan_many_dft_r2c(1, &FFT_len, N_FFTs, (float *)FFT_Ey, &FFT_len, 1, (FFT_len / 2 + 1) * sizeof(fftwf_complex), FFT_Ey, &FFT_len, 1, (FFT_len / 2 + 1) * sizeof(fftwf_complex), FFTW_PATIENT);

		Ex_FFT_Out.open(OUT_PATH__ + FILE_NAME__ + "_Ex_" + (OutMode ? ".bin" : ".txt"), ios::out | ios::trunc | (OutMode ? (int)0 : ios::binary));
		Ex_FFT_Out.seekp(ios::beg);

		Ey_FFT_Out.open(OUT_PATH__ + FILE_NAME__ + "_Ey_" + (OutMode ? ".bin" : ".txt"), ios::out | ios::trunc | (OutMode ? (int)0 : ios::binary));
		Ey_FFT_Out.seekp(ios::beg);

		while (!DataIn.eof())
		{	
			int N_FFTs_loaded;

			if(InpMode == ASCII)		N_FFTs_loaded = Read_Ascii_to_float((float *)FFT_Ex, (float *)FFT_Ey, DataIn, FFT_len, (N_FFTs / 2));
			else						N_FFTs_loaded = Read_Bin_to_float((float *)FFT_Ex, (float *)FFT_Ey, DataIn, FFT_len, (N_FFTs / 2));
				
				fftwf_execute(plan_x);
				fftwf_execute(plan_y);

				if (OutMode == ASCII)		for (int j = 0; j < N_FFTs_loaded * FFT_len/2 + 1; j++)			Ex_FFT_Out << FFT_Ex[j][0] << " " << FFT_Ex[j][1] << "\n";
				else						Ex_FFT_Out.write((char*)FFT_Ex, N_FFTs_loaded * (2 * sizeof(float)*(FFT_len / 2 + 1)));

				if (OutMode == ASCII)		for (int j = 0; j < N_FFTs_loaded * FFT_len / 2 + 1; j++)			 Ey_FFT_Out << FFT_Ey[j][0] << " " << FFT_Ey[j][1] << "\n";
				else						Ey_FFT_Out.write((char*)FFT_Ey, N_FFTs_loaded * (2 * sizeof(float)*(FFT_len / 2 + 1)));
		}
	
	fftwf_destroy_plan(plan_x);
	fftwf_destroy_plan(plan_y);

	fftwf_free(FFT_Ex); fftwf_free(FFT_Ey);

	return 0;
}

void Roundto2p(int &N) {						// round(N) to closest power of 2

	int p = 0;
	int temp = N;

	while (temp > 1)
	{
		temp = temp >> 1;
		p++;
	}

	if (abs(N - (1 << p)) > abs(N - (1 << (p + 1)))) 					N = 1 << p;
	else																N = 1 << (p + 1);

}

int Calc_Optm_BuffNum(const int64_t DataSize, const DataMode InpMode, const int64_t BuffSize, const int64_t MAX_TotSize) {							// TotSize = BuffNum*BuffSize;			// BuffSize, MAX_TotSize in bytes, 

	const int factor = InpMode ? 1 : 4;					// ASCII, BIN File factor		// Todo: Improve, Make it More General


	if ((DataSize / factor) * (int64_t)sizeof(float) > MAX_TotSize)				return (int)MAX_TotSize / BuffSize;						// Todo: Allocate such that DataSize is close to an Intergral Multiple of TotSize (going to be allocated)
	else																return (int)(DataSize / factor) / BuffSize;

}

// File Tools

int64_t getFile_Size(ifstream &File) {						// Return Size in bytes

	streampos pos_ini = File.tellg();

	File.seekg(ios::beg);
	streampos pos_beg = File.tellg();

	File.seekg(ios::end);
	streampos pos_end = File.tellg();

	File.seekg(pos_ini);

	return (int64_t)(pos_end - pos_beg);			//  Size in Bytes
}

int Read_Ascii_to_float(float *Ex, float *Ey, ifstream &InpFile, const int Buff_len, const int Buff_num, const int64_t MAX_TotSize /* = 512 * 1024 * 1024 */) {

	const int64_t Size = min<int64_t>(2 * Buff_len*Buff_num * 4, MAX_TotSize);							// Todo: Use Statistical Model to predice The Required Buffersize to Read

	char *temp_Buff; temp_Buff = (char *)malloc(Size);								//Buffer

	int64_t i = 0;

	while ((i < Buff_len*Buff_num) && (!InpFile.eof())) {

		InpFile.read(temp_Buff, min<int64_t>(2 * (Buff_len*Buff_num - i) * 4, Size));					// Reads predicted required amount of data, clamped by Max Buffer Size  **Improve with Statistical Model
		streamsize ChRead_cnt = InpFile.gcount();
		int64_t _parsed = 0;

		for (int j = 0; true; j++) {		// Checking for incomoplete Data, seekg(till previous complete value);

			if ((temp_Buff[Size - 1 - j] == '\n') || (temp_Buff[Size - 1 - j] == ' ')) {
				ChRead_cnt += -j;
				InpFile.seekg(-j, ios::cur);
				break;
			}

			temp_Buff[Size - 1 - j] = '\0';
		}

		while (_parsed < ChRead_cnt) {

			size_t k = 0;
			Ex[i] = (float)stol(&temp_Buff[_parsed], &k);
			_parsed += k;

			Ey[i] = (float)stol(&temp_Buff[_parsed], &k);
			_parsed += k;
			i++;

			if (i == Buff_len)		i += 2;
		}
	}
	for (int j = 0; j < (i%Buff_len); j++)			 Ex[i] = Ey[i] = 0.0f;

	return (i / 2 % Buff_len) ? (int)(i / 2 / Buff_len) : (int)(i / 2 / Buff_len) + 1;				// returns Number of Filled Arrays 

}
int Read_Bin_to_float(float *Ex, float *Ey, ifstream &InpFile, const int Buff_len, const int Buff_num, const int64_t MAX_TotSize /* = 512 * 1024 * 1024 */) {			//Buff_num == (No of Buff_len Arrays to fill)/Channel				
																																								// Todo: Add Multibyte int Read, float and double Read
	const int64_t Size = min<int64_t>(2 * Buff_len*Buff_num * 1, MAX_TotSize);

	char *temp_Buff; temp_Buff = (char *)malloc(Size);								//Buffer

	int64_t N = 0;

	while ((N < Buff_len*Buff_num) && (!InpFile.eof())) {

		InpFile.read(temp_Buff, min<int64_t>(2 * (Buff_len*Buff_num - N) * 1, Size));			// Reads only required amount of data, clamped by Max Buffer Size
		int64_t k = InpFile.gcount();

		for (int j = 0; j < k / 2; j ++) {									// Converts and loads fft buffer accordingly (takes care of padding), Keeps track of number of read bytes so as to not convert more than then read & not write more than the data exists.

			Ex[N+j] = (float)temp_Buff[2 * (N+j)];
			Ey[N+j] = (float)temp_Buff[2 * (N+j) + 1];

			if (j == Buff_len)		j += 2;
		}
		N += k / 2;
	}
	for (int j = 0; j < ((N / 2) % Buff_len); j++)				Ex[N] = Ey[N] = 0.0f;

	return (N / 2 % Buff_len) ? (int)(N / 2 / Buff_len) : (int)(N / 2 / Buff_len) + 1;				// returns Number of Filled Arrays 
}

// String Tools

int isValid_overwrite_YesNo(string overwrite_YesNo) {

	const vector<string> Vaild_Yes = { "true", "yes", "y" };
	const vector<string> Vaild_No = { "false", "no", "n" };

	std::transform(overwrite_YesNo.begin(), overwrite_YesNo.end(), overwrite_YesNo.begin(), ::tolower);

	for (int i = 0; i < Vaild_Yes.size(); i++)				if (overwrite_YesNo == Vaild_Yes[i])			return (int)true;
	for (int i = 0; i < Vaild_No.size(); i++)				if (overwrite_YesNo == Vaild_No[i])				return (int)false;

	return -1;
}
bool StringToBool_overwrite_YesNo(string overwrite_YesNo) {

	int Validoverwrite_YesNo = isValid_overwrite_YesNo(overwrite_YesNo);

	if (Validoverwrite_YesNo != -1)			return (Validoverwrite_YesNo != false);
	else	printf("Error: StringToBool_overwrite_YesNo() couldn't convert to bool \n");
}

/*
int PLot() {


	mglData dat(30, 40); // data to for plotting 
	for(long i=0;i<30;i++) for(long j=0;j<40;j++) dat.a[i+30*j] = 1/(1+(i-15)*(i-15)/225.+(j-20)*(j-20)/400.);

	mglGraph gr; // class for plot drawing 
	gr.Rotate(50,60); // rotate axis 
	gr.Light(true); 

	gr.Surf(dat); // plot surface Basically plot is d

	return 0;



	HMGL gr = mgl_create_graph(600, 400);
	mgl_fplot(gr, "sin(pi*x)", "", ""); 
	mgl_write_frame(gr, "test.png", ""); 
	mgl_delete_graph(gr);

	return 0;
} 
*/