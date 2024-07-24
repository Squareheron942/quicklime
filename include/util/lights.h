#pragma once

#include <3ds.h>
#include <citro3d.h>

#define HW_MAX_LIGHTS 8

class Light;
namespace lights {
    extern C3D_LightEnv lightenv;
    extern C3D_Light lights[HW_MAX_LIGHTS];
    extern Light *active[HW_MAX_LIGHTS];
    extern unsigned char n;
    extern bool lightenvneedsupdating;
}
