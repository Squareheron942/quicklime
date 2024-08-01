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
	std::string _name;
	GameObject *root;
    std::list<GameObject> objects; // needs to be a list so it doesn't get reallocated (sad)
    entt::registry reg;
    
    friend class SceneLoader;
    friend class Camera;
    friend class AudioSource;
    friend class AudioManager;
    friend void sceneLoadThread(void* params);
    
    void act_on_objects(void(GameObject::*action)()) { for (GameObject& child : objects) (child.*action)(); } 
    public:
    const std::string& name = _name;
    ~Scene();

    void awake();
    void start();
    void update();
    void fixedUpdate();
    void draw();

    Scene(std::string name);
};