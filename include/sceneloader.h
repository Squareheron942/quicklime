#pragma once

#include <memory>
#include <string>

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
    static std::unique_ptr<Scene> load(std::string file);

    /**
     * @brief Load scene from file
     *
     * @param file Path of .scene file to read
     * @param progress Stores progress percentage in the range [0, 1]
     * @return std::unique_ptr<Scene> Pointer to the scene instance.
     */
    static std::unique_ptr<Scene> loadAsync(std::string file, float* progress);
};