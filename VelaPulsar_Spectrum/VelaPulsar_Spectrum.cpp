// VelaPulsar_Spectrum.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

#include <sys/types.h>
//#include <thread>
#include <sys/stat.h>
#include <math.h>

#include"../lib/fftw-3.3.5-dll64/include/fftw3.h"
//#include "../lib/mathgl-2.4.2-mingw.win64/include/mgl_cf.h"
#include "stdafx.h"

using namespace std;

enum DataMode { BIN, ASCII };

void Roundto2p(int &N);							// round(N) to closest power of 2

int Calc_Optm_FFTNum(const int64_t DataSize, const DataMode InpMode, const int64_t BuffSize, const int64_t MAX_TotSize);							// TotSize = BuffNum*BuffSize;			// BuffSize, MAX_TotSize in bytes, 
int64_t Calc_Optm_ASCII_Writebuff_size(const int N, const int64_t MAX_TotSize = 512 * 1024 * 1024);								// Calculates Write Buff size in the case of Outmode == ASCII

// File Tools

int64_t getFile_Size(const string &_FileName); 						// Return Size in bytes

int Read_Ascii_to_float(float *Ex, float *Ey, ifstream &InpFile, const int Buff_len, const int Buff_num, const int64_t MAX_TotSize = 512 * 1024 * 1024);    
int Read_Bin_to_float(float *Ex, float *Ey, ifstream &InpFile, const int Buff_len, const int Buff_num, const int64_t MAX_TotSize = 512 * 1024 * 1024);			//Buff_num == (No of Buff_len Arrays to fill)/Channel	MAX_TotSize == Max Temp Buffer Size			

// String Tools

int isValid_overwrite_YesNo(string overwrite_YesNo);
bool StringToBool_overwrite_YesNo(string overwrite_YesNo);

