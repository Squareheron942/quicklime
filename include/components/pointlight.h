#pragma once

#include "componentmanager.h"
#include "light.h"
#include <citro3d.h>

class PointLight : public Light {
	C3D_LightLutDA _lda;

  public:
	PointLight(GameObject &obj, const void *params);
	void update(C3D_Mtx *view) override;
};

COMPONENT_REGISTER(PointLight)