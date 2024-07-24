#include "scene.h"
#include "camera.h"

Scene::~Scene() {
	Console::log("Scene destructor");
    LightLock_Lock(&lock);
    objects.clear();
    LightLock_Unlock(&lock);
}

void Scene::awake() {
    LightLock_Lock(&lock);
    act_on_objects(&GameObject::Awake); // call awake() on every gameobject and enable them (to self disable do it when this is called)
    LightLock_Unlock(&lock);
}