#pragma once


#include <mpg123.h>
#include "audiodecode.h"

namespace {
    struct mp3threadargs {
        mpg123_handle* file;
        LightEvent* event;
        volatile bool *quit;
        unsigned char channel;
        int samplesperbuf, channelspersample;
        ndspWaveBuf *waveBufs;
    };
    
}

class MP3Decode : public AudioDecode {
    FILE* file;
	mpg123_handle* mh;
    int channels;
    public:
    MP3Decode(std::string file);
    virtual ~MP3Decode();
};