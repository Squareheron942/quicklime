#pragma once

#include <citro3d.h>

class Light {
    protected:
    Light();
    public:
    C3D_Light _light;
    C3D_LightLut _lut;
};