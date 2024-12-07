#pragma once

#include "components/mesh.h"
#include "shaders/shader.h"
#include "util/threads.h"
#include <memory>

namespace ql {
	class GameObject;

	class MeshRenderer {
		std::shared_ptr<mesh> meshdata;
		std::shared_ptr<shader> mat;
		GameObject *parent;

	  public:
		MeshRenderer(GameObject &obj, const void *data);
		MeshRenderer(MeshRenderer &&other);
		void render(C3D_Mtx &view, C3D_Mtx &projection);
		MeshRenderer &operator=(MeshRenderer &&other);
		std::shared_ptr<shader> material() const;
	};
} // namespace ql
