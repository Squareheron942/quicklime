#pragma once

#include <citro3d.h>

class transform {
    public:
        C3D_FVec position, rotation;
        transform() : position({0, 0, 0, 1}), rotation({0, 0, 0, 1}) {
            
        };
        
        /**
         * @brief Adds offset to current position
        */
        constexpr void translate(C3D_FVec &offset) {
            position.x += offset.x;
            position.y += offset.y;
            position.z += offset.z;
        };

        /**
         * @brief Rotates entity by `offset` radians
        */
        void rotate(C3D_FVec &offset) {   
            rotation.x += offset.x;
            rotation.y += offset.y;
            rotation.z += offset.z;
        }

        /**
         * Rotates entity by `offset` radians around origin point
        */
        void rotate(C3D_FVec &offset, C3D_FVec &origin) {

        }
};