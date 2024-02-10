#include "audiodecode.h"
#include "console.h"
#include "defines.h"

namespace {
    void audioCallback(void *const data) {
        if(((AudioDecode*)data)->quit) { // Quit flag
            #if DEBUG
            Console::warn("Audio callback quit");
            #endif
            return;
        }
        
        LightEvent_Signal(&((AudioDecode*)data)->event);
    }
}


bool AudioDecode::AudioInit(unsigned int samplerate, unsigned int samplesperbuf, unsigned char channelspersample) {
    // if no available channel, audio cannot be played so end here
    if (!foundchannel) return false;

    ndspChnReset(channel);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(channel, interpolation);
    ndspChnSetRate(channel, samplerate);
    ndspChnSetFormat(channel, NDSP_FORMAT_STEREO_PCM16);

    // Allocate audio buffer
    const size_t bufferSize = samplesperbuf * channelspersample * AUDIO_NUM_WAVBUFS;
    audioBuffer = linearAlloc(bufferSize);
    if(!audioBuffer) {
        Console::error("Failed to allocate audio buffer\n");
        return false;
    }

    // Setup waveBufs for NDSP
    memset(&waveBufs, 0, sizeof(waveBufs));
    int16_t *buffer = (int16_t*)audioBuffer;

    for(size_t i = 0; i < 3; ++i) {
        waveBufs[i].data_vaddr = buffer;
        waveBufs[i].status     = NDSP_WBUF_DONE;

        buffer += samplesperbuf * channelspersample / sizeof(buffer[0]);
    }

    #if DEBUG
    Console::log("Audio initialization");
    #endif

    return true;
}

void AudioDecode::AudioFinish() {
    audio_shared_inf::ndsp_used_channels &= ~BIT(channel);
}

AudioDecode::AudioDecode() {
    for (int i = 0; i < 24; i++) 
        if (!(BIT(i) & audio_shared_inf::ndsp_used_channels)) {
            audio_shared_inf::ndsp_used_channels |= BIT(i);
            channel = i;
            foundchannel = true;
            break;
        }
    #if DEBUG
    Console::log("Audio decoder created with chn%u", channel);
    #endif
    ndspSetCallback(audioCallback, this);
    LightEvent_Init(&event, RESET_ONESHOT);
}

AudioDecode::~AudioDecode() {
    Stop();
}