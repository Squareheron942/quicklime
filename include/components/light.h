#pragma once

#include <citro3d.h>

class Light {
    protected:
    C3D_FVec _tp; // transformed position
    public:
    C3D_LightLut _lut;
    Light();
    ~Light();
    C3D_Light* light;
    C3D_FVec position;
    virtual void update(C3D_Mtx* view);
    unsigned char index;
};