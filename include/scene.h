#pragma once
#include <stdio.h>
#include <string>
#include "gameobject.h"
#include <entt.hpp>

class Scene {
    public:
        std::string name;
        entt::registry objects;
	    GameObject root;

        virtual ~Scene() {
            // printf("Scene \"%s\" destroyed.\n", name.c_str());
        }

        virtual void update() = 0;
        virtual void fixedUpdate() {};
        virtual void drawTop(float iod) = 0;
        virtual void drawBottom() = 0;

    protected:
        Scene(std::string name) : name(name), root(objects) {
            root.name = "root";
            // printf("Scene \"%s\" created.\n", name.c_str());
        }
};