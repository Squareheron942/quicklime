#include "meshrenderer.h"
#include "componentmanager.h"
#include "defines.h"
#include "mesh.h"
#include "renderer.h"
#include "sl_assert.h"
#include "slmdlloader.h"
#include "stats.h"
#include "transform.h"
#include <string>

namespace {
	struct meshrenderer_args {
		RendererType type; // useless but is used for parent class so still
						   // necessary to include
						   // mesh name
						   // material file
	};
} // namespace

MeshRenderer::MeshRenderer(GameObject &obj, const void *data)
	: parent(&obj) { // parent will never be null
	ASSERT(data != nullptr, "Mesh parameter was null");
	std::string meshpath = (char *)data;
	ASSERT(meshpath.size() > 0, "Model path is empty");
	std::string matpath = (char *)data + meshpath.size() + 1;
	ASSERT(matpath.size() > 0, "Material path is empty");
	mat = mdlLoader::parseMat(matpath);
	std::optional<std::shared_ptr<mesh>> mesh_opt =
		mdlLoader::parseModel(meshpath);
	ASSERT(mesh_opt.has_value(), "Invalid mesh");
	meshdata = mesh_opt.value();
	assert(mat != nullptr);
}

void MeshRenderer::render(C3D_Mtx &view, C3D_Mtx &projection) {
#if DEBUG
	stats::_drawcalls++;
#endif
	C3D_SetBufInfo(&meshdata->buf);
	C3D_SetAttrInfo(&meshdata->attrInfo);

	// always will have a transform by default
	// safe since pointer isn't stored
	C3D_Mtx model = *parent->getComponent<transform>();

	Mtx_Multiply(&model, &view, &model);

	mat->setMaterial(&model, &projection);

	// LOD system
	float distance2 = model.r[0].w * model.r[0].w +
					  model.r[1].w * model.r[1].w + model.r[2].w * model.r[2].w;

	for (LOD_info &inf : meshdata->LOD_levels) {
		if (inf.distance2 <= distance2) {
			C3D_DrawArrays(GPU_TRIANGLES, inf.beginindex, inf.size);
			break;
		}
	}

	mat->resetMaterial();
}

MeshRenderer &MeshRenderer::operator=(MeshRenderer &&other) {
	meshdata = std::move(other.meshdata);
	mat		 = std::move(other.mat);
	parent	 = other.parent;
	return *this;
}

COMPONENT_REGISTER(MeshRenderer)