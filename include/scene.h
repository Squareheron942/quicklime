#pragma once
#include <stdio.h>
#include <string>

class Scene {
    public:
        std::string name;
        virtual ~Scene() {
            // printf("Scene \"%s\" destroyed.\n", name.c_str());
        }

        virtual void update() = 0;
        virtual void fixedUpdate() {};
        virtual void drawTop(float iod) = 0;
        virtual void drawBottom() = 0;

    protected:
        Scene(std::string name) : name(name) {
            // printf("Scene \"%s\" created.\n", name.c_str());
        }
};