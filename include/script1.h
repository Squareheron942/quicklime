#pragma once

#include "script.h"
#include "componentmanager.h"
#include "transform.h"
#include "console.h"
#include "controls.h"
#include "gameobject.h"

// example of script usage
class Script1 : public Script {
    using Script::Script;
    public:
    int n_iter;
    GameObject* s = NULL;

    void Start() {
        n_iter = 0;
        Console::log("Script1 started\n");

        s = find("script1object");
        if (!s) Console::warn("did not find it for some reason");
        else Console::success("script1object found WWWWWWWWW");
    };

    void Update() {
    
        transform* t = GetComponent<transform>();
        if (!t)
            Console::warn("Transform component not found by Script1");
        if (controls::getDown("a")) {
            n_iter++;
            Console::log("script1 update number %u\n", n_iter);
        }
    }

    void FixedUpdate() {};
};

COMPONENT_REGISTER(Script1)