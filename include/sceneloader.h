#pragma once

#include <memory>
#include <string>

class Scene; // forward declaration of scene class so it doesn't need to be included

class SceneLoader {
    SceneLoader() = delete;
    public:
    static std::unique_ptr<Scene> load(std::string file);
};