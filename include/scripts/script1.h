#pragma once

#include "script.h"
#include "componentmanager.h"
#include "transform.h"
#include "console.h"
#include "controls.h"
#include "gameobject.h"
#include "stats.h"

// example of script usage
class Script1 : public Script {
    using Script::Script;
    public:
    int n_iter;
    GameObject* s = NULL;
    transform* t = NULL;

    void Start() {
        n_iter = 0;
        Console::log("Script1 started\n");

        s = find("moveobject");

        t = s->getComponent<transform>();
        t->scale = {{1, 0.01f, 0.01f, 0.01f}};
    };

    void Update() {

        _x = t->position.x;
        _y = t->position.y;
        _z = t->position.z;

        if (controls::getDown("select")) Console::nextMenu();

        if (controls::getDown("a")) {
            n_iter++;
            Console::log("script1 update number %u\n", n_iter);
        }
    }

    void OnEnable() {
        Console::log("Script1 was enabled");
    }
};

COMPONENT_REGISTER(Script1)