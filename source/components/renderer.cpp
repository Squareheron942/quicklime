#include "renderer.h"
#include "meshrenderer.h"

namespace {
	struct renderer_args {
		RendererType t;
	};
}

Renderer::Renderer(GameObject& obj, const void* data): parent(&obj) {
	if (!data) return;
	const renderer_args args = *static_cast<const renderer_args*>(data);
	switch(args.t) {
		case RENDERER_MESH: 
			rnd.emplace<MeshRenderer>(obj, data);
			break;
		case RENDERER_TEXT: // not yet implemented
			break;
		case RENDERER_UI: 
			break;
		default: 
			break;
	}
}

void Renderer::render(C3D_Mtx &view, C3D_Mtx &projection) {
	std::visit([&](auto& renderer) { renderer.render(view, projection); }, rnd); // call render on whatever renderer is here
}