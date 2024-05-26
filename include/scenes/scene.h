#pragma once

#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>
#include <string>
#include "gameobject.h"
#include <entt.hpp>
#include "camera.h"
#include <vector>
#include "lights.h"
#include "pointlight.h"
#include "spotlight.h"
#include "stats.h"
#include <list>

class Scene {
	LightLock lock;
    protected:
        void r_act_on_objects(GameObject* root, void(GameObject::*action)()) {
            for (GameObject* child : root->children) {
                (child->*action)();
                r_act_on_objects(child, action);
            }
        }
    public:
        std::string name;
	    GameObject *root;
        std::vector<GameObject*> objects;
        std::vector<Camera*> cameras;
        entt::registry reg;

        ~Scene() {
        	Console::log("Scene destructor");
        	LightLock_Lock(&lock);
        	for (auto* object : objects) if (object) delete object; // remove all objects
         	LightLock_Unlock(&lock);
        }

        void awake() {
        	LightLock_Lock(&lock);
            r_act_on_objects(root, &GameObject::Awake); // call awake() on every gameobject and enable them (to self disable do it when this is called)
            LightLock_Unlock(&lock);
        }

        void start() {
        	LightLock_Lock(&lock);
            r_act_on_objects(root, &GameObject::Start); // start all scripts
            LightLock_Unlock(&lock);
        }

        virtual void update() {
        	LightLock_Lock(&lock);

            r_act_on_objects(root, &GameObject::Update); // call update() on every gameobject (propagates from root)

            // whatever other per frame logic stuff will get called here

            // call lateupdate() on every gameobject (propagates from root).
            // Used to ensure stuff like cameras move only when everything else is done moving
            r_act_on_objects(root, &GameObject::LateUpdate);
            LightLock_Unlock(&lock);
        };

        void fixedUpdate() {
        	LightLock_Lock(&lock);
            // all the physics stuff will go here
            r_act_on_objects(root, &GameObject::FixedUpdate);
            LightLock_Unlock(&lock);
        };

        virtual void drawTop() {
        	LightLock_Lock(&lock);
         	if (Camera::mainTop)
            Camera::mainTop->Render();
            LightLock_Unlock(&lock);
        };

        virtual void drawBottom() {}

        Scene(std::string name) : name(name) {
        	LightLock_Init(&lock);
        }
    protected:

};