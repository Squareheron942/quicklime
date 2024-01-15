#pragma once

#include "light.h"
#include <citro3d.h>

class SpotLight : public Light {
    SpotLight() = delete;
    C3D_FVec _td;
    public:
    C3D_FVec direction;
    SpotLight(const void* params);
    SpotLight(float angle);
    void update(C3D_Mtx* view) override;
};