#include "meshrenderer.h"
#include "componentmanager.h"
#include "mesh.h"
#include "slmdlloader.h"
#include "sl_assert.h"
#include <string>
#include "transform.h"
#include "defines.h"
#include "stats.h"
#include "renderer.h"


namespace {
	struct meshrenderer_args {
		RendererType type; // useless but is used for parent class so still necessary to include
		// mesh name
		// material file
	};
}

MeshRenderer::MeshRenderer(GameObject& obj, const void* data): parent(&obj) { // parent will never be null
	ASSERT(data != nullptr, "Mesh parameter was null");
	std::string meshpath = (char*)data;
	ASSERT(meshpath.size() > 0, "Model path is empty");
	std::string matpath = (char*)data + meshpath.size() + 1;
	ASSERT(matpath.size() > 0, "Material path is empty");
	mat = mdlLoader::parseMat(matpath);
	meshdata = mdlLoader::parseModel(meshpath);
	assert(meshdata != nullptr);
    assert(mat != nullptr);
}

void MeshRenderer::render(C3D_Mtx &view, C3D_Mtx &projection) {
	#if DEBUG
    stats::_drawcalls++;
    #endif
    C3D_SetBufInfo(&(*meshdata.get())->buf);
    C3D_SetAttrInfo(&(*meshdata.get())->attrInfo);
    
    C3D_Mtx model = *parent->getComponent<transform>(); // always will have a transform by default
    
    Mtx_Multiply(&model, &view, &model);
    
    mat->setMaterial(&model, &projection);
    
    // LOD system
    float distance2 = model.r[0].w * model.r[0].w + model.r[1].w * model.r[1].w + model.r[2].w * model.r[2].w;
    
    for (LOD_info& inf : (*meshdata.get())->LOD_levels) {
    	if (inf.distance2 <= distance2) {
            C3D_DrawArrays(GPU_TRIANGLES, inf.beginindex, inf.size);
            break;
     	}
    }
    
    mat->resetMaterial();
}

MeshRenderer& MeshRenderer::operator=(MeshRenderer&& other) {
	meshdata = std::move(other.meshdata);
	mat = std::move(other.mat);
	parent = other.parent;
	return *this;
}

COMPONENT_REGISTER(MeshRenderer)