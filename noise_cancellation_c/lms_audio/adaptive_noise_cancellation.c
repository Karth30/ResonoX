#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FRAME_SIZE 1024
#define MU 0.0001  // Learning rate

// WAV header structure
typedef struct {
    char chunkID[4];
    int chunkSize;
    char format[4];
    char subchunk1ID[4];
    int subchunk1Size;
    short audioFormat;
    short numChannels;
    int sampleRate;
    int byteRate;
    short blockAlign;
    short bitsPerSample;
    char subchunk2ID[4];
    int subchunk2Size;
} WAVHeader;

// Adaptive LMS Filter
void lms_filter(short *desired, short *reference, short *output, int numSamples) {
    float w = 0.0;  // Filter weight
    float error, y;
    
    for (int i = 0; i < numSamples; i++) {
        y = w * reference[i];   // Filtered output
        error = desired[i] - y; // Error signal
        w += MU * error * reference[i]; // Weight update
        output[i] = (short) error;
    }
}

// Read WAV file
short *read_wav(const char *filename, WAVHeader *header, int *numSamples) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening input file!\n");
        return NULL;
    }

    fread(header, sizeof(WAVHeader), 1, file);
    *numSamples = header->subchunk2Size / sizeof(short);
    
    short *data = (short *)malloc(*numSamples * sizeof(short));
    fread(data, sizeof(short), *numSamples, file);
    fclose(file);
    return data;
}

// Write WAV file
void write_wav(const char *filename, WAVHeader *header, short *data, int numSamples) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error opening output file!\n");
        return;
    }

    fwrite(header, sizeof(WAVHeader), 1, file);
    fwrite(data, sizeof(short), numSamples, file);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <desired.wav> <noise.wav> <output.wav>\n", argv[0]);
        return 1;
    }

    WAVHeader header;
    int numSamplesDesired, numSamplesNoise;

    // Read desired signal (speech + noise)
    short *desired = read_wav(argv[1], &header, &numSamplesDesired);
    if (!desired) return 1;

    // Read reference noise signal
    short *reference = read_wav(argv[2], &header, &numSamplesNoise);
    if (!reference || numSamplesDesired != numSamplesNoise) {
        printf("Error: Mismatched file sizes!\n");
        free(desired);
        return 1;
    }

    // Allocate memory for output
    short *output = (short *)malloc(numSamplesDesired * sizeof(short));
    
    // Apply LMS adaptive filter
    lms_filter(desired, reference, output, numSamplesDesired);

    // Write output WAV file
    write_wav(argv[3], &header, output, numSamplesDesired);

    // Clean up
    free(desired);
    free(reference);
    free(output);

    printf("Noise cancellation completed. Output saved to %s\n", argv[3]);
    return 0;
}
