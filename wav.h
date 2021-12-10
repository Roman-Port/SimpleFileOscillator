#pragma once

#define WAV_HEADER_SIZE 44

bool read_wav_header(unsigned char buffer[WAV_HEADER_SIZE], short* channels, short* bitsPerSample, int* sampleRate);