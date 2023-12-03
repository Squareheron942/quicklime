#include "meshrenderer.h"
#include "mesh.h"
#include "material.h"
#include "gameobject.h"
#include <citro3d.h>
#include "transform.h"
#include "componentmanager.h"
#include "assert.h"
#include "console.h"

MeshRenderer::MeshRenderer(GameObject& parent, void* args) : parent(&parent), mat((material*)args) {

}

MeshRenderer::MeshRenderer(GameObject& parent, material* mat) : parent(&parent), mat(mat) {

}

bool flag = true;

void MeshRenderer::render(C3D_Mtx& view, C3D_Mtx& projection) {
    mesh* m = parent->getComponent<mesh>();
    assert(m != nullptr);

    C3D_SetBufInfo(m->buf);
    
    C3D_Mtx model = *parent->getComponent<transform>(); // always will have a transform by default

    Mtx_Multiply(&model, &model, &view);

    mat->setMaterial(&view, &projection);

    if (flag) Console::log("Vertices %u", m->numVerts);

    flag = false;

    // Draw the VBO
    C3D_DrawArrays(GPU_TRIANGLES, 0, m->numVerts);
}

COMPONENT_REGISTER(MeshRenderer)