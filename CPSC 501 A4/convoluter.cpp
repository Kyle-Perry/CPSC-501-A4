#include <iostream>
#include <fstream>
#include <cstdint>
#include <stdio.h>

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
size_t fwriteIntLSB(int data, FILE* stream);
size_t fwriteShortLSB(short int data, FILE* stream);
void printHeader(wavHeader fileHeader);
void writeFile(wavHeader fileHeader, FILE* outputFile);

int main(int argc, char *argv[])
{
     FILE* sourceFile = NULL;
     FILE* irFile = NULL;
     FILE* outputFile = NULL;

     wavHeader sourceHeader;
     wavHeader irHeader;

     if (argc == 4)
     {
         fopen_s(&sourceFile, argv[1], "r");
         if (sourceFile == NULL)
         {
             std::cerr << "Error: Failed to open file \"" << argv[1] << "\"" << std::endl;
             exit(-1);
         }

         fopen_s(&irFile, argv[2], "r");
         if (irFile == NULL)
         {
             std::cerr << "Error: Failed to open file \"" << argv[2] << "\"" << std::endl;
             exit(-1);
         }
         fopen_s(&outputFile, argv[3], "wb");
         if (outputFile == NULL)
         {
             std::cerr << "Error: Failed to create file \"" << argv[3] << "\"" << std::endl;
             exit(-1);
         }

         sourceHeader = readHeader(sourceFile);
         printHeader(sourceHeader);

         sourceHeader.subchunk1Size = 16;

         std::cout << std::endl << std::endl;

         irHeader = readHeader(irFile);
         printHeader(irHeader);

         writeFile(sourceHeader, outputFile);

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
        for (int i = 0; i < (fileHeader.subchunk1Size - 16); i++)
            fread(&junk, sizeof(char), 1, filename);
    }

    fread(fileHeader.subchunk2ID, sizeof(char), 4, filename);
    fread(&fileHeader.subchunk2Size, sizeof(uint32_t), 1, filename);

    fileHeader.dataElements = fileHeader.subchunk2Size / sizeof(int16_t);
    fileHeader.data = new int16_t[fileHeader.dataElements];
    for(int i = 0; i < fileHeader.dataElements; i++)
        fread(fileHeader.data, sizeof(int16_t), 1, filename);

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

void writeFile(wavHeader fileHeader, FILE* outputFile) {
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

    std::cout << fileHeader.dataElements << std::endl;
    for (uint32_t i = 0; i < fileHeader.dataElements; i++)
    {
        fwriteShortLSB(fileHeader.data[i], outputFile);
    }
}

size_t fwriteIntLSB(int data, FILE* stream)
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
