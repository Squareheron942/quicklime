#pragma once
#include <citro3d.h>

/**
 * @brief General renderer, abstract class that is only meant to be inherited
 * 
 */
class Renderer {
    protected:
    Renderer() {}
    public:
        virtual void render(C3D_Mtx& view, C3D_Mtx& projection, C3D_Mtx* replacement = NULL) = 0;
};