// VelaPulsar_Spectrum.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <sys/types.h>
#include <math.h>

#include"../lib/fftw-3.3.5-dll64/fftw3.h"
#include "stdafx.h"

using namespace std;

int Read_Ascii_to_float(float *Ex, float *Ey, ifstream &InpFile, int Buff_Size);
int Read_Bin_to_float(float *Ex, float *Ey, ifstream &InpFile, int Buff_Size);

void Roundto2p(int &BUFF_Size);
/*
struct Cpx_Int
{
	int r;
	int i;
};
*/
enum DataMode {BIN, ASCII};
enum Complex { real, imag };

int main()								// Spectrogram cpp
{																										// argv analysis
	string IN_PATH__, OUT_PATH__; 
	bool InpMode, OutMode;
	float SampleRate;		// in MHz
	float TimeRes;			// in seconds

	/*			Debug		*/
	int argc = 7;

	//argv = (char**) malloc(argc * 1024);
	const char argv[][1024] = { "VelaPulsar_Spectrum.exe", "D:\\Documents\\SWAN_RRI\\Vega_Pulsar\\ch00_B0833-45_20150612_191438_010_1", "BIN", "D:\\Documents\\SWAN_RRI\\Vega_Pulsar\\FFT\\" , "BIN", "1.024", "0.100" };

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
	

	ifstream DataIn; 

	DataIn.open(IN_PATH__, ios::in | (InpMode ? 0 : ios::binary));											// Opening Data File

	if (!DataIn.is_open()) {
		cout << " Error Opening the file, please recheck the Input Filepath and Try Again \n";
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";
		
		DataIn.close();
		exit(2);
	}
	DataIn.seekg(ios::beg);
	

	const string FILE_NAME__ = IN_PATH__.substr(IN_PATH__.find_last_of('\\'));

	ifstream OutFileTemp; OutFileTemp.open(OUT_PATH__ + FILE_NAME__ + "_Ex_" + '0' + " .bin", ios::in | ios::binary);

	if (OutFileTemp.is_open()) {
		cout << " Output file already exists, please use a different name and Try Again \n";
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";
		
		OutFileTemp.close();
		exit(3);
	}
	OutFileTemp.close();

	OutFileTemp.open(OUT_PATH__ + FILE_NAME__ + "_Ey_" + '0' + " .bin", ios::in | ios::binary);

	if (OutFileTemp.is_open()) {
		cout << " Output file already exists, please use a different name and Try Again \n";
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";

		OutFileTemp.close();
		exit(3);
	}
	OutFileTemp.close(); 

	ofstream FFT_Ex_OutBin, FFT_Ey_OutBin;  //Todo: check folder is empty
/*
	if (!FFT_Ex_OutBin.is_open()) {
		cout << " Error writting to file, please recheck the Output Filepath and Try Again \n";
		cout << "Format: " + string(argv[0]) + "  <input file PATH> InpMode(BIN/ASCII) <output file PATH> OutMode(BIN/ASCII) Sample_rate(in MHz) Time_Resolution(in seconds) \n";

		FFT_Ex_OutBin.close();
		exit(2);
	}
	FFT_Ex_OutBin.seekp(ios::beg);

*/

	int BUFF_Size = (SampleRate*1000000)* TimeRes;	

	Roundto2p(BUFF_Size);

	TimeRes = (BUFF_Size / (SampleRate * 1000000));		// in seconds

	cout << "Time_Resolution set to " << TimeRes << "(in seconds)" << endl;

	float *Ex;  Ex = (float *)fftwf_malloc(BUFF_Size * sizeof(float));
	float *Ey;  Ey = (float *)fftwf_malloc(BUFF_Size * sizeof(float));
	
	fftwf_complex *FFT_Ex; FFT_Ex = (fftwf_complex *)fftwf_malloc((BUFF_Size/2 +1)* sizeof(fftwf_complex));
	fftwf_complex *FFT_Ey; FFT_Ey = (fftwf_complex *)fftwf_malloc((BUFF_Size/2 +1)* sizeof(fftwf_complex));

	fftwf_plan cfg_x = fftwf_plan_dft_r2c_1d(BUFF_Size, Ex, FFT_Ex, FFTW_PATIENT);
	fftwf_plan cfg_y = fftwf_plan_dft_r2c_1d(BUFF_Size, Ey, FFT_Ey, FFTW_PATIENT);

		int i = 0;
		while (!DataIn.eof())
		{
			if(InpMode == ASCII)		Read_Ascii_to_float(Ex, Ey, DataIn, BUFF_Size);
			else						Read_Bin_to_float(Ex, Ey, DataIn, BUFF_Size);
				
				fftwf_execute(cfg_x);
				FFT_Ex_OutBin.open(OUT_PATH__ + FILE_NAME__ + "_Ex_" + to_string(i) + " .bin", ios::out | ios::trunc | (OutMode ? (int)0 : ios::binary));
				FFT_Ex_OutBin.seekp(ios::beg);

				if (OutMode == ASCII)		
					for (int j = 0; j < BUFF_Size/2 + 1; j++)			 
						FFT_Ex_OutBin << FFT_Ex[j][real] << " " << FFT_Ex[j][imag] << "\n";
				else						FFT_Ex_OutBin.write((char*)FFT_Ex, 2 * sizeof(float)*(BUFF_Size / 2 + 1));
				FFT_Ex_OutBin.close();

				fftwf_execute(cfg_y);
				FFT_Ey_OutBin.open(OUT_PATH__ + FILE_NAME__ + "_Ey_" + to_string(i) + " .bin", ios::out | ios::trunc | (OutMode ? (int)0 : ios::binary));
				FFT_Ey_OutBin.seekp(ios::beg);

				if (OutMode == ASCII)		for (int j = 0; j < BUFF_Size / 2 + 1; j++)			 FFT_Ey_OutBin << FFT_Ey[j][real] << " " << FFT_Ey[j][imag] << "\n";
				else						FFT_Ey_OutBin.write((char*)FFT_Ey, 2 * sizeof(float)*(BUFF_Size / 2 + 1));
				FFT_Ey_OutBin.close();

				i++;
		}
	
	fftwf_destroy_plan(cfg_x);
	fftwf_destroy_plan(cfg_y);

	fftwf_free(Ex); fftwf_free(Ey);
	fftwf_free(FFT_Ex); fftwf_free(FFT_Ey);

	return 0;
}

