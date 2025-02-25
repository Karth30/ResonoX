#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sndfile.h"  // For audio file handling

#define N 128   // Number of filter coefficients
#define MU 0.01 // Step size

void adaptive_noise_cancellation(float *x, float *d, float *w, float *e, int length) {
    float y;
    
    for (int n = N - 1; n < length; n++) {
        y = 0.0;
        
        // Compute filter output (estimated noise)
        for (int i = 0; i < N; i++) {
            y += w[i] * x[n - i];
        }

        e[n] = d[n] - y; // Error signal (clean audio)

        // Update filter weights
        for (int i = 0; i < N; i++) {
            w[i] += MU * e[n] * x[n - i];
        }
    }
}

int main() {
    // File input/output variables
    SNDFILE *inputFile, *noiseFile, *outputFile;
    SF_INFO sfinfo;

    // Open noisy audio file
    inputFile = sf_open("noisy_audio.wav", SFM_READ, &sfinfo);
    if (!inputFile) {
        printf("Error opening noisy audio file!\n");
        return -1;
    }

    // Open noise reference file
    noiseFile = sf_open("noise_signal.wav", SFM_READ, &sfinfo);
    if (!noiseFile) {
        printf("Error opening noise reference file!\n");
        sf_close(inputFile);
        return -1;
    }

    // Allocate memory for audio processing
    int length = sfinfo.frames;
    float *noisySignal = (float *)malloc(length * sizeof(float));
    float *noiseSignal = (float *)malloc(length * sizeof(float));
    float *filteredSignal = (float *)malloc(length * sizeof(float));
    float w[N] = {0}; // Adaptive filter weights

    // Read audio samples
    sf_readf_float(inputFile, noisySignal, length);
    sf_readf_float(noiseFile, noiseSignal, length);

    // Apply Adaptive Noise Cancellation (ANC)
    adaptive_noise_cancellation(noiseSignal, noisySignal, w, filteredSignal, length);

    // Write output cleaned audio
    outputFile = sf_open("cleaned_audio.wav", SFM_WRITE, &sfinfo);
    if (!outputFile) {
        printf("Error creating output file!\n");
        return -1;
    }

    sf_writef_float(outputFile, filteredSignal, length);

    // Clean up
    sf_close(inputFile);
    sf_close(noiseFile);
    sf_close(outputFile);
    free(noisySignal);
    free(noiseSignal);
    free(filteredSignal);

    printf("Cleaned audio saved as 'cleaned_audio.wav'.\n");
    return 0;
}
