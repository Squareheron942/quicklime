#pragma once
#include <stdio.h>
#include <string>
#include "gameobject.h"
#include <entt.hpp>

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
        entt::registry objects;
	    GameObject root;

        virtual ~Scene() {}

        virtual void update() = 0;
        virtual void fixedUpdate() {};
        virtual void drawTop(float iod) = 0;
        virtual void drawBottom() = 0;

    protected:
        Scene(std::string name) : name(name), root(objects) {
            root.name = "root";
        }
};