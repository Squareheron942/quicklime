#include "pointlight.h"
#include <citro3d.h>
#include "lights.h"

namespace {
    struct pointlight_args { // pretty much just left it at what I think are decently reasonable settings
        float distance = 1.1780972451f;
        float color[4] = {0.992, 0.984, 0.827, 1.f};
        C3D_FVec position = {1.f, 0.f, 0.f, 0.f};
    };
}

PointLight::PointLight(const void* args) {
    pointlight_args formatted_args;
    if (args) formatted_args = *(pointlight_args*)args;
    
    LightLutDA_Quadratic(&_lda, 1, 0, 0, 1);
    C3D_LightDistAttnEnable(light, true);
    C3D_LightDistAttn(light, &_lda);

    C3D_LightColor(light, formatted_args.color[0], formatted_args.color[1], formatted_args.color[2]);
    position = formatted_args.position;

}

PointLight::PointLight(float distance) {
    pointlight_args formatted_args;

    LightLutDA_Quadratic(&_lda, 1, 0, 0, 1);
    C3D_LightDistAttnEnable(light, true);
    C3D_LightDistAttn(light, &_lda);
    // position = formatted_args.position;
    position = FVec4_New(0, 2, 0, 1);
    C3D_LightPosition(light, &position);

    C3D_LightColor(light, formatted_args.color[0] * 2, formatted_args.color[1] * 2, formatted_args.color[2] * 2);
}

void PointLight::update(C3D_Mtx *view) {
    _tp = Mtx_MultiplyFVec4(view, position);
    C3D_LightPosition(light, &_tp);
};