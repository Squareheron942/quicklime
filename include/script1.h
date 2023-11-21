#pragma once

#include "script.h"
#include "componentmanager.h"
#include "transform.h"
#include "console.h"
#include "controls.h"

// example of script usage
class Script1 : public Script {
    using Script::Script;
    public:
    int n_iter;

    void Start() {
        n_iter = 0;
        Console::log("Script1 started\n");
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