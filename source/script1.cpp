#include "componentmanager.h"
#include "script1.h"
#include "transform.h"
#include <stdio.h>

void Script1::Update() {
    n_iter++;
    transform* t = GetComponent<transform>();
    // if (t) printf("position: %f %f %f\n", t->position.x++, t->position.y, t->position.y);
    // printf("script1 update number %u\n", n_iter);
}

void Script1::Start() {
    n_iter = 0;
    printf("script1 started\n");
};
void Script1::FixedUpdate() {};

COMPONENT_REGISTER(Script1)