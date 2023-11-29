#include <iostream>
#include <fstream>
#include <cstdint>
#include <stdio.h>
#include <math.h>

#define TWO_PI      6.28318530717959
#define SWAP(a,b)   tempr = (a);(a) = (b);(b) = tempr

struct wavHeader {
	char chunkID[4];
	uint32_t chunkSize;
	char format[4];
	char subchunk1ID[4];
	uint32_t subchunk1Size;
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	char subchunk2ID[4];
	uint32_t subchunk2Size;
	uint32_t dataElements;
	int16_t* data;
};

wavHeader readHeader(FILE* filename);
size_t fwriteIntLSB(int32_t data, FILE* stream);
size_t fwriteShortLSB(short int data, FILE* stream);
size_t fwrite24BitLSB(int32_t data, FILE* stream);
void printHeader(wavHeader fileHeader);
void writeFile(wavHeader fileHeader, int16_t* data, FILE* outputFile);
double* scaleData(int16_t* data, uint32_t size);
void convolve(double* x, uint32_t N, double* h, uint32_t M, double* y, uint32_t P);
int16_t* descaleData(double* data, uint32_t size);
wavHeader produceOutput(uint32_t size);
void four1(double data[], int nn, int isign);
double* freqConvolve(double x[], double h[], uint32_t N);
double* zeroPadArray(uint32_t newSize, double* data, uint32_t size);


int main(int argc, char *argv[])
{
	 FILE* sourceFile = NULL;
	 FILE* irFile = NULL;
	 FILE* outputFile = NULL;

	 wavHeader sourceHeader;
	 wavHeader irHeader;
	 wavHeader outputHeader;

	 double* scaledSource = NULL;
	 double* scaledIR = NULL;
	 double* scaledOutput = NULL;
	 double* paddedSource = NULL;
	 double* paddedIR = NULL;
	 int16_t* outputData = NULL;

	 uint32_t outputSize = 0;
	 uint32_t largestSize = 0;
	 uint32_t newSize = 1;

	 if (argc == 4)
	 {
		 sourceFile = fopen(argv[1], "rb");
		 if (sourceFile == NULL)
		 {
			 std::cerr << "Error: Failed to open file \"" << argv[1] << "\"" << std::endl;
			 exit(-1);
		 }

		 irFile = fopen(argv[2], "rb");
		 if (irFile == NULL)
		 {
			 std::cerr << "Error: Failed to open file \"" << argv[2] << "\"" << std::endl;
			 exit(-1);
		 }
		 outputFile = fopen(argv[3], "wb");
		 if (outputFile == NULL)
		 {
			 std::cerr << "Error: Failed to create file \"" << argv[3] << "\"" << std::endl;
			 exit(-1);
		 }

		 sourceHeader = readHeader(sourceFile);
		 irHeader = readHeader(irFile);
		 
		 std::cout << "Input File: \"" << argv[1] << "\" information:\n" << "================================" << std::endl;
		 printHeader(sourceHeader);
		 std::cout << "================================" << std::endl << std::endl;

		 std::cout << "IR File: \"" << argv[2] << "\" information:\n" << "================================" << std::endl;
		 printHeader(irHeader);
		 std::cout << "================================" << std::endl;
		 
		 outputSize = sourceHeader.dataElements + irHeader.dataElements - 1;
		 outputHeader = produceOutput(outputSize);

		 scaledSource = scaleData(sourceHeader.data, sourceHeader.dataElements);
		 scaledIR = scaleData(irHeader.data, irHeader.dataElements);
		 
		 if (sourceHeader.dataElements > irHeader.dataElements)
			 largestSize = sourceHeader.dataElements;
		 else
			 largestSize = irHeader.dataElements;

		 while (newSize < largestSize) {
			 newSize <<= 1;
		 }
		 
		 paddedSource = zeroPadArray(newSize, scaledSource, sourceHeader.dataElements);
		 paddedIR = zeroPadArray(newSize, scaledIR, irHeader.dataElements);

		 delete(scaledSource);
		 delete(scaledIR);

		 four1(paddedSource- 1, newSize, 1);
		 four1(paddedIR - 1, newSize, 1);
		 scaledOutput = freqConvolve(paddedSource, paddedIR, newSize);
		 four1(scaledOutput -1, newSize, -1);
		 
		 outputData = descaleData(scaledOutput, newSize);
		 writeFile(outputHeader, outputData, outputFile);

		 fclose(sourceFile);
		 fclose(irFile);
		 fclose(outputFile);
	 }
	 else
	 {
		 std::cerr << "Error: Incorrect number of arguments given. Proper usage is \"" << argv[0] << "\" sourceFilename impulseResponseFilename outputFilename" << std::endl;
		 exit(-1);
	 }


	 return 0;
}

