#include <iostream>
#include <fstream>
#include <cstdint>

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
    int16_t* data;
};

wavHeader readHeader(FILE* filename);
uint16_t wap_endian16(uint16_t val);
uint32_t swap_endian32(uint32_t val);
void printHeader(wavHeader fileHeader);

int main(int argc, char *argv[])
{
     FILE* sourceFile = NULL;
     FILE* irFile = NULL;

     wavHeader sourceHeader;
     wavHeader irHeader;

     if (argc == 3)
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
         std::cout << "Files " << argv[1] << " and " << argv[2] << " successfully opened!" << std::endl;
    
         sourceHeader = readHeader(sourceFile);
         printHeader(sourceHeader);

         std::cout << std::endl << std::endl;

         irHeader = readHeader(irFile);
         printHeader(irHeader);

     }
     else
     {
         std::cerr << "Error: Incorrect number of arguments given. Proper usage is \"" << argv[0] << "\" sourceFilename impulseResponseFilename" << std::endl;
         exit(-1);
     }


     return 0;
}

wavHeader readHeader(FILE* filename)
{
    wavHeader fileHeader;
    char junk = '\0';

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
        fileHeader.subchunk1Size = 16;
    }

    fread(fileHeader.subchunk2ID, sizeof(char), 4, filename);
    fread(&fileHeader.subchunk2Size, sizeof(uint32_t), 1, filename);

    fileHeader.data = new int16_t[fileHeader.subchunk2Size / sizeof(int16_t)];
    fread(fileHeader.data, sizeof(int16_t), (fileHeader.subchunk2Size / sizeof(int16_t)), filename);

    return fileHeader;
}

uint16_t swap_endian16(uint16_t val) {
    return (val >> 8) | (val << 8);
}

uint32_t swap_endian32(uint32_t val) {
    return (val << 24) | ((val << 8) & 0x00ff0000) | ((val >> 8) & 0x0000ff00) | (val >> 24);
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
}
