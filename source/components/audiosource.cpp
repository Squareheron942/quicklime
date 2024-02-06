#include "audiosource.h"
#include "componentmanager.h"
#include "gameobject.h"
#include "opusdecode.h"
#include "aacdecode.h"
#include "mp3decode.h"
#include "bcstmdecode.h"
#include "wavdecode.h"
#include <3ds.h>
#include <string>

namespace {
    struct AudioSource_args {
        uint16_t flags;
    };
}

AudioSource::AudioSource(GameObject& parent, const void* data) {
    AudioSource_args args;
    if (data) args = *(AudioSource_args*)data;

    Console::log("AudioSource constructor");
}

void AudioSource::Play(std::string file) {
    std::string extension = file.substr(file.find_last_of(".") + 1);

    Console::log("AudioSource playing");
    if (extension == "ogg" || extension == "opus") {
        decoder = new OpusDecode(file);
    } else if (extension == "mp3") {

    } else if (extension == "bcstm") {

    } else if (extension == "wav" || extension == "wave") {

    }
    Console::log("Decoder created");
    Console::log("channels used %u");
    decoder->Play();
}

COMPONENT_REGISTER(AudioSource)