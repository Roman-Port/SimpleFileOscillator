#include "wav.h"
#include <stdint.h>
#include <cassert>
#include <cstring>

template <class T>
T _wav_read(unsigned char** buffer, size_t* remaining) {
    //Make sure we have enough space remaining
    assert(*remaining >= sizeof(T));

    //Read
    T value;
    memcpy(&value, *buffer, sizeof(T));

    //Update state
    (*buffer) += sizeof(T);
    (*remaining) -= sizeof(T);

    return value;
}

bool read_wav_header(unsigned char buffer[WAV_HEADER_SIZE], short* channels, short* bitsPerSample, int* sampleRate) {
    //Read all
    size_t remaining = WAV_HEADER_SIZE;
    uint32_t tagRiff = _wav_read<uint32_t>(&buffer, &remaining);
    int32_t length = _wav_read<int32_t>(&buffer, &remaining);
    uint32_t tagWave = _wav_read<uint32_t>(&buffer, &remaining);
    uint32_t tagFmt = _wav_read<uint32_t>(&buffer, &remaining);
    int32_t fmtLength = _wav_read<int32_t>(&buffer, &remaining);
    int16_t formatTag = _wav_read<int16_t>(&buffer, &remaining);
    *channels = _wav_read<int16_t>(&buffer, &remaining);
    *sampleRate = _wav_read<int32_t>(&buffer, &remaining);
    int32_t avgBytesPerSec = _wav_read<int32_t>(&buffer, &remaining);
    int16_t blockAlign = _wav_read<int16_t>(&buffer, &remaining);
    *bitsPerSample = _wav_read<int16_t>(&buffer, &remaining);
    uint32_t tagData = _wav_read<uint32_t>(&buffer, &remaining);
    int32_t dataLen = _wav_read<int32_t>(&buffer, &remaining);
    assert(remaining == 0);

    //Validate tags
    if (tagRiff != 1179011410 || tagWave != 1163280727 || tagFmt != 544501094)
        return false;

    return true;
}