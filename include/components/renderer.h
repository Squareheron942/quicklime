#pragma once

#include "components/meshrenderer.h"
#include "util/dummyrenderer.h"
#include "util/renderqueue.h"
#include <variant>

namespace ql {
	class TextRenderer;
	class GameObject;

	enum RendererType {
		RENDERER_NONE,
		RENDERER_MESH,
		RENDERER_TEXT,
		RENDERER_UI
	};

	// composition based renderer type
	class Renderer {
		std::variant<DummyRenderer, MeshRenderer> rnd;
		RendererType t;
		u32 layer = 1; // by default on base layer (layer 1)
		GameObject *parent;

	  public:
		Renderer(GameObject &obj, const void *data);
		void render(C3D_Mtx &view, C3D_Mtx& projection, u32 mask);
		RenderQueue queue() const;
		Renderer& operator=(Renderer&&);
	};
} // namespace ql