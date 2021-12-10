#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <volk/volk.h>
#include "wav.h"

#define BUFFER_SIZE 65536

bool open_files(char* inputFilename, char* outputFilename, FILE** inputArg, FILE** outputArg, int* sampleRate) {
	//Open input file
	FILE* input = fopen(inputFilename, "rb");
	if (input == nullptr) {
		printf("Failed to open input file: %s\n", inputFilename);
		return false;
	}

	//Open output file
	FILE* output = fopen(outputFilename, "wb");
	if (output == nullptr) {
		printf("Failed to open output file: %s\n", outputFilename);
		return false;
	}

	//Read the WAV file header
	unsigned char header[WAV_HEADER_SIZE];
	if (fread(header, 1, WAV_HEADER_SIZE, input) != WAV_HEADER_SIZE) {
		printf("Unable to read from input file.\n");
		return false;
	}

	//Parse WAV header and do some checks on it
	short channels;
	short bitsPerSample;
	if (!read_wav_header(header, &channels, &bitsPerSample, sampleRate)) {
		printf("Invalid input file: Failed to parse WAV header.\n");
		return false;
	}
	if (channels != 2) {
		printf("Invalid input file: IQ files must have two channels, this file has %i!\n", channels);
		return false;
	}
	if (bitsPerSample != 16) {
		printf("Invalid input file: This program only supports PCM-16 files. Format %i is currently unsupported.\n", bitsPerSample);
		return false;
	}
	if (*sampleRate <= 0) {
		printf("Invalid input file: The sample rate %i is invalid.\n", *sampleRate);
		return false;
	}

	//Copy the source WAV file header to the output. This is a janky hack, but it'll work for a simple program like this...
	if (fwrite(header, 1, WAV_HEADER_SIZE, output) != WAV_HEADER_SIZE) {
		printf("Failed to write WAV header to output file. Out of disk space?\n");
		return false;
	}

	//Success. Set output
	(*inputArg) = input;
	(*outputArg) = output;
	return true;
}

int main(int argc, char** argv)
{
	//Make sure we have all of the correct args and parse the frequency offset
	int freqOffset;
	if (argc != 4 || sscanf(argv[3], "%i", &freqOffset) != 1) {
		printf("Simple File Oscillator by RomanPort - Invalid Usage\n\nUsage: %s [input filename] [output filename] [frequency offset (Hz)]", argv[0]);
		return -1;
	}

	//Open all the files
	FILE* input;
	FILE* output;
	int sampleRate;
	if (!open_files(argv[1], argv[2], &input, &output, &sampleRate))
		return -1;

	//Allocate buffers
	size_t alignment = volk_get_alignment();
	lv_16sc_t* buffer_file = (lv_16sc_t*)volk_malloc(sizeof(lv_16sc_t) * BUFFER_SIZE, alignment);
	lv_32fc_t* buffer_main = (lv_32fc_t*)volk_malloc(sizeof(lv_32fc_t) * BUFFER_SIZE, alignment);
	if (buffer_file == nullptr || buffer_main == nullptr) {
		printf("Failed to allocate memory.\n");
		return -1;
	}

	//Configure rotator
	double angle = 2.0 * 3.14159265359 * freqOffset / sampleRate;
	lv_32fc_t inc(std::cos(angle), std::sin(angle));
	lv_32fc_t phase(1, 0);

	//Enter loop
	int count;
	do {
		//Read from file
		count = fread(buffer_file, sizeof(lv_16sc_t), BUFFER_SIZE, input);

		//Convert to floats
		volk_16ic_convert_32fc(buffer_main, buffer_file, count);

		//Process (this is where the real work is)
		volk_32fc_s32fc_x2_rotator_32fc(buffer_main, buffer_main, inc, &phase, count);

		//Convert to short
		volk_32fc_convert_16ic(buffer_file, buffer_main, count);

		//Write to output
		if (fwrite(buffer_file, sizeof(lv_16sc_t), count, output) != count) {
			printf("Failed to write to disk. Out of disk space?\n");
			break;
		}
	} while (count != 0);

	//Close the files
	fclose(input);
	fclose(output);

	//Done
	printf("Finished processing file!\n");

	return 0;
}