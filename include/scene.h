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
        std::vector<GameObject*> objects;
        // still hardcoded until i add proper light component
        C3D_Light light;

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

        void printSceneTree(GameObject& root, int indentlevel = 1) {
            for (GameObject* obj : root.children) {
                if (!obj) continue;
                for (int i = 0; i < indentlevel; i++)
                    printf("| ");
                printf("%s\n", root.name.c_str());
                printSceneTree(*obj, indentlevel + 1);
            }
        }

        void fixedUpdate() {
            // all the physics stuff will go here

            r_act_on_objects(root, &GameObject::FixedUpdate);
        };

        virtual void drawTop() {
            Camera::mainTop->Render(); 
        };

        virtual void drawBottom() {}

        Scene(std::string name) : name(name) {
            // this is placeholder while I implement light component
            C3D_LightEnvInit(&lights::lightenv);
            C3D_LightEnvBind(&lights::lightenv);

            C3D_FVec lightVec = FVec4_New(0.0f, 0.0f, 0.f, 1.0f);

            C3D_LightInit(&light, &lights::lightenv);
            C3D_LightColor(&light, 0.992, 0.984, 0.827);
            C3D_LightPosition(&light, &lightVec);
        }
    protected:
        
};