#include "audiodecode.h"
#include "console.h"

namespace {
    void audioCallback(void *const data) {
        if(((AudioDecode*)data)->quit) { // Quit flag
            Console::warn("Audio callback quit");
            return;
        }
        
        LightEvent_Signal(&((AudioDecode*)data)->event);
    }
}


bool AudioDecode::AudioInit(audio_params params) {
    // if no available channel, audio cannot be played so end here
    if (!foundchannel) return false;

    ndspChnReset(channel);
    ndspChnSetInterp(channel, interpolation);
    ndspChnSetRate(channel, params.samplerate);
    ndspChnSetFormat(channel, NDSP_FORMAT_STEREO_PCM16);

    Console::log("Audio initialization");

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
    Console::log("Set audio callback");
    ndspSetCallback(audioCallback, this);
    LightEvent_Init(&event, RESET_ONESHOT);
}

AudioDecode::~AudioDecode() {
    quit = true;
    // Free the audio thread
    threadJoin(threadId, UINT64_MAX);
    threadFree(threadId);
}