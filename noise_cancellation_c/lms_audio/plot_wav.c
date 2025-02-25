//plotting of the .wav files:
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define WAV_HEADER_SIZE 44  // Standard WAV file header size

void plot_waveform(const char *data_file) {
    //FILE *gnuplot = popen("gnuplot -persistent", "w");
    FILE *gnuplot = popen("\"D:\\gnuplot\\bin\\gnuplot.exe\" -persistent", "w");
    if (!gnuplot) {
        fprintf(stderr, "Error opening GNUplot.\n");
        return;
    }
    fprintf(gnuplot, "set title 'WAV File Waveform'\n");
    fprintf(gnuplot, "set xlabel 'Sample Index'\n");
    fprintf(gnuplot, "set ylabel 'Amplitude'\n");
    fprintf(gnuplot, "plot '%s' with lines title 'Audio Waveform'\n", data_file);
    pclose(gnuplot);
}

void read_wav_data(const char *wav_filename, const char *output_data_filename) {
    FILE *wav_file = fopen(wav_filename, "rb");
    if (!wav_file) {
        fprintf(stderr, "Error opening WAV file.\n");
        return;
    }

    FILE *data_file = fopen(output_data_filename, "w");
    if (!data_file) {
        fprintf(stderr, "Error creating output data file.\n");
        fclose(wav_file);
        return;
    }

    // Skip WAV header
    fseek(wav_file, WAV_HEADER_SIZE, SEEK_SET);

    int16_t sample;
    int index = 0;

    // Read samples and write them to the output file
    while (fread(&sample, sizeof(int16_t), 1, wav_file)) {
        fprintf(data_file, "%d %d\n", index++, sample);
    }

    fclose(wav_file);
    fclose(data_file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <wav_file>\n", argv[0]);
        return 1;
    }

    const char *wav_filename = argv[1];
    const char *data_filename = "waveform.dat";

    // Read WAV file and generate data
    read_wav_data(wav_filename, data_filename);

    // Plot waveform using GNUplot
    plot_waveform(data_filename);

    return 0;
}
