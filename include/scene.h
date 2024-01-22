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

class Scene {
    protected:
        void r_act_on_objects(GameObject* root, void(GameObject::*action)()) {
            for (GameObject* child : root->children) {
                (child->*action)();
                r_act_on_objects(child, action);
            }
        }
    public:
        std::string name;
        entt::registry reg;
	    GameObject *root;
        SpotLight splight;
        std::vector<GameObject*> objects;
        // still hardcoded until i add proper light component

        virtual ~Scene() {}

        void awake() {
            r_act_on_objects(root, &GameObject::Awake); // call awake() on every gameobject and enable them (to self disable do it when this is called)
        }

        void start() {
            r_act_on_objects(root, &GameObject::Start); // start all scripts
        }

        virtual void update() {

            r_act_on_objects(root, &GameObject::Update); // call update() on every gameobject (propagates from root)

            // whatever other per frame logic stuff will get called here

            r_act_on_objects(root, &GameObject::LateUpdate); // call lateupdate() on every gameobject (propagates from root). Used to ensure stuff like cameras move only when everything else is done moving

            // printf("%s\n", root.name.c_str());
            // printSceneTree(root, 1);

            Console::update();
        };

        void fixedUpdate() {
            // all the physics stuff will go here

            r_act_on_objects(root, &GameObject::FixedUpdate);
        };

        virtual void drawTop() {
            Camera::mainTop->Render(); 
        };

        virtual void drawBottom() {}

        Scene(std::string name) : name(name), splight(3.141592653579) {}
    protected:
        
};