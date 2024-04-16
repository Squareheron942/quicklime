#include "audiodecode.h"
#include "console.h"
#include "defines.h"
#include "stats.h"

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


bool AudioDecode::AudioInit(unsigned int samplerate, unsigned char channels, unsigned int bufsize) {
    // if no available channel, audio cannot be played so end here
    if (!foundchannel) return false;

    ndspChnReset(channel);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(channel, interpolation);
    ndspChnSetRate(channel, samplerate);
    ndspChnSetFormat(channel, channels == 2 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);

    // Allocate audio buffer
    const size_t bufferSize = bufsize * AUDIO_NUM_WAVBUFS * sizeof(ndspWaveBuf);
    audioBuffer = linearAlloc(bufferSize);
    stats::linear += bufferSize;
    if(!audioBuffer) {
        Console::error("Failed to allocate audio buffer");
        return false;
    }

    // Setup waveBufs for NDSP
    memset(&waveBufs, 0, sizeof(waveBufs));
    int16_t *buffer = (int16_t*)audioBuffer;

    for(size_t i = 0; i < AUDIO_NUM_WAVBUFS; ++i) {
        waveBufs[i].data_vaddr = buffer;
        waveBufs[i].status     = NDSP_WBUF_DONE;

        buffer += bufsize / sizeof(buffer[0]);
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

void AudioDecode::Stop() {
	if (quit) return; // if it has already been run then don't do it again
    quit = true;
    LightEvent_Signal(&event);
    // Free the audio thread
    threadJoin(threadId, UINT64_MAX);
    threadFree(threadId);
    ndspChnReset(channel);
    audio_shared_inf::ndsp_used_channels &= ~BIT(channel);
    if (audioBuffer) {
        stats::linear -= linearGetSize(audioBuffer);
        linearFree(audioBuffer);
        audioBuffer = NULL;
        Console::log("Audio buffer freed");
    } else {
        Console::error("No buffer");
    }
}

AudioDecode::~AudioDecode() {
    Stop();
    Console::log("Audio decoder destroyed");
}