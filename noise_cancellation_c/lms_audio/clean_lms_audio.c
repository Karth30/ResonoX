#include <stdio.h>
#include <stdlib.h>

#define N 128   // Number of filter coefficients
#define MU 0.01 // Step size

// WAV file header (44 bytes)
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

void adaptive_noise_cancellation(short *x, short *d, float *w, short *e, int length) {
    float y;
    
    for (int n = N - 1; n < length; n++) {
        y = 0.0;
        
        // Compute filter output (estimated noise)
        for (int i = 0; i < N; i++) {
            y += w[i] * x[n - i];
        }

        e[n] = d[n] - (short)y; // Error signal (clean audio)

        // Update filter weights
        for (int i = 0; i < N; i++) {
            w[i] += MU * e[n] * x[n - i];
        }
    }
}

int main() {
    FILE *noisyFile, *noiseFile, *outputFile;
    WAVHeader header;

    // Open noisy WAV file
    noisyFile = fopen("D:\\Downloads\\noise_cancellation_c\\lms_audio\\noisy_audio.wav", "rb");
    noiseFile = fopen("converted_audio.wav", "rb");

    if (!noisyFile || !noiseFile) {
        printf("Error opening input files!\n");
        return -1;
    }

    // Read WAV header
    fread(&header, sizeof(WAVHeader), 1, noisyFile);
    fseek(noiseFile, 44, SEEK_SET); // Skip header for noise file

    // Calculate number of samples
    int length = header.subchunk2Size / (header.bitsPerSample / 8);

    // Allocate memory
    short *noisySignal = (short *)malloc(length * sizeof(short));
    short *noiseSignal = (short *)malloc(length * sizeof(short));
    short *filteredSignal = (short *)malloc(length * sizeof(short));
    float w[N] = {0}; // Adaptive filter weights

    // Read audio samples
    fread(noisySignal, sizeof(short), length, noisyFile);
    fread(noiseSignal, sizeof(short), length, noiseFile);

    fclose(noisyFile);
    fclose(noiseFile);

    // Apply LMS noise cancellation
    adaptive_noise_cancellation(noiseSignal, noisySignal, w, filteredSignal, length);

    // Write cleaned output as WAV
    outputFile = fopen("cleaned_audio.wav", "wb");
    fwrite(&header, sizeof(WAVHeader), 1, outputFile); // Write WAV header
    fwrite(filteredSignal, sizeof(short), length, outputFile);
    fclose(outputFile);

    // Cleanup
    free(noisySignal);
    free(noiseSignal);
    free(filteredSignal);

    printf("Noise removed! Output saved as 'cleaned_audio.wav'.\n");
    return 0;
}
