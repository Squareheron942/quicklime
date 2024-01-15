#pragma once

#include "scene.h"
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <citro3d.h>
#include "lights.h"

//TODO needs to be basically completely rewritten/remade
class SceneManager {
    private:
    public:
        static inline std::unique_ptr<Scene> currentScene;
        template <typename T, typename... Args> 
        requires std::is_base_of_v<Scene, T>
        static void load(Args&&... args) {
            SceneManager::currentScene.reset();
            C3D_LightEnvInit(&lights::lightenv);
            C3D_LightEnvBind(&lights::lightenv);
            SceneManager::currentScene.reset(new T(std::forward<Args>(args)...));
        }
        static void update() {
            currentScene->update();
        }
};