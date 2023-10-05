#include <3ds.h>
#include <utility>
#include "controls.h"
#include <citro3d.h>
#include <unordered_map>
#include <string>
#include <algorithm>

namespace controls {
    namespace // makes these inaccessible outside this namespace
    {
        #define M_RAD 0.01745329252f
        u32 kDown = 0, kHeld = 0, kUp = 0, kRepeat = 0;
        angularRate gRate;
        C3D_FVec gPos = {0, 0, 0};
        circlePosition cPos, csPos;
        std::unordered_map<std::string, unsigned int> mappings {
            {"A", KEY_A},
            {"B", KEY_B},
            {"SELECT", KEY_SELECT},
            {"START", KEY_START},
            {"DRIGHT", KEY_DRIGHT},
            {"DLEFT", KEY_DLEFT},
            {"DUP", KEY_DUP},
            {"DDOWN", KEY_DDOWN},
            {"R", KEY_R},
            {"L", KEY_L}, 
            {"X", KEY_X}, // 10
            {"Y", KEY_Y}, // 11
            {"ZL", KEY_ZL}, // 14
            {"ZR", KEY_ZR}, // 15
            {"TOUCH", KEY_TOUCH}, // 20
            {"CSRIGHT", KEY_CSTICK_RIGHT}, // 24
            {"CSLEFT", KEY_CSTICK_LEFT}, // 25
            {"CSUP", KEY_CSTICK_UP}, // 26
            {"CSDOWN", KEY_CSTICK_DOWN}, // 27
            {"CRIGHT", KEY_CPAD_RIGHT}, // 28
            {"CLEFT", KEY_CPAD_LEFT}, // 29
            {"CUP", KEY_CPAD_UP}, // 30
            {"CDOWN", KEY_CPAD_DOWN}, // 31
            {"UP", KEY_DUP | KEY_CPAD_UP},
            {"DOWN", KEY_DOWN | KEY_CPAD_DOWN},
            {"LEFT", KEY_DLEFT | KEY_CPAD_LEFT},
            {"RIGHT",KEY_DRIGHT | KEY_CPAD_RIGHT}
        };
    };


    void update() {
        float deltaTime = 0.0024f;
        
        hidGyroRead(&gRate);
        hidCircleRead(&cPos);
        hidCstickRead(&csPos);
        hidScanInput();

        kDown = hidKeysDown(); 
        kRepeat = hidKeysDownRepeat(); 
        kHeld = hidKeysHeld(); 
        kUp = hidKeysUp();

        gPos.x += gRate.x * deltaTime * M_RAD;
        gPos.y += gRate.y * deltaTime * M_RAD;
        gPos.z += gRate.z * deltaTime * M_RAD;
    }

    bool getDown(std::string inputName) {
        std::transform(inputName.begin(), inputName.end(), inputName.begin(), [](unsigned char c){ return std::toupper(c); });
        return kDown & mappings.at(inputName);
    }

    bool getRepeat(std::string inputName) {
        std::transform(inputName.begin(), inputName.end(), inputName.begin(), [](unsigned char c){ return std::toupper(c); });
        return kRepeat & mappings.at(inputName); 
    }

    bool getHeld(std::string inputName) {
        std::transform(inputName.begin(), inputName.end(), inputName.begin(), [](unsigned char c){ return std::toupper(c); });
        return kHeld & mappings.at(inputName); 
    }

    bool getUp(std::string inputName) {
        std::transform(inputName.begin(), inputName.end(), inputName.begin(), [](unsigned char c){ return std::toupper(c); });
        return kUp & mappings.at(inputName); 
    }

    /**
     * Sets the mapping of a key name to a hid key value
    */
    void setMapping(std::string keyName, unsigned int mapping) { 
        std::transform(keyName.begin(), keyName.end(), keyName.begin(), [](unsigned char c){ return std::toupper(c); });
        mappings.at(keyName) = mapping; 
    }

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
};