#pragma once

#include <citro3d.h>
#include <memory>

namespace ql {
	class GameObject;
	class shader;

	class DummyRenderer {
	  public:
		inline DummyRenderer(){};
		inline DummyRenderer(GameObject &obj, const void *data){};
		inline void render(C3D_Mtx &view, C3D_Mtx &projection){};
		inline std::shared_ptr<shader> material() const { return nullptr; }
	};
} // namespace ql