#pragma once

#include "vertex.h"
#include "citro3d.h"

class Mesh {
    public:
    // Configure attributes for use with the vertex shader
	C3D_AttrInfo* attrInfo;
    C3D_BufInfo* buf;
    Mesh(void* vertices, int numVerts, char vertsize) : numVerts(numVerts), vertsize(vertsize), vertices(vertices) {
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
    int numVerts;
    char vertsize;
    void *vertices;
    
    ~Mesh() {
        linearFree(vertices); 
    }
};  