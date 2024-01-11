#pragma once

#include <citro3d.h>

class GameObject;

class transform {
    public:
        C3D_FVec position;
        C3D_FQuat rotation;
        C3D_FVec scale;
        transform() : position({1, 0, 0, 0}), rotation({1, 0, 0, 0}), scale({1, 1, 1, 1}) {}; // sets position, rotation, and scale to do nothing
        
        // constructor to be used when data created by componentmanager
        transform(GameObject& parent, const void* data) : position({1, 0, 0, 0}), rotation({1, 0, 0, 0}), scale({1, 1, 1, 1}) {
            if (data) {
                position = (*(transform*)data).position;
                rotation = (*(transform*)data).rotation;
                scale = (*(transform*)data).scale;
            }
        };

        transform(C3D_FVec position = {1, 0, 0, 0}, C3D_FQuat rotation = {1, 0, 0, 0}, C3D_FVec scale = {1, 1, 1, 1}) : position(position), rotation(rotation), scale(scale) {};

        /**
         * @brief Adds offset to current position
        */
        inline void translate(C3D_FVec offset) {
            position.x += offset.x;
            position.y += offset.y;
            position.z += offset.z;
        };


        /**
         * @brief Rotates entity by `offset` radians
         * 
         * @param offset Euler angles to offset by
         * @param local Whether or not to use local or global rotation
        */
        inline void rotate(C3D_FVec offset, bool local = true) {
            rotation = Quat_RotateZ(rotation, offset.z, local);
            rotation = Quat_RotateX(rotation, offset.x, local);
            rotation = Quat_RotateY(rotation, offset.y, local);
        }

        inline void rotateX(float angle, bool local = true) { rotation = Quat_RotateX(rotation, angle, local); }
        inline void rotateY(float angle, bool local = true) { rotation = Quat_RotateY(rotation, angle, local); }
        inline void rotateZ(float angle, bool local = true) { rotation = Quat_RotateZ(rotation, angle, local); }

        /**
         * @brief Set the rotation component
         * 
         * @param attitude New direction to set rotation to
         * @param local Whether or not to use local or global rotation
         */
        inline void setRotation(C3D_FVec attitude, bool local = true) {
            rotation = Quat_FromPitchYawRoll(attitude.x, attitude.y, attitude.z, local);
        }

        /**
         * Rotates entity by `offset` radians around origin point
         * 
         * @param offset How much to rotate by
         * @param origin Point to rotate around
        */
        inline void rotate(C3D_FVec offset, C3D_FVec origin) {
            C3D_FVec posoffset = {position.x - origin.x, position.y - origin.y, position.z - origin.z};
            C3D_FQuat rot = Quat_FromPitchYawRoll(offset.x, offset.y, offset.z, false);
            position = Quat_CrossFVec3(rot, posoffset);
            position.x += origin.x;
            position.y += origin.y;
            position.z += origin.z;
            rotation = Quat_Multiply(rot, rotation);
        }

        static inline C3D_FQuat slerp(C3D_FQuat qa, C3D_FQuat qb, double t) {
            // quaternion to return
            C3D_FQuat qm;

            // Calculate angle between them.
            float cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;

            // if qa=qb or qa=-qb then theta = 0 and we can return qa
            if (abs(cosHalfTheta) >= 1.0){
                qm.w = qa.w;qm.x = qa.x;qm.y = qa.y;qm.z = qa.z;
                return qm;
            }
            // Calculate temporary values.
            float halfTheta = acos(cosHalfTheta);
            float sinHalfTheta = sqrt(1.0 - cosHalfTheta*cosHalfTheta);
            // if theta = 180 degrees then result is not fully defined
            // we could rotate around any axis normal to qa or qb
            if (fabs(sinHalfTheta) < 0.001){ // fabs is floating point absolute
                qm.w = (qa.w * 0.5 + qb.w * 0.5);
                qm.x = (qa.x * 0.5 + qb.x * 0.5);
                qm.y = (qa.y * 0.5 + qb.y * 0.5);
                qm.z = (qa.z * 0.5 + qb.z * 0.5);
                return qm;
            }
            float ratioA = sinf((1 - t) * halfTheta) / sinHalfTheta;
            float ratioB = sinf(t * halfTheta) / sinHalfTheta; 

            //calculate Quaternion.
            qm.w = (qa.w * ratioA + qb.w * ratioB);
            qm.x = (qa.x * ratioA + qb.x * ratioB);
            qm.y = (qa.y * ratioA + qb.y * ratioB);
            qm.z = (qa.z * ratioA + qb.z * ratioB);
            return qm;
        }


        /**
         * @brief Converts the internal quaternion to euler angles. NOT RECOMMENDED AS IT IS VERY SLOW (multiple sqrt and atan2 used)
         * 
         * @return C3D_FVec 
         */
        inline C3D_FVec eulerAngles() {
            C3D_FVec angles;
            C3D_FQuat q = rotation;

            // pitch (x-axis rotation)
            float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
            float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
            angles.x = atan2f(sinr_cosp, cosr_cosp);

            // yaw (y-axis rotation)
            float sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
            float cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
            angles.y = 2 * atan2f(sinp, cosp) - M_PI / 2;

            // roll (z-axis rotation)
            float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
            float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
            angles.z = atan2f(siny_cosp, cosy_cosp);

            return angles;
        }

        inline operator C3D_Mtx() {
            C3D_Mtx m, rot;
            
            Mtx_FromQuat(&rot, rotation);
            Mtx_Identity(&m);
            
            Mtx_Translate(&m, position.x, position.y, position.z, true); // translate
            Mtx_Multiply(&m, &rot, &m); // rotate 
            Mtx_Scale(&m, scale.x, scale.y, scale.z); // scale
            
            return m;
        }
};