#pragma once

#include "script.h"

// example of script usage
class Script1 : public Script {
    public:
    using Script::Script;
    void Start();
    void Update();
    void FixedUpdate();
    int n_iter;
};

extern bool Script1_component;