#include <3ds.h>
#include <utility>
#include "controls.h"
#include <citro3d.h>
#include <unordered_map>
#include <string>
#include <algorithm>
#include "sl_time.h"

namespace controls {

    namespace // makes these inaccessible outside this namespace
    {
        #define M_RAD 0.01745329252f
        u32 kDown = 0, kHeld = 0, kUp = 0, kRepeat = 0;
        angularRate gRate;
        C3D_FVec gPos = {0, 0, 0};
        circlePosition cPos, csPos;
        float gyro_s = 0.5f, gyro_d = 10.f;
        // TODO Replace with enum and array

        unsigned int mappings[27] = {
            KEY_A,
            KEY_B,
            KEY_X,
            KEY_Y,
            KEY_SELECT,
            KEY_START,
            KEY_DRIGHT,
            KEY_DLEFT,
            KEY_DUP,
            KEY_DDOWN,
            KEY_L,
            KEY_R,
            KEY_ZL,
            KEY_ZR,
            KEY_CSTICK_UP,
            KEY_CSTICK_DOWN,
            KEY_CSTICK_LEFT,
            KEY_CSTICK_RIGHT,
            KEY_CPAD_UP,
            KEY_CPAD_DOWN,
            KEY_CPAD_LEFT,
            KEY_CPAD_RIGHT,
            KEY_DUP | KEY_CPAD_UP,
            KEY_DOWN | KEY_CPAD_DOWN,
            KEY_DLEFT | KEY_CPAD_LEFT,
            KEY_DRIGHT | KEY_CPAD_RIGHT,
            KEY_TOUCH
        };
    };

    void update() {
        hidGyroRead(&gRate);
        hidCircleRead(&cPos);
        hidCstickRead(&csPos);
        hidScanInput();

        kDown = hidKeysDown(); 
        kRepeat = hidKeysDownRepeat(); 
        kHeld = hidKeysHeld(); 
        kUp = hidKeysUp();

        gRate.x *= gyro_s;
        gRate.y *= gyro_s;
        gRate.z *= gyro_s;

        gPos.x += gRate.x * Time::deltaTime * M_RAD;
        gPos.y += gRate.y * Time::deltaTime * M_RAD;
        gPos.z += gRate.z * Time::deltaTime * M_RAD;
    }

    bool getDown(key inputName) { return kDown & mappings[(unsigned char)inputName]; }

    bool getRepeat(key inputName) { return kRepeat & mappings[(unsigned char)inputName]; }

    bool getHeld(key inputName) { return kHeld & mappings[(unsigned char)inputName]; }

    bool getUp(key inputName) { return kUp & mappings[(unsigned char)inputName]; }

    /**
     * Sets the mapping of a key name to a hid key value
    */
    void setMapping(key keyName, unsigned int mapping) { mappings[(unsigned char)keyName] = mapping; }

    /**
     * @returns The internal gyro angular rate (in deg/s)
    */
    const angularRate gyroRate() { return gRate; } // this way no one can modify the internal states

    /**
     * @returns The internal gyro position (in radians)
    */
    const C3D_FVec gyroPos() { return gPos; };

    /**
     * @brief Resets internal gyro state to given value
     * @param[in] newPos Position to reset the gyro state to (in radians)
    */
    void resetGyro(const C3D_FVec newPos) {
        gPos = newPos;
    };

    /**
     * @returns The position of the Circle Pad
    */
    const circlePosition circlePos() { /*hidCircleRead(&cPos);*/return cPos; }

    /**
     * @returns The position of the C Stick
    */
    const circlePosition cStickPos() { /*hidCstickRead(&cPos);*/return csPos; }

    /**
     * @brief Returns sensitivity multiplier used by gyro
    */
    float gyroSensitivity() { return gyro_s; }

    /**
     * @brief Sets sensitivity multiplier used by gyro
    */
    void setGyroSensitivity(float sensitivity) { gyro_s = sensitivity; }

    /**
     * @brief Returns gyro dead zone radius
    */
    float gyroDeadZone() { return gyro_d; }

    /**
     * @brief Sets gyro dead zone radius
    */
    void setGyroDeadZone(float min_rate) { gyro_d = min_rate; }
};