void Roundto2p(int &BUFF_Size) {

	int p = 0;
	int temp_Size = BUFF_Size;

	while (temp_Size > 1)
	{
		temp_Size = temp_Size >> 1;
		p++;
	}

	if (abs(BUFF_Size - (1 << p)) > abs(BUFF_Size - (1 << (p + 1)))) 		BUFF_Size = 1 << p;
	else																BUFF_Size = 1 << (p + 1);

}

int Read_Ascii_to_float(float *Ex, float *Ey, ifstream &InpFile, int Buff_Size) {

	string temp_x, temp_y;								// buffer
	
	int i = 0;

	while (i < Buff_Size && !InpFile.eof())
	{
		getline(InpFile, temp_x, ' ');
		getline(InpFile, temp_y, ' ');

		Ex[i] = (float)strtof(temp_x.c_str(), NULL);
		Ey[i] = (float)strtof(temp_y.c_str(), NULL);

		i++;
	}

	if (InpFile.eof())		while (i < Buff_Size)		{	Ex[i] = Ey[i] = 0;		i++; }

	return 0;
}
int Read_Bin_to_float(float *Ex, float *Ey, ifstream &InpFile, int Buff_Size) {

	char temp_x, temp_y;								// buffer

	int i = 0;

	while (i < Buff_Size && !InpFile.eof())
	{
		InpFile.read(&temp_x, 1);
		InpFile.read(&temp_y, 1);

		Ex[i] = (float)temp_x;
		Ey[i] = (float)temp_y;

		i++;
	}

	if (InpFile.eof())		while (i < Buff_Size) { Ex[i] = Ey[i] = 0;		i++; }

	return 0;
}