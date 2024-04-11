#pragma once

#include "script.h"
#include "componentmanager.h"
#include "transform.h"
#include "console.h"
#include "controls.h"
#include "gameobject.h"
#include "stats.h"
#include "audiosource.h"

// example of script usage
class TestScript : public Script {
    using Script::Script;
    public:
    int n_iter;
    GameObject* s = NULL;
    transform* t = NULL;
    AudioSource* player = NULL;

    void Start() {
        n_iter = 0;
        Console::log("TestScript started\n"); 

        player = GetComponent<AudioSource>();

        t = GetComponent<transform>();
    };

    void Update() {

        if (controls::getDown(controls::key::KEY_A)) {
            if (player) player->Play("romfs:/assets/ember.opus");
            Console::log("Opus playback started");
        } else if (controls::getDown(controls::key::KEY_B)) {
            if (player) player->Play("romfs:/assets/banger.mp3");
            Console::log("MP3 playback started");
        } else if (controls::getDown(controls::key::KEY_X)) {
            if (player) player->Play("romfs:/assets/guitar.ogg");
            Console::log("Vorbis playback started");
        }
    }

    void OnEnable() {
        Console::log("TestScript was enabled");
    }
};

COMPONENT_REGISTER(TestScript)