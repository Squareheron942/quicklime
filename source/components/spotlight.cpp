#include "spotlight.h"
#include <citro3d.h>
#include "lights.h"

namespace {
    struct spotlight_args { // pretty much just left it at what I think are decently reasonable settings
        float spread_angle = 1.1780972451f;
        float color[4] = {0.992, 0.984, 0.827, 1.f};
        C3D_FVec direction = {1.f, 0.5773502692f, 0.5773502692f, 0.5773502692f}; // pointing down, forward, left
        C3D_FVec position = {1.f, 0.f, 0.f, 0.f};
    };
}

SpotLight::SpotLight(const void* args) {
    spotlight_args formatted_args;
    if (args) formatted_args = *(spotlight_args*)args;
    C3D_LightSpotEnable(light, true);
    LightLut_Spotlight(&_lut, formatted_args.spread_angle);
    C3D_LightColor(light, formatted_args.color[0], formatted_args.color[1], formatted_args.color[2]);
    direction = formatted_args.direction;
    position = formatted_args.position;

}

SpotLight::SpotLight(float angle) {
    spotlight_args formatted_args;
    C3D_LightSpotEnable(light, true);
    LightLut_Spotlight(&_lut, angle);
    C3D_LightSpotLut(light, &_lut);
    direction = formatted_args.direction;
    C3D_LightSpotDir(light, direction.x, direction.y, direction.z);
    position = {1, 0, 100, 0};
    C3D_LightPosition(light, &position);
    C3D_LightColor(light, formatted_args.color[0], formatted_args.color[1], formatted_args.color[2]);
}

void SpotLight::update(C3D_Mtx *view) {
    _tp = Mtx_MultiplyFVec4(view, position);
    _td = Mtx_MultiplyFVec3(view, direction);
    C3D_LightPosition(light, &_tp);
    C3D_LightSpotDir(light, _td.y, _td.y, _td.z);
};