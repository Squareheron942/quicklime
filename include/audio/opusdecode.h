#pragma once

#include <3ds.h>
#include "audiodecode.h"

#include <opusfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "console.h"

namespace {
    struct opusthreadargs {
        OggOpusFile* OpusFile;
        LightEvent* event;
        volatile bool *quit;
        int samplesperbuf, channelspersample;
        ndspWaveBuf *waveBufs;
    };
}

class OpusDecode : public AudioDecode {
    OggOpusFile *opusFile;
    opusthreadargs args;
    int16_t *audioBuffer = NULL;

    public:
    OpusDecode(audio_params params);
    OpusDecode(std::string file);
    ~OpusDecode();

    virtual void Play(audio_params params);
    virtual void Play();
};