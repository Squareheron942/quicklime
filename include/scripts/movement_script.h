#pragma once

#include "script.h"
#include "componentmanager.h"
#include "movement_script.h"
#include "transform.h"
#include "controls.h"
#include "sl_time.h"
#include "console.h"
#include <algorithm>

#define MS_HALF_PI 1.5707963268
#define MS_DEG_RAD 0.0174532925

class MovementScript : public Script {
    using Script::Script;
    public:
    float lerpTimer, yrot;
    transform* t;

    void Start() {
        lerpTimer = 0.f;
        yrot = 0.f;
        t = GetComponent<transform>();
    }

    void Update() {
        angularRate g_rate = controls::gyroRate();
        lerpTimer -= Time::deltaTime;

        yrot = t->eulerAngles().y;

        float x = (abs(controls::circlePos().dx) > 20 ? controls::circlePos().dx : 0) * Time::deltaTime * 0.005f;
        float y = (abs(controls::circlePos().dy) > 20 ? controls::circlePos().dy : 0) * Time::deltaTime * 0.005f;

        float s = sinf(yrot), c = cosf(yrot);

        t->position.x -= x * c - y * s;
        t->position.z -= x * s + y * c;
        t->position.y -= (controls::getHeld("L") ? 0.64 : controls::getHeld("R") ? -0.64 : 0) * Time::deltaTime;

        if (controls::getDown("a")) Console::log("A pressed");
        if (controls::getDown("b")) Console::log("B pressed");
        if (controls::getDown("y")) lerpTimer = 0.5f; // time in seconds to take when recentering
        
        if (lerpTimer > 0.3) t->rotation = transform::slerp({1, 0, 0, 0}, t->rotation, lerpTimer); // lerp towards player direction instefd

        t->rotateY(-(abs(g_rate.z) > controls::gyroDeadZone() ? g_rate.z : 0) * Time::deltaTime * MS_DEG_RAD * controls::gyroSensitivity(), false);
        t->rotateX((abs(g_rate.x) > controls::gyroDeadZone() ? g_rate.x : 0) * Time::deltaTime * MS_DEG_RAD * controls::gyroSensitivity());
    }

    void FixedUpdate() {};
};

COMPONENT_REGISTER(MovementScript)