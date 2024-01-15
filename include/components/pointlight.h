#pragma once

#include "light.h"
#include <citro3d.h>

class PointLight : public Light {
    PointLight() = delete;
    C3D_LightLutDA _lda;
    public:
    PointLight(const void* params);
    PointLight(float angle);
    void update(C3D_Mtx* view) override;
};