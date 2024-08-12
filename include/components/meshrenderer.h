#pragma once

#include "util/threads.h"
#include "components/mesh.h"
#include "shaders/shader.h"
#include <memory>

class GameObject;

class MeshRenderer {
	std::shared_ptr<mesh> meshdata;
	std::unique_ptr<shader> mat;
	GameObject* parent;
	public:
	MeshRenderer(GameObject& obj, const void* data);
	void render(C3D_Mtx& view, C3D_Mtx& projection);
	MeshRenderer& operator=(MeshRenderer&& other);
};