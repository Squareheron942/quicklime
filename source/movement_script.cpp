#include "movement_script.h"
#include "transform.h"
#include "controls.h"
#include "sl_time.h"
#include "componentmanager.h"

// ComponentManager::map.insert({"MovementScript", ComponentManager::createInstance<MovementScript>});

void MovementScript::Update() {
    transform* t = GetComponent<transform>();
    t->rotation.x = controls::gyroPos().x;
	t->rotation.y = -controls::gyroPos().z;
	// float angle = atan2f(abs(controls::circlePos().dx) > 20 ? controls::circlePos().dx : 0, abs(controls::circlePos().dy) > 20 ? controls::circlePos().dy : 0) + cam->rotation.y;
	// float invmagnitude = 1 / sqrtf(controls::circlePos().dx * controls::circlePos().dx + controls::circlePos().dy * controls::circlePos().dy);

    t->position.x += (abs(controls::circlePos().dx) > 20 ? controls::circlePos().dx : 0) * Time::deltaTime * 0.005f;
    t->position.z += (abs(controls::circlePos().dy) > 20 ? controls::circlePos().dy : 0) * Time::deltaTime * 0.005f;
	t->position.y += (controls::getHeld("L") ? 128 : controls::getHeld("R") ? -128 : 0) * Time::deltaTime * 0.005f;
}

COMPONENT_REGISTER(MovementScript)