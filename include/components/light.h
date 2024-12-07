#pragma once

#include "components/pointlight.h"
#include "shaders/lights.h"
#include "util/dummylight.h"
#include "util/ql_assert.h"
#include <variant>
#include <citro3d.h>

namespace ql {
	class GameObject;

	enum LightType {
		LightType_NONE,
		LightType_POINT
	};

	class Light {
		GameObject *p;
		std::variant<DummyLight, PointLight> internal_light;
		LightType t;

	  public:
		Light(GameObject &owner, const void *data);
		Light &operator=(Light &&);
		void setSelf(C3D_Mtx &modelView);
	};
} // namespace ql