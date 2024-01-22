#include "mesh.h"
#include "gameobject.h"
#include "console.h"
#include <memory>

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
            perm |= ((i & 0xF) << (4 * i));
        } // for each attrib add the corresponding number (for 3 attribs, should make 0x210 aka 528)
        return perm;
    }
}

mesh::mesh(GameObject& parent, const void* data) {
    // mesh_data d;

    // if (data) d = *(mesh_data*)data;

    // numVerts = d.numVerts;
    // vertsize = d.vertsize;
    // vertices.reset(d.vertices);
    // radius = d.radius;
    // GSPGPU_FlushDataCache(vertices.get(), vertsize * numVerts); // make sure the data is in ram and not CPU cache
    
    // // Configure attributes for use with the vertex shader
    // AttrInfo_Init(&attrInfo);
    // for (int i = 0; i < d.attrnum; i++) 
    //     AttrInfo_AddLoader(&attrInfo, i, (GPU_FORMATS)d.attrtypes[i], d.attrlen[i]);

    // BufInfo_Init(&buf);
    // BufInfo_Add(&buf, vertices.get(), vertsize, d.attrnum, permut_from_num_attr(d.attrnum)); // put the attributes in the default registers (this is actually broken and weird and does not work properly)
}

mesh::mesh(GameObject& parent, std::shared_ptr<void> vertices, unsigned int numVerts, unsigned char vertsize, float radius, unsigned char attrnum, unsigned char attrtypes[], unsigned char attrlen[]) : numVerts(numVerts), vertsize(vertsize), vertices(vertices), radius(radius), parent(&parent) {
    // make sure the vertex data is in ram and not CPU cache
    GSPGPU_FlushDataCache(vertices.get(), vertsize * numVerts); 
    
    // Configure attributes for use with the vertex shader
    AttrInfo_Init(&attrInfo);
    for (int i = 0; i < attrnum; i++)
        AttrInfo_AddLoader(&attrInfo, i, (GPU_FORMATS)attrtypes[i], attrlen[i]);

    BufInfo_Init(&buf);
    BufInfo_Add(&buf, vertices.get(), vertsize, attrnum, permut_from_num_attr(attrnum));
}

mesh::~mesh() {
    delete mat;
    Console::log("mesh on %s deleted", parent->name.c_str());
    // std::shared_ptr removed automatically, which also removes the data
}

COMPONENT_REGISTER(mesh)