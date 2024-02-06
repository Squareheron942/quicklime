#pragma once

#include <stdio.h>
#include <string>

class material;
class bone;
class GameObject;

namespace mdlLoader
{
    /**
     * @brief Creates a material from a material file
     * 
     * @param f The material file to read from
     * @return material* A pointer to the created material
     */
    [[nodiscard]] material* parseMat(FILE* f);

    /**
     * @brief Creates a skeleton structure from model file
     * 
     * @param f The model file pointer
     * @return bone* The skeleton
     */
    [[nodiscard]] bone* parseBones(FILE* f);

    bool addModel(std::string path, GameObject& object, bool createnew = false);
}