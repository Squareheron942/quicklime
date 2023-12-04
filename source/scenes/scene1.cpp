#include <3ds.h>
#include <citro3d.h>
#include <string.h>
#include "fragvshader_shbin.h"
#include "scenemanager.h"
#include "scene1.h"
#include "fast_obj.h"
#include "transform.h"
#include <entt.hpp>
#include "controls.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "mesh.h"
#include "material.h"
#include "slmdlloader.h"
#include "console.h"
#include "sl_time.h"
#include "componentmanager.h"

#include "script1.h"
#include "movement_script.h"
#include "stats.h"
#include "camera.h"

Scene1::Scene1() : Scene("Scene1"), script1object(objects), moveobject(objects) {

	// add components and scripts (components before scripts so the scripts can actually grab them properly)
	transform defaultpos({1, 0, 0, 0});
	transform campos({1, -1, 0, 0});
	ComponentManager::addComponent("transform", script1object, &defaultpos);
	ComponentManager::addComponent("mesh", script1object);
	ComponentManager::addComponent("transform", moveobject, &campos);
	ComponentManager::addComponent("Camera", moveobject);

	Camera::mainTop = moveobject.getComponent<Camera>();

	ComponentManager::addScript("MovementScript", moveobject);
	ComponentManager::addScript("Script1", script1object);

	if (Camera::mainTop) Camera::mainTop->objects.push_front(&script1object);
	else Console::warn("No top camera");
	
	mdlLoader::addModel("romfs:/data/scene1/models/cube.slmdl", script1object);
	
	script1object.name = "script1object";
	moveobject.name = "moveobject";
	root.addChild(script1object);
	root.addChild(moveobject);

	r_act_on_objects(&root, &GameObject::Awake); // call awake() on every gameobject and enable them (to self disable do it when this is called)

	C3D_LightEnvInit(&lightEnv);
	C3D_LightEnvBind(&lightEnv);
	C3D_LightEnvMaterial(&lightEnv, &lmat);

	LightLut_Phong(&lut_Light, 300);
	C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lut_Light);

	C3D_FVec lightVec = FVec4_New(0.0f, 0.0f, -0.5f, 1.0f);

	C3D_LightInit(&light, &lightEnv);
	C3D_LightColor(&light, 0.992, 0.984, 0.827);
	C3D_LightPosition(&light, &lightVec);


	r_act_on_objects(&root, &GameObject::Start); // start all scripts
}

void Scene1::update() {
	r_act_on_objects(&root, &GameObject::Update); // call update() on every gameobject (propagates from root)

	// whatever other per frame logic stuff will get called here

	r_act_on_objects(&root, &GameObject::LateUpdate); // call lateupdate() on every gameobject (propagates from root). Used to ensure stuff like cameras move only when everything else is done moving

	Console::update();
};

void Scene1::drawTop(float iod)
{
	// for (unsigned int i = 0; i < numgroups; i++) {
	C3D_LightEnvBind(&lightEnv);

	Camera::mainTop->Render(); 
}



void Scene1::drawBottom() {}

Scene1::~Scene1() {
	// Free the texture
	// C3D_TexDelete(&bottom_tex);
	// C3D_TexDelete(&top_tex);

	// // Free the VBO
	// for (unsigned int i = 0; i < numgroups; i++) linearFree(meshes[i]);

	// delete[] meshes;

	// // Free the shader program
	// shaderProgramFree(&program);
	// DVLB_Free(vshader_dvlb);
}