wavHeader readHeader(FILE* filename)
{
	wavHeader fileHeader;
	char junk = '\0';
	int16_t temp = 0;

	fread(fileHeader.chunkID, sizeof(char), 4, filename);

	fread(&fileHeader.chunkSize, sizeof(uint32_t), 1, filename);

	fread(fileHeader.format, sizeof(char), 4, filename);
	fread(fileHeader.subchunk1ID, sizeof(char), 4, filename);

	fread(&fileHeader.subchunk1Size, sizeof(uint32_t), 1, filename);
	fread(&fileHeader.audioFormat, sizeof(uint16_t), 1, filename);
	fread(&fileHeader.numChannels, sizeof(uint16_t), 1, filename);
	fread(&fileHeader.sampleRate, sizeof(uint32_t), 1, filename);
	fread(&fileHeader.byteRate, sizeof(uint32_t), 1, filename);
	fread(&fileHeader.blockAlign, sizeof(uint16_t), 1, filename);
	fread(&fileHeader.bitsPerSample, sizeof(uint16_t), 1, filename);

	if (fileHeader.subchunk1Size > 16) {
		for (uint32_t i = 0; i < (fileHeader.subchunk1Size - 16); i++)
			fread(&junk, sizeof(char), 1, filename);
	}

	fread(fileHeader.subchunk2ID, sizeof(char), 4, filename);
	fread(&fileHeader.subchunk2Size, sizeof(uint32_t), 1, filename);

	fileHeader.dataElements = fileHeader.subchunk2Size / sizeof(int16_t);
	fileHeader.data = new int16_t[fileHeader.dataElements];
	fread(fileHeader.data, sizeof(int16_t), fileHeader.dataElements, filename);
		
	return fileHeader;
}

void printHeader(wavHeader fileHeader) {
	std::cout << "Chunk ID: " << fileHeader.chunkID[0] << fileHeader.chunkID[1] << fileHeader.chunkID[2] << fileHeader.chunkID[3] << std::endl;
	std::cout << "Chunk Size: " << fileHeader.chunkSize << std::endl;
	std::cout << "Format: " << fileHeader.format[0] << fileHeader.format[1] << fileHeader.format[2] << fileHeader.format[3] << std::endl;
	std::cout << "Subchunk 1 ID: " << fileHeader.subchunk1ID[0] << fileHeader.subchunk1ID[1] << fileHeader.subchunk1ID[2] << fileHeader.subchunk1ID[3] << std::endl;
	std::cout << "Subchunk 1 Size: " << fileHeader.subchunk1Size << std::endl;
	std::cout << "Audio Format: " << fileHeader.audioFormat << std::endl;
	std::cout << "Number of Channels: " << fileHeader.numChannels << std::endl;
	std::cout << "Sample Rate: " << fileHeader.sampleRate << std::endl;
	std::cout << "Byte Rate: " << fileHeader.byteRate << std::endl;
	std::cout << "Block Align: " << fileHeader.blockAlign << std::endl;
	std::cout << "Bits Per Sample: " << fileHeader.bitsPerSample << std::endl;
	std::cout << "Subchunk 2 ID: " << fileHeader.subchunk2ID[0] << fileHeader.subchunk2ID[1] << fileHeader.subchunk2ID[2] << fileHeader.subchunk2ID[3] << std::endl;
	std::cout << "Subchunk 2 Size: " << fileHeader.subchunk2Size << std::endl;
	std::cout << "Number of data elements: " << fileHeader.dataElements << std::endl;
}

