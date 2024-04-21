#pragma once

#include <string>
#include <3ds.h>

class GameObject;
class AudioDecode;

class AudioSource {
	LightLock lock;
    std::string audio_clip;
    uint16_t flags = 0;
    AudioDecode* decoder = NULL;

    public:
    AudioSource(GameObject& parent, const void* data);
    ~AudioSource();

    inline const bool stereo(void) { return flags & BIT(1); }
    inline const bool spatial(void) { return flags & BIT(2); }

    void Play(std::string file);
};