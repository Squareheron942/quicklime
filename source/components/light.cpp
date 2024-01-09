#include "light.h"
#include "lights.h"

Light::Light() {
    C3D_LightInit(&_light, &lights::lightenv);
}