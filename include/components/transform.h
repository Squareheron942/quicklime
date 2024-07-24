#pragma once

#include <citro3d.h>

class GameObject;

class transform {
		C3D_FVec _position;
        C3D_FQuat _rotation;
        C3D_FVec _scale;
        LightLock _l;
    public:
    	const C3D_FVec& position = _position;
    	const C3D_FQuat& rotation = _rotation;
     	const C3D_FVec& scale = _scale;
      
        ~transform();

        transform(); // sets position, rotation, and scale to do nothing

        // constructor to be used when data created by componentmanager
        transform(GameObject& parent, const void* data);

        transform(C3D_FVec position = {{1, 0, 0, 0}}, C3D_FQuat rotation = {{1, 0, 0, 0}}, C3D_FVec scale = {{1, 1, 1, 1}});;
        transform& operator=(transform&& other);
        /**
         * @brief Adds offset to current position
        */
        void translate(C3D_FVec offset);


        /**
         * @brief Rotates entity by `offset` radians
         *
         * @param offset Euler angles to offset by
         * @param local Whether or not to use local or global rotation
        */
        void rotate(C3D_FVec offset, bool local = true);

        void rotateX(float angle, bool local = true);
        void rotateY(float angle, bool local = true);
        void rotateZ(float angle, bool local = true);

        /**
         * @brief Set the rotation component
         *
         * @param attitude New direction to set rotation to
         * @param local Whether or not to use local or global rotation
         */
        void setRotation(C3D_FVec attitude, bool local = true);

        /**
         * Rotates entity by `offset` radians around origin point
         *
         * @param offset How much to rotate by
         * @param origin Point to rotate around
        */
        void rotate(C3D_FVec offset, C3D_FVec origin);

        static C3D_FQuat slerp(C3D_FQuat qa, C3D_FQuat qb, double t);


        /**
         * @brief Converts the internal quaternion to euler angles. NOT RECOMMENDED AS IT IS VERY SLOW (multiple sqrt and atan2 used)
         *
         * @return C3D_FVec
         */
        C3D_FVec eulerAngles() const;

        operator C3D_Mtx() const;
};