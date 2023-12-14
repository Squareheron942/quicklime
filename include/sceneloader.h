#pragma once

class Scene; // forward declaration of scene class so it doesn't need to be included

class SceneLoader {
    SceneLoader() {}
    public:
    Scene* load(const char* file);
};