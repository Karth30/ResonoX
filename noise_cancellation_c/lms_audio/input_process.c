#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// WAV file header structure (44 bytes)
typedef struct {
    char chunkID[4];      // "RIFF"
    int chunkSize;
    char format[4];       // "WAVE"
    char subchunk1ID[4];  // "fmt "
    int subchunk1Size;
    short audioFormat;
    short numChannels;
    int sampleRate;
    int byteRate;
    short blockAlign;
    short bitsPerSample;
    char subchunk2ID[4];  // "data"
    int subchunk2Size;
} WAVHeader;

void convert_to_mono_16bit_44kHz(const char *inputFile, const char *outputFile) {
    FILE *inFile = fopen(inputFile, "rb");
    if (!inFile) {
        printf("Error: Cannot open input file %s\n", inputFile);
        return;
    }

    // Read the WAV header
    WAVHeader header;
    fread(&header, sizeof(WAVHeader), 1, inFile);

    // Print original file info
    printf("Original WAV File: %d Hz, %d-bit, %d channel(s)\n",
           header.sampleRate, header.bitsPerSample, header.numChannels);

    // Ensure the input is PCM format
    if (header.audioFormat != 1) {
        printf("Error: Only PCM WAV files are supported!\n");
        fclose(inFile);
        return;
    }

    // Convert to 16-bit, mono, 44.1kHz
    header.sampleRate = 44100;
    header.numChannels = 1;
    header.bitsPerSample = 16;
    header.byteRate = header.sampleRate * header.numChannels * (header.bitsPerSample / 8);
    header.blockAlign = header.numChannels * (header.bitsPerSample / 8);
    header.subchunk2Size = header.subchunk2Size / header.numChannels; // Adjust for mono
    header.chunkSize = 36 + header.subchunk2Size; // Update chunk size

    FILE *outFile = fopen(outputFile, "wb");
    if (!outFile) {
        printf("Error: Cannot create output file %s\n", outputFile);
        fclose(inFile);
        return;
    }

    // Write updated header to output file
    fwrite(&header, sizeof(WAVHeader), 1, outFile);

    // Process audio data
    short sample;
    int totalSamples = header.subchunk2Size / (header.bitsPerSample / 8);
    for (int i = 0; i < totalSamples; i++) {
        fread(&sample, sizeof(short), 1, inFile);
        fwrite(&sample, sizeof(short), 1, outFile);
        
        // Skip extra channel in stereo input
        if (header.numChannels == 2) {
            fseek(inFile, sizeof(short), SEEK_CUR);
        }
    }

    fclose(inFile);
    fclose(outFile);

    printf("Conversion complete! Output saved as '%s'.\n", outputFile);
}

int main() {
    const char *inputWAV = "noisy_audio.wav";
    const char *outputWAV = "converted_audio.wav";

    convert_to_mono_16bit_44kHz(inputWAV, outputWAV);

    return 0;
}
