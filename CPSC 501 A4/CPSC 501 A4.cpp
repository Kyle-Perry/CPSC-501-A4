#include <iostream>
#include <fstream>

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
    //int16_t* data;
};

int main(int argc, char *argv[])
{
     FILE* sourceFile = NULL;
     FILE* impulseResponseFile = NULL;

     if (argc == 3)
     {

     }
     else
     {
         std::cerr << "Error: Incorrect number of arguments given. Proper usage is \"" << argv[0] << "\" sourceFilename impulseResponseFilename" << std::endl;
     }
}