#pragma once

#include <citro3d.h>

namespace ql {
	class GameObject;

	class DummyRenderer {
	  public:
		inline DummyRenderer(){};
		inline DummyRenderer(GameObject &obj, const void *data){};
		inline void render(C3D_Mtx &view, C3D_Mtx &projection){};
	};
} // namespace ql