#include "lights.h"
#include <citro3d.h>

C3D_LightEnv lights::lightenv = {{}};
Light *lights::active[HW_MAX_LIGHTS] = { NULL };
C3D_Light lights::lights[HW_MAX_LIGHTS] = {{}};
unsigned char lights::n = 0;
bool lights::lightenvneedsupdating = true;