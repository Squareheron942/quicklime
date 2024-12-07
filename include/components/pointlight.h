#pragma once

#include "ql_assert.h"
#include "shaders/lights.h"
#include <citro3d.h>
#include <cstring>

namespace ql {
	class PointLight {
		friend class Light;
		C3D_FVec position, transformedPosition;
		bool distanceattenuation;
		int id;
		C3D_FVec color;

	  public:
		PointLight(void *data);
		PointLight(PointLight &&l);
		PointLight &operator=(PointLight &&l);
		void set(C3D_Mtx &modelView);
		~PointLight();
	};
} // namespace ql