#pragma once

#include "light.h"
#include <citro3d.h>

class SpotLight : public Light {
    SpotLight() {}
    public:
    SpotLight(void* params);
};