#include "audiosource.h"
#include "componentmanager.h"
#include "gameobject.h"
#include "opusdecode.h"
#include "aacdecode.h"
#include "mp3decode.h"
#include "bcstmdecode.h"
#include "wavdecode.h"
#include "vorbisdecode.h"
#include <3ds.h>
#include <string>
#include "threads.h"

namespace {
    struct AudioSource_args {
        uint16_t flags;
    };
}

AudioSource::AudioSource(GameObject& parent, const void* data) {
	LightLock_Init(&lock);
    AudioSource_args args;
    if (data) args = *(AudioSource_args*)data;

    Console::log("AudioSource constructor");
}

AudioSource::~AudioSource() {
	LightLock_Guard l(lock);
	if (decoder) delete decoder;
}

void AudioSource::Play(std::string file) {
	LightLock_Guard l(lock);
    std::string extension = file.substr(file.find_last_of(".") + 1);

    if (decoder) delete decoder;

    Console::log("AudioSource playing");

    /*
    	Supported formats:
     	  - opus
      	  - mp3
       	to do:
       	  - bcstm
          - wav
          - vorbis
          - aac
    */
    if (extension == "opus") {
        decoder = new OpusDecode(file);
    } else if (extension == "mp3") {
        decoder = new MP3Decode(file);
    } else if (extension == "bcstm") {

    } else if (extension == "wav" || extension == "wave") {

    } else if (extension == "ogg" || extension == "oga") {
        decoder = new VorbisDecode(file);
    }

    Console::log("Decoder created");
    Console::log("channels used %u");
    // decoder->Play(file);
}

COMPONENT_REGISTER(AudioSource)