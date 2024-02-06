#pragma once

#define AUDIO_NUM_WAVBUFS 3

#include <3ds.h>
#include "audiosharedinf.h"

#define THREAD_AFFINITY (-1)           // Execute thread on any core
#define THREAD_STACK_SZ (32 * 1024)    // 32kB stack for audio thread

struct audio_params {
    bool stereo : 1 = false;
    unsigned int samplerate = 48000;
};

class AudioDecode {
    protected:
    bool foundchannel = false;
    unsigned char channel = 0;
    unsigned char dataformat = 0;
    ndspInterpType interpolation = NDSP_INTERP_POLYPHASE;
    int samplerate = 0, samplesperbuf = 0;
    Thread threadId;
    bool AudioInit(audio_params params);
    void AudioFinish();
    AudioDecode();
    ~AudioDecode();
    ndspWaveBuf waveBufs[3];
    public:
    volatile bool quit = false;
    LightEvent event;
    virtual void Play(audio_params params) = 0;
    virtual void Play() = 0;

    void Stop() {
        quit = true;
    }
};