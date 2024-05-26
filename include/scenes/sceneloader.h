#pragma once

#include <memory>
#include <string>
#include <3ds.h>

typedef struct {
	const float& progress; // read only reference
	const bool& isDone; // read only reference
	LightEvent& activationEvent;
} AsyncSceneLoadOperation;

class Scene; // forward declaration of scene class so it doesn't need to be included

class SceneLoader {
    SceneLoader() = delete;
    public:
    /**
     * @brief Load scene from file
     *
     * @param file Path of .scene file to read
     * @return std::unique_ptr<Scene> Pointer to the scene instance
     */
    [[nodiscard]] static bool load(std::string file);

    /**
     * @brief Load scene from file. The returned unique_ptr must be kept alive until progress = 1.
     *
     * @param file Path of .scene file to read
     * @return AsyncSceneLoadOperation Information about scene load progress. If scene not found, progress will be -1
     */
    static AsyncSceneLoadOperation loadAsync(std::string file);
};