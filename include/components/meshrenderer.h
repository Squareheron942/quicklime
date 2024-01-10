#pragma once

#include "renderer.h"
#include <citro3d.h>

class GameObject;
class material;

class MeshRenderer : public Renderer {
    GameObject* parent;
    material* mat;

    public:

    MeshRenderer(GameObject& parent, const void* args);
    MeshRenderer(GameObject& parent, material* mat);

    virtual void render(C3D_Mtx& view, C3D_Mtx& projection, C3D_Mtx* replacement = NULL);
};