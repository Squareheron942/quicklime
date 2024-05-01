#include "light.h"
#include "lights.h"
#include <citro3d.h>
#include <3ds.h>
#include "assert.h"
#include "console.h"
#include "threads.h"

LightLock Light::lock = LightLock();

Light::Light() {
	LightLock_Guard l(lock);
    if (lights::lightenvneedsupdating) {
        C3D_LightEnvInit(&lights::lightenv);
        C3D_LightEnvBind(&lights::lightenv);
        lights::lightenvneedsupdating = false;
    }
    ++lights::n;
    assert(lights::n < HW_MAX_LIGHTS);

    for (unsigned char i = 0; i < HW_MAX_LIGHTS; ++i) {
        if (lights::active[i] == nullptr) {
            index = i;
            lights::active[i] = this;
            light = &lights::lights[index];
            break;
        }
    }

    position = FVec4_New(0.0f, 0.0f, 0.0f, 1.0f);

    C3D_LightInit(light, &lights::lightenv);
    C3D_LightColor(light, 0.992, 0.984, 0.827);
    C3D_LightPosition(light, &position);
    Console::log("Light %u created", index);
}

void Light::update(C3D_Mtx *view) {
	LightLock_Guard l(lock);
    _tp = Mtx_MultiplyFVec4(view, position);
    C3D_LightPosition(&lights::lights[index], &_tp);
};

Light::~Light() {
	LightLock_Guard l(lock);
    lights::active[index] = NULL;
    --lights::n;
    Console::log("Light %u deleted", index);
}