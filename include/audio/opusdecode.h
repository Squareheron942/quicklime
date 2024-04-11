#pragma once

#include <3ds.h>
#include "audiodecode.h"

#include <opusfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "console.h"

// namespace {
    struct opusthreadargs {
        OggOpusFile* OpusFile;
        LightEvent* event;
        volatile bool *quit;
        unsigned char channel;
        int samplesperbuf, channelspersample;
        ndspWaveBuf *waveBufs;
    };
// }

class OpusDecode : public AudioDecode {
    OggOpusFile *opusFile;
    opusthreadargs args;

    public:
    OpusDecode(std::string file);
    virtual ~OpusDecode();
};