int main(int argc, char *argv[])								// Spectrogram cpp
{
//	PLot(); exit(0);												// argv analysis
	string IN_PATH__, OUT_PATH__; 
	DataMode InpMode, OutMode;
	float SampleRate;		// in MHz
	float TimeRes;			// in seconds

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
		cout << "IN_PATH__: " << IN_PATH__ << "\t" << "InMode: " << (InpMode ? "ASCII" : "BIN") << "\n";
		cout << "OUT_PATH__: " << OUT_PATH__ << "\t" << "OutMode: " << (OutMode ? "ASCII" : "BIN") << "\n";
		cout << "Sample Rate: " << SampleRate << " Mhz\n";
		cout << "TimeRes: " << TimeRes << " Seconds\n"; 
		cout << endl;


	}
	else if (argc > 7) {

		cout << "Too many arguements.................\n ";
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";
		exit(1);
	}
	
	ios_base::sync_with_stdio(false);					// Fast I\O C ++
	cin.tie(NULL);


	ifstream DataIn;  DataIn.rdbuf()->pubsetbuf(nullptr, 0); DataIn.tie(NULL);

	DataIn.open(IN_PATH__, ios::in | (InpMode ? 0 : ios::binary));											// Opening Data File

	if (!DataIn.is_open()) {
		cout << " Error Opening the file, please recheck the Input Filepath and Try Again \n";
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";
		
		DataIn.close();
		exit(2);
	}
	DataIn.seekg(ios::beg);

	const int64_t DataIn_Size = getFile_Size(IN_PATH__);

	const string FILE_NAME__ = IN_PATH__.substr(IN_PATH__.find_last_of('\\'));

	
	ifstream OutFileTemp;
	
	OutFileTemp.open(OUT_PATH__ + FILE_NAME__ + "_Ex_" + (OutMode ? ".txt" : ".bin"), ios::in | (OutMode ? 0 : ios::binary));

	if (OutFileTemp.is_open()) {
		cout << "Ex Channel Output file already exists, Do you want to overwrite? (y/n): ";
		string YesNo;			 cin >> YesNo;

		if (StringToBool_overwrite_YesNo(YesNo) == true) {

			OutFileTemp.close();

			const int result = remove((OUT_PATH__ + FILE_NAME__ + "_Ex_" + (OutMode ? ".txt" : ".bin")).c_str());				// remove the existing file
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

	OutFileTemp.open(OUT_PATH__ + FILE_NAME__ + "_Ey_" + (OutMode ? ".txt" : ".bin"), ios::in | (OutMode ? 0 : ios::binary));

	if (OutFileTemp.is_open()) {
		cout << "Ey Channel Output file already exists, Do you want to overwrite? (y/n): ";
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

	int N_FFTs = Calc_Optm_FFTNum(DataIn_Size, InpMode, (FFT_len / 2 + 1) * sizeof(fftwf_complex), MAX_Avail_RAM);
	N_FFTs = (N_FFTs % 2) ? (N_FFTs - 1) : N_FFTs;																				// Assuming N_FFT != 0 or 1, N_FFT >=2,		Todo: Handle 0 & 1 Cases, Add Multi-channel Capability
	 
//	fftw_init_threads();				// Multi-threading

	fftwf_complex *FFT_Ex; FFT_Ex = (fftwf_complex *)fftwf_malloc((N_FFTs/2)*(FFT_len + 2) * sizeof(float));
	fftwf_complex *FFT_Ey; FFT_Ey = (fftwf_complex *)fftwf_malloc((N_FFTs/2)*(FFT_len + 2) * sizeof(float));

//	fftw_plan_with_nthreads(thread::hardware_concurrency());			// Multi-threading 

	const string Wisdom_PATH = "Wisdom" + '\\' + FILE_NAME__ + ".WISDOM";
	fftw_import_wisdom_from_filename(Wisdom_PATH.c_str());				// Importing Wisdom
																								//FFTW Advacned Interface  Todo: Save Wisdom !! Save Time
	fftwf_plan plan_x = fftwf_plan_many_dft_r2c(1, &FFT_len, N_FFTs / 2, (float *)FFT_Ex, &FFT_len, 1, (FFT_len + 2), (fftwf_complex *)FFT_Ex, &FFT_len, 1, (FFT_len / 2 + 1), FFTW_PATIENT);
	fftwf_plan plan_y = fftwf_plan_many_dft_r2c(1, &FFT_len, N_FFTs / 2, (float *)FFT_Ey, &FFT_len, 1, (FFT_len + 2), (fftwf_complex *)FFT_Ey, &FFT_len, 1, (FFT_len / 2 + 1), FFTW_PATIENT);

	fftw_export_wisdom_to_filename(Wisdom_PATH.c_str());				// Exporting Wisdom

	ofstream Ex_FFT_Out;
	if (OutMode == ASCII) {
		const int64_t Optm_Write_buff_size = Calc_Optm_ASCII_Writebuff_size((N_FFTs / 2)*(FFT_len + 2));

		char *OutWrite_buff = (char *)malloc(Optm_Write_buff_size);
		Ex_FFT_Out.rdbuf()->pubsetbuf(OutWrite_buff, Optm_Write_buff_size);
	}
	else					Ex_FFT_Out.rdbuf()->pubsetbuf(nullptr, 0);
	Ex_FFT_Out.tie(NULL);

	Ex_FFT_Out.open(OUT_PATH__ + FILE_NAME__ + "_Ex_" + (OutMode ? ".txt" : ".bin"), ios::out | ios::trunc | (OutMode ? (int)0 : ios::binary));
	Ex_FFT_Out.seekp(ios::beg);


	ofstream Ey_FFT_Out;
	if (OutMode == ASCII) {
		const int64_t Optm_Write_buff_size = Calc_Optm_ASCII_Writebuff_size((N_FFTs / 2)*(FFT_len + 2));

		char *OutWrite_buff = (char *)malloc(Optm_Write_buff_size);
		Ey_FFT_Out.rdbuf()->pubsetbuf(OutWrite_buff, Optm_Write_buff_size);
	}
	else					Ey_FFT_Out.rdbuf()->pubsetbuf(nullptr, 0);
	Ey_FFT_Out.tie(NULL);

		Ey_FFT_Out.open(OUT_PATH__ + FILE_NAME__ + "_Ey_" + (OutMode ? ".txt" : ".bin"), ios::out | ios::trunc | (OutMode ? (int)0 : ios::binary));
		Ey_FFT_Out.seekp(ios::beg);

		while (!DataIn.eof())
		{	
			int N_FFTs_loaded;

			if(InpMode == ASCII)		N_FFTs_loaded = Read_Ascii_to_float((float *)FFT_Ex, (float *)FFT_Ey, DataIn, FFT_len, (N_FFTs / 2));	//No of FFT's Loaded into memory /Data Channel
			else						N_FFTs_loaded = Read_Bin_to_float((float *)FFT_Ex, (float *)FFT_Ey, DataIn, FFT_len, (N_FFTs / 2));
				
				fftwf_execute(plan_x);
				fftwf_execute(plan_y);

				if (OutMode == ASCII)		for (int j = 0; j < N_FFTs_loaded * (FFT_len/2 + 1); j++)			Ex_FFT_Out << FFT_Ex[j][0] << " " << FFT_Ex[j][1] << "\n";
				else																							Ex_FFT_Out.write((char*)FFT_Ex, N_FFTs_loaded * (sizeof(fftwf_complex)*(FFT_len / 2 + 1)));

				if (OutMode == ASCII)		for (int j = 0; j < N_FFTs_loaded * (FFT_len/ 2 + 1); j++)		    Ey_FFT_Out << FFT_Ey[j][0] << " " << FFT_Ey[j][1] << "\n";
				else																							Ey_FFT_Out.write((char*)FFT_Ey, N_FFTs_loaded * (sizeof(fftwf_complex)*(FFT_len / 2 + 1)));
		}
	
	fftwf_destroy_plan(plan_x);
	fftwf_destroy_plan(plan_y);

//	fftw_cleanup_threads();				// Multi-threading

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

int Calc_Optm_FFTNum(const int64_t DataSize, const DataMode InpMode, const int64_t BuffSize, const int64_t MAX_TotSize) {							// TotSize = BuffNum*BuffSize;			// BuffSize, MAX_TotSize in bytes, 

	const int factor = InpMode ? 4 : 1;					// ASCII, BIN File factor		// Todo: Improve, Make it More General


	if ((DataSize / factor) * (int64_t)sizeof(float) > MAX_TotSize)				return (int)(MAX_TotSize / BuffSize);						// Todo: Allocate such that DataSize is close to an Intergral Multiple of TotSize (going to be allocated)
	else																return (int)((DataSize / factor) / BuffSize);

}
int64_t Calc_Optm_ASCII_Writebuff_size(const int N, const int64_t MAX_TotSize /* = 512 * 1024 * 1024 */ ) {
	return min<int64_t>((int64_t)N * 8, MAX_TotSize);										// Assuming 1float requires 8 chars to store as ASCII
}
// File Tools

int64_t getFile_Size(const string &_FileName) {						// Return Size in bytes

	struct _stat64 Fstat;			_stat64(_FileName.c_str(), &Fstat);

	return (int64_t)Fstat.st_size;			//  Size in Bytes
}

int Read_Ascii_to_float(float *Ex, float *Ey, ifstream &InpFile, const int Buff_len, const int Buff_num, const int64_t MAX_TotSize /* = 512 * 1024 * 1024 */) {

	const int64_t Size = min<int64_t>(2 * Buff_len*Buff_num * 4, MAX_TotSize);							// Todo: Use Statistical Model to predice The Required Buffersize to Read

	char * const temp_Buff = (char *)malloc(Size + 1);	temp_Buff[Size] = '\0';			//Buffer

	register int  i = 0; 	register int  k = 0;

	while ((i < Buff_len*Buff_num) && (!InpFile.eof())) {

		InpFile.read(temp_Buff, min<int64_t>(2 * (Buff_len*Buff_num - i) * 4, Size));					// Reads predicted required amount of data, clamped by Max Buffer Size  **Improve with Statistical Model
		streamsize ChRead_cnt = InpFile.gcount();
		
/*		printf("Read  \t %d \n", ChRead_cnt);											// Debugging 
		if (InpFile.eof()) printf("EndofFile  \t %d \n", ChRead_cnt);				*/

		register char *_parsedptr = temp_Buff;											// Directly Dealing with Pointers rather than Count Variable for Optimisation
		char * const _parsedend_ptr = temp_Buff + ChRead_cnt;

		for (register int j = 0; true; j++) {		// Checking for incomplete Data, seekg(till previous complete value);

			if ((temp_Buff[ChRead_cnt - 1 - j] == '\n') || (temp_Buff[ChRead_cnt - 1 - j] == ' ')) {
				ChRead_cnt += -j;
				if(j > 0 )  InpFile.seekg(-j, ios::cur);
				break;
			}

			temp_Buff[ChRead_cnt - 1 - j] = '\0';
		}
		
		while ((i < Buff_len*Buff_num)&&(_parsedptr + 1 < _parsedend_ptr)) {

			Ex[i + 2*k] = (float)strtol(_parsedptr, &_parsedptr, 10);					// Using strtol for Optimisation purposes, NOTE: stol is very very slow when used with char*, as it meant to only deal with strings
			Ey[i + 2*k] = (float)strtol(_parsedptr, &_parsedptr, 10);
			i++;

			if (i%Buff_len == 0)		k++;
		}

		if ((_parsedptr + 1 < _parsedend_ptr))	InpFile.seekg(_parsedptr - _parsedend_ptr, ios::cur);				// Seekback to be able to reread the unparsed Data the next time
	}
	
	if ((i%Buff_len) != 0) { 
		for (; i < (k + 1)*Buff_len; i++)			 Ex[i + 2*k] = Ey[i + 2*k] = 0.0f;
		k++; 
	}

	free(temp_Buff);
	return k;				// returns Number of Filled Arrays 

}
int Read_Bin_to_float(float *Ex, float *Ey, ifstream &InpFile, const int Buff_len, const int Buff_num, const int64_t MAX_TotSize /* = 512 * 1024 * 1024 */) {			//Buff_num == (No of Buff_len Arrays to fill)/Channel				
																																								// Todo: Add Multibyte int Read, float and double Read
	const int64_t Size = min<int64_t>(2 * Buff_len*Buff_num * 1, MAX_TotSize);

	register char *temp_Buff = (char *)malloc(Size);								//Buffer

	register int i = 0;  register int k = 0;

	while ((i < Buff_len*Buff_num) && (!InpFile.eof())) {

		InpFile.read(temp_Buff, min<int64_t>(2 * (Buff_len*Buff_num - i) * 1, Size));			// Reads only required amount of data, clamped by Max Buffer Size
		streamsize ChRead_cnt = InpFile.gcount();

		const int i_end = i + ChRead_cnt / 2;

		while (i < i_end) {									// Converts and loads fft buffer accordingly (takes care of padding), Keeps track of number of read bytes so as to not convert more than then read & not write more than the data exists.

			Ex[i + 2*k] = (float)temp_Buff[2*i];
			Ey[i + 2*k] = (float)temp_Buff[2*i + 1];
			i++;

			if (i%Buff_len == 0)		k++;
		}
	}

	if ((i%Buff_len) != 0) {
		for (; i < (k + 1)*Buff_len; i++)			 Ex[i + 2 * k] = Ey[i + 2 * k] = 0.0f;
		k++;
	}

	free(temp_Buff);
	return k;				// returns Number of Filled Arrays 
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
