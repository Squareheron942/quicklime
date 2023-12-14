#pragma once

#include "citro3d.h"
#include <string>
#include "console.h"

class Texture {
    public:
    std::string name;
    C3D_Tex tex;
    C3D_TexCube cube;
    Texture() {
        Console::log("texture created");
    }
};