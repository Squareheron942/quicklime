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

        s = find("script1object");
        if (!s) Console::warn("did not find it for some reason");
        else Console::success("script1object found WWWWWWWWW");

        t = GetComponent<transform>();
    };

    void Update() {

        _frametime = Time::deltaTime * 1000;
        _fps = 1.0f / Time::deltaTime;
        _x = t->position.x;
        _y = t->position.y;
        _z = t->position.z;

        if (controls::getDown("select")) Console::nextMenu();
    
        if (!t)
            Console::warn("Transform component not found by Script1");
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