#include "mesh.h"
#include "gameobject.h"
#include "console.h"
#include <memory>
#include "stats.h"
#include "componentmanager.h"


namespace {
    struct mesh_data {
        int numVerts = 0;
        char vertsize = 0;
        void *vertices = NULL;
        float radius = 0;
        unsigned char attrnum = 0; // number of attributes
        unsigned char attrtypes[16]; // type of attribute
        unsigned char attrlen[16]; // number of elements for each attribute
    };

    u64 permut_from_num_attr(u8 n) {
        u64 perm = 0;
        for (u8 i = 0; i < n; i++) {
            perm |= ((i & 0xF) << (i << 2));
        } // for each attrib add the corresponding number (for 3 attribs, should make 0x210 aka 528)
        return perm;
    }
} 

mesh::mesh(void* vertices, unsigned int numVerts, unsigned char vertsize, float radius, unsigned char attrnum, unsigned char attrtypes[], unsigned char attrlen[]) : numVerts(numVerts), vertsize(vertsize), _vertices(vertices), radius(radius) {
    // make sure the vertex data is in ram and not CPU cache
    GSPGPU_FlushDataCache(vertices, vertsize * numVerts);

    // Configure attributes for use with the vertex shader
    AttrInfo_Init(&attrInfo);
    for (int i = 0; i < attrnum; i++)
        AttrInfo_AddLoader(&attrInfo, i, (GPU_FORMATS)attrtypes[i], attrlen[i]);

    BufInfo_Init(&buf);
    BufInfo_Add(&buf, vertices, vertsize, attrnum, permut_from_num_attr(attrnum));
    stats::_vertices += numVerts;
}

mesh::~mesh() {
	linearFree(_vertices);
    stats::_vertices -= numVerts;
}