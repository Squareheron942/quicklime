#pragma once

#define AUDIO_NUM_WAVBUFS 3
#define AUDIO_WAVBUF_TIME_MS 120 / 1000

#include <3ds.h>
#include "audiosharedinf.h"
#include <string>
#include "stats.h"

#define THREAD_AFFINITY (-1)           // Execute thread on any core
#define THREAD_STACK_SZ (32 * 1024)    // 32kB stack for audio thread

struct audio_params {
    bool stereo : 1 = false;
    unsigned int samplerate = 48000;
    int samplesperbuf, channelspersample;
};

class AudioDecode {
    protected:
    LightLock lock;
    bool foundchannel = false;
    unsigned char channel = 0;
    unsigned char dataformat = 0;
    ndspInterpType interpolation = NDSP_INTERP_POLYPHASE;
    unsigned int samplesperbuf = 0, bufsize = 0;
    long int samplerate = 0;
    Thread threadId;
    bool AudioInit(unsigned int samplerate, unsigned char channels, unsigned int bufsize);
    void AudioFinish();
    AudioDecode();
    ndspWaveBuf waveBufs[3];
    public:
    virtual ~AudioDecode();
    volatile bool quit = false;
    void *audioBuffer = NULL;
    LightEvent event;

    void Stop();
};