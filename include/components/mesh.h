#pragma once

#include <citro3d.h>
#include <memory>
#include <vector>

class GameObject;

struct LOD_info {
	float distance2; // LOD distance^2 
	int beginindex, size;
};

class mesh {
	friend class MeshRenderer;
    // Configure attributes for use with the vertex shader
	C3D_AttrInfo attrInfo;
    C3D_BufInfo buf;
    unsigned int numVerts;
    unsigned int vertsize;
    void* _vertices;
    float radius;
    std::vector<LOD_info> LOD_levels;
    public:
    const void* vertices() { return _vertices; }
    mesh(void* vertices, unsigned int numVerts, unsigned char vertsize, float radius, unsigned char attrnum, unsigned char attrtypes[], unsigned char attrlen[]);
    ~mesh();
};