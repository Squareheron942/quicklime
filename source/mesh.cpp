#include "mesh.h"
#include "gameobject.h"
#include "console.h"

namespace {
    struct mesh_data {
        int numVerts = 0;
        char vertsize = 0;
        void *vertices = NULL;
        float radius = 0;
    };
}

mesh::mesh(GameObject& parent, void* data) {
    mesh_data d;

    if (data) d = *(mesh_data*)data;

    numVerts = d.numVerts;
    vertsize = d.vertsize;
    vertices = d.vertices;
    radius = d.radius;
    GSPGPU_FlushDataCache(vertices, vertsize * numVerts); // make sure the data is in ram and not CPU cache
    
    // Configure attributes for use with the vertex shader
    attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2=normal
    buf = C3D_GetBufInfo();
    BufInfo_Init(buf);
    BufInfo_Add(buf, d.vertices, d.vertsize, 3, 0x210);
}

mesh::mesh(void* vertices, unsigned int numVerts, unsigned char vertsize, float radius) : numVerts(numVerts), vertsize(vertsize), vertices(vertices), radius(radius) {
    // Configure attributes for use with the vertex shader
    attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2=normal
    buf = C3D_GetBufInfo();
    BufInfo_Init(buf);
    BufInfo_Add(buf, vertices, vertsize, 3, 0x210);
}

mesh::~mesh() {
    linearFree(vertices); 
}

COMPONENT_REGISTER(mesh)