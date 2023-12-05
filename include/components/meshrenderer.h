#pragma once

#include "renderer.h"
#include <citro3d.h>

class GameObject;
class material;

class MeshRenderer : public Renderer {
    GameObject* parent;
    material* mat;

    public:

    MeshRenderer(GameObject& parent, void* args);
    MeshRenderer(GameObject& parent, material* mat);

    virtual void render(C3D_Mtx& projection);
};