#pragma once

#include "vertex.h"
#include "citro3d.h"
#include "defines.h"
#include "componentmanager.h"
#include "material.h"
#include <memory>

class GameObject;

class mesh {
    public:
    // Configure attributes for use with the vertex shader
	C3D_AttrInfo attrInfo;
    C3D_BufInfo buf;
    int numVerts;
    char vertsize;
    std::shared_ptr<void> vertices;
    float radius;
    material* mat;
    GameObject* parent;

    mesh(GameObject& parent, const void* data);

    mesh(GameObject& parent, std::shared_ptr<void> vertices, unsigned int numVerts, unsigned char vertsize, float radius, unsigned char attrnum, unsigned char attrtypes[], unsigned char attrlen[]);
    
    ~mesh();
};