void writeFile(wavHeader fileHeader, int16_t* data, FILE* outputFile) {
	fputs("RIFF", outputFile);

	fwriteIntLSB(fileHeader.chunkSize, outputFile);

	fputs("WAVE", outputFile);

	fputs("fmt ", outputFile);

	fwriteIntLSB(fileHeader.subchunk1Size, outputFile);

	fwriteShortLSB(fileHeader.audioFormat, outputFile);

	fwriteShortLSB(fileHeader.numChannels, outputFile);

	fwriteIntLSB(fileHeader.sampleRate, outputFile);

	fwriteIntLSB(fileHeader.byteRate, outputFile);

	fwriteShortLSB(fileHeader.blockAlign, outputFile);

	fwriteShortLSB(fileHeader.bitsPerSample, outputFile);

	fputs("data", outputFile);

	fwriteIntLSB(fileHeader.subchunk2Size, outputFile);

	for (uint32_t i = 0; i < fileHeader.dataElements; i++)
	{
		fwriteShortLSB(data[i], outputFile);
	}
}

size_t fwriteIntLSB(int32_t data, FILE* stream)
{
	unsigned char array[4];

	array[3] = (unsigned char)((data >> 24) & 0xFF);
	array[2] = (unsigned char)((data >> 16) & 0xFF);
	array[1] = (unsigned char)((data >> 8) & 0xFF);
	array[0] = (unsigned char)(data & 0xFF);
	return fwrite(array, sizeof(unsigned char), 4, stream);
}

size_t fwriteShortLSB(short int data, FILE* stream)
{
	unsigned char array[2];

	array[1] = (unsigned char)((data >> 8) & 0xFF);
	array[0] = (unsigned char)(data & 0xFF);
	return fwrite(array, sizeof(unsigned char), 2, stream);
}

size_t fwrite24BitLSB(int32_t data, FILE* stream)
{
	unsigned char array[3];

	array[2] = (unsigned char)((data >> 16) & 0xFF);
	array[1] = (unsigned char)((data >> 8) & 0xFF);
	array[0] = (unsigned char)(data & 0xFF);
	return fwrite(array, sizeof(unsigned char), 3, stream);
}

double* scaleData(int16_t* data, uint32_t size) {
	double* scaledInput = new double[size];

	for (size_t i = 0; i < size; i++) {
		scaledInput[i] = static_cast<double>(data[i]);
	}
	return scaledInput;
}

void convolve(double* x, uint32_t N, double* h, uint32_t M, double* y, uint32_t P) {
	size_t n, m;
	
	for (n = 0; n < P; n++) {
		y[n] = 0.0;
	}

	for (n = 0; n < N; n++) {
		for (m = 0; m < M; m++) {
			y[n + m] += x[n] * h[m];
		}
	}
}

int16_t* descaleData(double* data, uint32_t size) {
	int16_t* descaled = new int16_t[size];
	
	for (size_t i = 0; i < size; i++) {
		descaled[i] = static_cast<int16_t>(data[i << 1] * 10);
	}
	return descaled;
}

