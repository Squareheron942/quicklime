#pragma once


#include <mpg123.h>
#include "audiodecode.h"

namespace {
    struct mp3threadargs {
        mpg123_handle* mh;
        LightEvent* event;
        volatile bool *quit;
        unsigned char channel;
        int bufsize, channels;
        ndspWaveBuf *waveBufs;
    };
    
}

class MP3Decode : public AudioDecode {
    FILE* file;
	mpg123_handle* mh;
    mp3threadargs args;
    int channels;
    public:
    MP3Decode(std::string file);
    virtual ~MP3Decode();
};