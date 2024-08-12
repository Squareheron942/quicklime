#pragma once

#include <variant>
#include "util/dummyrenderer.h"
#include "components/meshrenderer.h"

class TextRenderer;
class GameObject;

enum RendererType {
	RENDERER_NONE,
    RENDERER_MESH,
    RENDERER_TEXT,
    RENDERER_UI
};

// composition based renderer type
class Renderer {
    std::variant<DummyRenderer, MeshRenderer> rnd;
    RendererType t;
    GameObject* parent;
    public:
    Renderer(GameObject& obj, const void* data);
    void render(C3D_Mtx& view, C3D_Mtx& projection);
};