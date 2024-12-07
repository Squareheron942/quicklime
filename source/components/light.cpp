#include "light.h"
#include "componentmanager.h"
#include <utility>

namespace ql {
	Light::Light(GameObject &owner, const void *data) {
		ASSERT(lights::lights_in_use < HW_NUM_LIGHTS, "Too many lights in use");
		if (lights::lights_in_use >= HW_NUM_LIGHTS)
			return;
	}
	Light &Light::operator=(ql::Light &&l) {
		internal_light = std::move(l.internal_light);
		l.internal_light.emplace<DummyLight>();
		t	= l.t;
		p	= l.p;
		l.p = nullptr;
		return *this;
	}

	void Light::setSelf(C3D_Mtx &view) {
		std::visit([&](auto &light) { light.set(view); }, internal_light);
	}
	COMPONENT_REGISTER(Light);
} // namespace ql