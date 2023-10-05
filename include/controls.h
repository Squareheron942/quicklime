#pragma once

#include <3ds.h>
#include <utility>
#include <citro3d.h>
#include <unordered_map>
#include <string>
#include <algorithm>

namespace controls {
    // namespace // makes these inaccessible outside this namespace
    // {
    //     #define M_RAD 0.01745329252f
    //     inline u32 kDown = 0, kHeld = 0, kUp = 0, kRepeat = 0;
    //     inline angularRate gRate;
    //     inline C3D_FVec gPos = {0, 0, 0};
    //     inline circlePosition cPos, csPos;
    //     inline std::unordered_map<std::string, unsigned int> mappings {
    //         {"A", KEY_A},
    //         {"B", KEY_B},
    //         {"SELECT", KEY_SELECT},
    //         {"START", KEY_START},
    //         {"DRIGHT", KEY_DRIGHT},
    //         {"DLEFT", KEY_DLEFT},
    //         {"DUP", KEY_DUP},
    //         {"DDOWN", KEY_DDOWN},
    //         {"R", KEY_R},
    //         {"L", KEY_L}, 
    //         {"X", KEY_X}, // 10
    //         {"Y", KEY_Y}, // 11
    //         {"ZL", KEY_ZL}, // 14
    //         {"ZR", KEY_ZR}, // 15
    //         {"TOUCH", KEY_TOUCH}, // 20
    //         {"CSRIGHT", KEY_CSTICK_RIGHT}, // 24
    //         {"CSLEFT", KEY_CSTICK_LEFT}, // 25
    //         {"CSUP", KEY_CSTICK_UP}, // 26
    //         {"CSDOWN", KEY_CSTICK_DOWN}, // 27
    //         {"CRIGHT", KEY_CPAD_UP}, // 28
    //         {"CLEFT", 0x1d}, // 29
    //         {"CUP", 0x1e}, // 30
    //         {"CDOWN", 0x1f}, // 31
    //         {"UP", KEY_DUP | KEY_CPAD_UP},
    //         {"DOWN", 0x18},
    //         {"LEFT", 0x19},
    //         {"RIGHT", 0x1a},
    //         {"A", 0x1d},
    //         {"A", 0x1c},
    //         {"A", 0x1d},
    //         {"A", 0x1e},
    //         {"A", 0x1f},
    //     };
    // };


    void update();

    bool getDown(std::string inputName);

    bool getRepeat(std::string inputName);

    bool getHeld(std::string inputName);

    bool getUp(std::string inputName);

    /**
     * @returns The internal gyro angular rate (in deg/s)
    */
    const angularRate gyroRate(); // this way no one can modify the internal states
    /**
     * @returns The internal gyro position (in radians)
    */
    const C3D_FVec gyroPos();

    /**
     * @brief Resets internal gyro state to given value
     * @param[in] newPos Position to reset the gyro state to (in radians)
    */
    void resetGyro(const C3D_FVec newPos);

    /**
     * @returns The position of the Circle Pad
    */
    const circlePosition circlePos();

    /**
     * @returns The position of the C Stick
    */
    const circlePosition cStickPos();
};