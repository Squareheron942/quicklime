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
#include "objloader.h"
#include "console.h"
#include "sl_time.h"
#include "componentmanager.h"
#include "lights.h"

#include "script1.h"
#include "movement_script.h"
#include "stats.h"
#include "camera.h"

Scene1::Scene1() : Scene("Scene1"), script1object(objects), moveobject(objects), testobj(objects) {
	// set up lighting first, before any of the material setup happens
	C3D_LightEnvInit(&lights::lightenv);
	C3D_LightEnvBind(&lights::lightenv);

	C3D_FVec lightVec = FVec4_New(0.0f, 0.0f, -0.5f, 1.0f);

	C3D_LightInit(&light, &lights::lightenv);
	C3D_LightColor(&light, 0.992, 0.984, 0.827);
	C3D_LightPosition(&light, &lightVec);
	
	
	
	// add components and scripts (components before scripts so the scripts can actually grab them properly)
	ComponentManager::addComponent("transform", script1object);
	ComponentManager::addComponent("transform", testobj);
	ComponentManager::addComponent("transform", moveobject);
	ComponentManager::addComponent("Camera", moveobject);

	Camera::mainTop = moveobject.getComponent<Camera>();

	ComponentManager::addScript("MovementScript", moveobject);
	ComponentManager::addScript("Script1", script1object);

	if (Camera::mainTop) Camera::mainTop->objects.push_front(&script1object);
	else Console::warn("No top camera");

	if (Camera::mainTop) Camera::mainTop->objects.push_front(&testobj);
	
	mdlLoader::addModel("romfs:/assets/scene2/models/Body__KitsuneBody_Body__KitsuneBody.001.slmdl", script1object); // test object
	mdlLoader::addModel("romfs:/assets/scene2/models/Body__KitsuneBody_Body__KitsuneBody.001.slmdl", testobj); // test object
	
	script1object.name = "script1object";
	moveobject.name = "moveobject";
	testobj.name = "testobj";
	root.addChild(script1object);
	root.addChild(moveobject);
	root.addChild(testobj);
	

	r_act_on_objects(&root, &GameObject::Awake); // call awake() on every gameobject and enable them (to self disable do it when this is called)

	


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
	Camera::mainTop->Render(); 
}



void Scene1::drawBottom() {}

Scene1::~Scene1() {
	
}