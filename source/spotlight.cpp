#include "spotlight.h"
#include <citro3d.h>

namespace {
    struct spotlight_args { // pretty much just left it at what I think are decently reasonable settings
        float spread_angle = 1.1780972451f;
        float color[4] = {0.992, 0.984, 0.827, 1.f};
        C3D_FVec direction = {1.f, 0.f, 0.f, 0.f};
        C3D_FVec position = {1.f, 0.f, 0.f, 0.f};
    };
}

SpotLight::SpotLight(const void* args) {
    spotlight_args formatted_args;
    if (args) formatted_args = *(spotlight_args*)args;
    C3D_LightSpotEnable(&_light, true);
    LightLut_Spotlight(&_lut, formatted_args.spread_angle);

}