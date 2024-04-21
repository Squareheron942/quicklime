#include "meshrenderer.h"
#include "mesh.h"
#include "material.h"
#include "gameobject.h"
#include <citro3d.h>
#include "transform.h"
#include "componentmanager.h"
#include "assert.h"
#include "console.h"
#include "stats.h"
#include "defines.h"
#include "renderertypes.h"

MeshRenderer::MeshRenderer(GameObject& parent, const void* args) : parent(&parent), mat((material*)args) {
    parent.renderer |= RENDERER_MESH;
}

MeshRenderer::MeshRenderer(GameObject& parent, material* mat) : parent(&parent), mat(mat) {
    parent.renderer |= RENDERER_MESH;
}

MeshRenderer::~MeshRenderer() {
	Console::log("Mesh renderer destructor");
}

void MeshRenderer::render(C3D_Mtx& view, C3D_Mtx& projection, C3D_Mtx* replacement_mv) {
    mesh* m = parent->getComponent<mesh>();
    #if DEBUG
    assert(m != nullptr);
    assert(mat != nullptr);
    assert(parent != nullptr);
    stats::_drawcalls++; // increment internal counter for debugging info
    #endif

    C3D_SetBufInfo(&m->buf);
    C3D_SetAttrInfo(&m->attrInfo);

    C3D_Mtx model = *parent->getComponent<transform>(); // always will have a transform by default

    Mtx_Multiply(&model, &view, replacement_mv ? replacement_mv : &model);

    osTickCounterStart(&stats::profiling::cnt_setmtl);

    mat->setMaterial(&model, &projection);

    osTickCounterUpdate(&stats::profiling::cnt_setmtl);
    stats::profiling::rnd_setmtl += osTickCounterRead(&stats::profiling::cnt_setmtl);

    osTickCounterStart(&stats::profiling::cnt_drawarr);
    // Draw the VBO
    C3D_DrawArrays(GPU_TRIANGLES, 0, m->numVerts);

    osTickCounterUpdate(&stats::profiling::cnt_drawarr);
    stats::profiling::rnd_drawarr += osTickCounterRead(&stats::profiling::cnt_drawarr);

    mat->resetMaterial();
}

COMPONENT_REGISTER(MeshRenderer)