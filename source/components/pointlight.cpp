#include "pointlight.h"

namespace ql {
	namespace {
		struct pointlight_args {
			C3D_FVec position;
			C3D_FVec color;
			bool distanceattenuation;
		};
	} // namespace
	PointLight::PointLight(void *data) {
		id = lights::allocateLight();
		ASSERT(id >= 0, "Invalid light ID");
	}
	PointLight::PointLight(PointLight &&l) {
		memcpy(this, &l, sizeof(PointLight)); // since PointLight is POD we can just do this
		l.id = -1;							  // remove light ownership
	}
	PointLight &PointLight::operator=(PointLight &&l) {
		memcpy(this, &l, sizeof(PointLight)); // since PointLight is POD we can just do this
		l.id = -1;							  // remove light ownership
		return *this;
	}
	void PointLight::set(C3D_Mtx &modelView) {
		transformedPosition = Mtx_MultiplyFVec4(&modelView, position);
		C3D_LightPosition(&lights::lights[id], &transformedPosition);
	}
	PointLight::~PointLight() {
		lights::freeLight(id);
	}
} // namespace ql