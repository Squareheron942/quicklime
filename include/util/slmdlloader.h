#pragma once

#include <string>
#include <memory>
#include "threads.h"
#include <stdio.h>

class shader;
class bone;
class mesh;
class GameObject;

namespace mdlLoader
{
    /**
     * @brief Creates a shader instance from a material file
     * 
     * @param f The material file to read from
     * @return material* A pointer to the created material
     */
    [[nodiscard]] std::unique_ptr<shader> parseMat(const std::string file);

    /**
     * @brief Creates a skeleton structure from model file
     * 
     * @param f The model file pointer
     * @return bone* The skeleton
     */
    [[nodiscard]] bone* parseBones(FILE* f);

    [[nodiscard]] std::shared_ptr<LightLock_Mutex<mesh>> parseModel(const std::string path, bool createnew = false);
}
