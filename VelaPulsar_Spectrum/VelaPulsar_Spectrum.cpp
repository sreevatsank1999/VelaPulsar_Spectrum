// VelaPulsar_Spectrum.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include "stdafx.h"
#include "lib\kiss_fft.h"
#include "lib\kiss_fftr.h"

using namespace std;

int Ascii_to_Int(int *Ex, int *Ey, ifstream &InpFile, int Buff_Size);
void Roundto2p(int &BUFF_Size);

struct Cpx_Int
{
	int r;
	int i;
};


int main(char* argv[], int argc)								// Spectrogram cpp
{																										// argv analysis
	string IN_PATH__, OUT_PATH__;
	float SampleRate;		// in Hz
	float TimeRes;			// in ms

	if (string(argv[1]) == "-h" || string(argv[1]) == "--help") {
		cout << "Format: Spectrogram  <input file PATH>  <output file PATH> Sample_rate(in Hz) Time_Resolution(in ms) \n";
		exit(0);
	}

	if (argc < 5) {
		cout << "Too few arguements.................\n ";
		cout << "Format: Spectrogram  <input file PATH>  <output file PATH> Sample_rate(in Hz) Time_Resolution(in ms) \n";
		exit(-1);
	}
	else if (argc == 5) {
		IN_PATH__ = string(argv[1]);
		OUT_PATH__ = string(argv[2]);
		
		SampleRate = strtof(argv[3],NULL);
		TimeRes = strtof(argv[4], NULL);
	}
	else if (argc > 5) {

		cout << "Too many arguements.................\n ";
		cout << "Format: Spectrogram  <input file PATH>  <output file PATH> Sample_rate(in Hz) Time_Resolution(in ms) \n";
		exit(1);
	}
	

	ifstream DataIn_Ascii; 

	DataIn_Ascii.open(IN_PATH__, ios::in);											// Opening Data File

	if (!DataIn_Ascii.is_open()) {
		cout << " Error Opening the file, please recheck the Input Filepath and Try Again \n";
		cout << "Format: Spectrogram  <input file PATH>  <output file PATH> Sample_rate(in Hz) Time_Resolution(in ms) \n";
		
		DataIn_Ascii.close();
		exit(2);
	}
	DataIn_Ascii.seekg(ios::beg);
	

	string FILE_NAME__ = IN_PATH__.substr(IN_PATH__.find_last_of('/'));

	ifstream OutFileTemp; OutFileTemp.open(OUT_PATH__ + FILE_NAME__ + "_Ex_" + string((int)0) + " .bin" , ios::in); 

	if (OutFileTemp.is_open()) {
		cout << " Output file already exists, please use a different name and Try Again \n";
		cout << "Format: Spectrogram  <input file PATH>  <output file PATH> Sample_rate(in Hz) Time_Resolution(in ms) \n";
		
		OutFileTemp.close();
		exit(3);
	}
	OutFileTemp.close();

	OutFileTemp.open(OUT_PATH__ + FILE_NAME__ + "_Ey_" + string((int)0) + " .bin", ios::in);

	if (OutFileTemp.is_open()) {
		cout << " Output file already exists, please use a different name and Try Again \n";
		cout << "Format: Spectrogram  <input file PATH>  <output file PATH> Sample_rate(in Hz) Time_Resolution(in ms) \n";

		OutFileTemp.close();
		exit(3);
	}
	OutFileTemp.close(); 

	ofstream FFT_Ex_OutBin, FFT_Ey_OutBin;  // check folder is empty
/*
	if (!FFT_Ex_OutBin.is_open()) {
		cout << " Error writting to file, please recheck the Output Filepath and Try Again \n";
		cout << "Format: Spectrogram  <input file PATH>  <output file PATH> Sample_rate(in Hz) Time_Resolution(in ms) \n";

		FFT_Ex_OutBin.close();
		exit(2);
	}
	FFT_Ex_OutBin.seekp(ios::beg);

*/

	int BUFF_Size = SampleRate*TimeRes / 1000;	

	Roundto2p(BUFF_Size);

	TimeRes = (BUFF_Size / SampleRate) * 1000;		// in ms 

	cout << "Time_Resolution set to " << TimeRes << "(in ms)" << endl;


	int *Ex;  Ex = (int *)malloc(BUFF_Size * sizeof(int));
	int *Ey;  Ey = (int *)malloc(BUFF_Size * sizeof(int));
		
	kiss_fftr_cfg cfg = kiss_fftr_alloc(BUFF_Size, 0, NULL, NULL);
	
	kiss_fft_cpx *FFT_Ex; FFT_Ex = (kiss_fft_cpx *)malloc(BUFF_Size * sizeof(kiss_fft_cpx));
	kiss_fft_cpx *FFT_Ey; FFT_Ey = (kiss_fft_cpx *)malloc(BUFF_Size * sizeof(kiss_fft_cpx));


	if (DataIn_Ascii.is_open()) {
	
		int i = 0;
		while (!DataIn_Ascii.eof())
		{
			Ascii_to_Int(Ex, Ey, DataIn_Ascii, BUFF_Size);
				
				kiss_fftr(cfg, Ex, FFT_Ex);
				FFT_Ex_OutBin.open(OUT_PATH__ + FILE_NAME__ + "_Ex_" + to_string(i) + " .bin", ios::out | ios::trunc);	 //| ios::binary);
				FFT_Ex_OutBin.seekp(ios::beg);
				for (int j = 0; j < BUFF_Size; j++)			 FFT_Ex_OutBin << FFT_Ex[j].r << "," << FFT_Ex[j].i << "\n";
				FFT_Ex_OutBin.close();

				kiss_fftr(cfg, Ey, FFT_Ey);
				FFT_Ey_OutBin.open(OUT_PATH__ + FILE_NAME__ + "_Ey_" + to_string(i) + " .bin", ios::out | ios::trunc);	 //| ios::binary);
				FFT_Ey_OutBin.seekp(ios::beg);
				for (int j = 0; j < BUFF_Size; j++)			 FFT_Ey_OutBin << FFT_Ey[j].r << "," << FFT_Ey[j].i << "\n";
				FFT_Ey_OutBin.close();

				i++;
			}
		}
	else			cout << "Error Opening File\n";
	
	kiss_fftr_free(cfg);
	
	return 0;
}

void Roundto2p(int &BUFF_Size) {

	int p = 0;
	int temp_Size = BUFF_Size;

	while (temp_Size == 1)
	{
		temp_Size = temp_Size >> 1;
		p++;
	}

	if (abs(BUFF_Size - (1 << p)) > abs(BUFF_Size - (1 << (p + 1)))) 		BUFF_Size = 1 << p;
	else																BUFF_Size = 1 << (p + 1);

}

int Ascii_to_Int(int *Ex, int *Ey, ifstream &InpFile, int Buff_Size) {

	string temp_x, temp_y;								// buffer
	
	int i = 0;

	while (i < Buff_Size && !InpFile.eof())
	{
		getline(InpFile, temp_x, ' ');
		getline(InpFile, temp_y, ' ');

		Ex[i] = (int)strtol(temp_x.c_str(), NULL, 10);
		Ey[i] = (int)strtol(temp_y.c_str(), NULL, 10);

		i++;
	}

	if (InpFile.eof())		while (i < Buff_Size)		{	Ex[i] = Ey[i] = 0;		i++; }

	return 0;
}