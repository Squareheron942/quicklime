#pragma once

#include "script.h"
#include "componentmanager.h"

class MovementScript : public Script {
    using Script::Script;
    public:
    void Start() {};
    void Update();
    void FixedUpdate() {};
};