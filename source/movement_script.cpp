#include "movement_script.h"
#include "transform.h"
#include "controls.h"
#include "sl_time.h"
#include "componentmanager.h"

void MovementScript::Update() {
    transform* t = GetComponent<transform>();
    t->rotation.x = controls::gyroPos().x;
	t->rotation.y = -controls::gyroPos().z;

    t->position.x += (abs(controls::circlePos().dx) > 20 ? controls::circlePos().dx : 0) * Time::deltaTime * 0.005f;
    t->position.z += (abs(controls::circlePos().dy) > 20 ? controls::circlePos().dy : 0) * Time::deltaTime * 0.005f;
	t->position.y += (controls::getHeld("L") ? 128 : controls::getHeld("R") ? -128 : 0) * Time::deltaTime * 0.005f;
}

COMPONENT_REGISTER(MovementScript)