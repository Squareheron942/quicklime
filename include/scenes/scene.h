#pragma once

#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>
#include <string>
#include "gameobject.h"
#include <entt.hpp>
#include "camera.h"
#include <list>

class Scene {
	LightLock lock;
	std::string name;
	GameObject *root;
    std::list<GameObject> objects; // needs to be a list so it doesn't get reallocated (sad)
    entt::registry reg;
    
    friend class SceneLoader;
    friend class Camera;
    friend void sceneLoadThread(void* params);
    
    void act_on_objects(void(GameObject::*action)()) { for (GameObject& child : objects) (child.*action)(); } 
    public:
    ~Scene();

    void awake();

    void start() {
       	LightLock_Init(&lock);
        LightLock_Guard l(lock);
        act_on_objects(&GameObject::Start); // start all scripts
    }

    void update() {
        act_on_objects(&GameObject::Update); // call update() on every gameobject (propagates from root)

        // whatever other per frame logic stuff will get called here

        // call lateupdate() on every gameobject (propagates from root).
        // Used to ensure stuff like cameras move only when everything else is done moving
        act_on_objects(&GameObject::LateUpdate);
    };

    void fixedUpdate() {
    	LightLock_Guard l(lock);
        // all the physics stuff will go here
        act_on_objects(&GameObject::FixedUpdate);
    };

    void draw() {
    	LightLock_Lock(&lock);
    	reg.view<Camera>().each([](auto& cam) { cam.Render(); });
        LightLock_Unlock(&lock);
    };

    Scene(std::string name) : name(name) {
       	LightLock_Init(&lock);
    }
};