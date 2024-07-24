#include <3ds.h>
#include <utility>
#include "controls.h"
#include <citro3d.h>
#include <unordered_map>
#include <string>
#include <algorithm>
#include "sl_time.h"
#include "threads.h"

#define M_RAD 0.01745329252f

namespace controls {

    namespace // makes these inaccessible outside this namespace
    {
    	LightLock _l = {1};
        unsigned int kDown = 0, kHeld = 0, kUp = 0, kRepeat = 0;
        angularRate gRate;
        C3D_FVec gPos = {0, 0, 0};
        circlePosition cPos, csPos;
        float gyro_s = 0.5f, gyro_d = 10.f;
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
    
    void init() {
    	LightLock_Init(&_l);
        HIDUSER_EnableGyroscope();
    }

    bool getDown(key inputName) { LightLock_Guard l(_l); return kDown & mappings[(unsigned char)inputName]; }
    bool getRepeat(key inputName) { LightLock_Guard l(_l); return kRepeat & mappings[(unsigned char)inputName]; }
    bool getHeld(key inputName) { LightLock_Guard l(_l); return kHeld & mappings[(unsigned char)inputName]; }
    bool getUp(key inputName) { LightLock_Guard l(_l); return kUp & mappings[(unsigned char)inputName]; }
    void setMapping(key keyName, unsigned int mapping) { LightLock_Guard l(_l); mappings[(unsigned char)keyName] = mapping; }
    const angularRate gyroRate() { LightLock_Guard l(_l); return gRate; }
    const C3D_FVec gyroPos() { LightLock_Guard l(_l); return gPos; };
    void resetGyro(const C3D_FVec newPos) { LightLock_Guard l(_l); gPos = newPos; };
    const circlePosition circlePos() { LightLock_Guard l(_l); return cPos; }
    const circlePosition cStickPos() { LightLock_Guard l(_l); return csPos; }
    float gyroSensitivity() { LightLock_Guard l(_l); return gyro_s; }
    void setGyroSensitivity(float sensitivity) { LightLock_Guard l(_l); gyro_s = sensitivity; }
    float gyroDeadZone() { LightLock_Guard l(_l); return gyro_d; }
    void setGyroDeadZone(float min_rate) { LightLock_Guard l(_l); gyro_d = min_rate; }
};