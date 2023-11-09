#include "componentmanager.h"
#include "script1.h"
#include "transform.h"
#include "console.h"
#include "controls.h"

void Script1::Update() {
    
    transform* t = GetComponent<transform>();
    if (!t)
        Console::warn("Transform component not found by Script1");
    // if (t) printf("position: %f %f %f\n", t->position.x++, t->position.y, t->position.y);
    if (controls::getDown("a")) {
        n_iter++;
        Console::log("script1 update number %u\n", n_iter);
    }
}

void Script1::Start() {
    n_iter = 0;
    Console::log("script1 started\n");
};
void Script1::FixedUpdate() {};

COMPONENT_REGISTER(Script1)