wavHeader produceOutput(uint32_t size) {
	wavHeader outputHeader;
	outputHeader.chunkID[0] = 'R';
	outputHeader.chunkID[1] = 'I';
	outputHeader.chunkID[2] = 'F';
	outputHeader.chunkID[3] = 'F';

	outputHeader.format[0] = 'W';
	outputHeader.format[1] = 'A';
	outputHeader.format[2] = 'V';
	outputHeader.format[3] = 'E';
		
	outputHeader.subchunk1ID[0] = 'f';
	outputHeader.subchunk1ID[1] = 'm';
	outputHeader.subchunk1ID[2] = 't';
	outputHeader.subchunk1ID[3] = ' ';

	outputHeader.subchunk2ID[0] = 'd';
	outputHeader.subchunk2ID[1] = 'a';
	outputHeader.subchunk2ID[2] = 't';
	outputHeader.subchunk2ID[3] = 'a';

	outputHeader.bitsPerSample = 16;
	outputHeader.numChannels = 1;
	outputHeader.sampleRate = 44100;
	outputHeader.dataElements = size;

	outputHeader.blockAlign = (outputHeader.numChannels * outputHeader.bitsPerSample) / 8;
	outputHeader.byteRate = outputHeader.sampleRate * outputHeader.blockAlign;

	outputHeader.subchunk2Size = outputHeader.dataElements* outputHeader.blockAlign;

	outputHeader.chunkSize = 36 + outputHeader.subchunk2Size;
	outputHeader.subchunk1Size = 16;
	outputHeader.audioFormat = 1;

	return outputHeader;
}

void four1(double data[], int nn, int isign) {
	uint32_t n, mmax, m, j, istep, i;
	double wtemp, wr, wpr, wpi, wi, theta;
	double tempr, tempi;
	double *swapj, *swapi;

	n = nn << 1;
	j = 1;

	for (i = 1; i < n; i += 2) {
		if (j > i) {
			swapj = &data[j];
			swapi = &data[i];
			SWAP(*swapj, *swapi);

			swapj = &data[j+1];
			swapi = &data[i+1];
			SWAP(*swapj, *swapi);
		}

		m = nn;

		while(m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}

		j += m;
	}

	mmax = 2;
	while (n > mmax) {
		istep = mmax << 1;
		theta = isign * (TWO_PI / mmax);
		wtemp = sin(0.5 * theta);
		wpr = -2.0 * wtemp * wtemp;
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;
		for (m = 1; m < mmax; m += 2) {
			for (i = m; i <= n; i += istep) {
				j = i + mmax;
				tempr = wr * data[j] - wi * data[j + 1];
				tempi = wr * data[j + 1] + wi * data[j];
				data[j] = data[i] - tempr;
				data[j + 1] = data[i + 1] - tempi;
				data[i] += tempr;
				data[i + 1] += tempi;
			}
			wr = (wtemp = wr) * wpr - wi * wpi + wr;
			wi = wi * wpr + wtemp * wpi + wi;
		}
		mmax = istep;
	}


}

double* zeroPadArray(uint32_t newSize, double* data, uint32_t size) {
	double* paddedArr;
	uint32_t arrSize = newSize << 1;

	paddedArr = new double[arrSize];


	for (size_t i = 0; i < arrSize; i++) {
		paddedArr[i] = 0.0;
	}

	for (size_t i = 0; i < size; i++) {
		paddedArr[(i << 1)] = data[i];
		paddedArr[(i << 1) + 1] = 0.0;
	}

	return paddedArr;
  
}

double* freqConvolve(double x[], double h[], uint32_t N) {
	uint32_t i, k, two_N = N << 1;
	double tempXR = 0.0, tempXI = 0.0, tempHR = 0.0, tempHI = 0.0;
	double* y = new double[two_N];

	for (k = 0, i = 0; k < N; k++, i += 2) {
		// Scale results by N
		tempXR = x[i] / (double)N;
		tempXI = x[i + 1] / (double)N;
		tempHR = h[i] / (double)N;
		tempHI = h[i + 1] / (double)N;
		
		y[i] = (tempXR * tempHR) - (tempXI * tempHI);
		y[i + 1] = (tempXI * tempHR) + (tempHI * tempXR);
	}

	return y;
}