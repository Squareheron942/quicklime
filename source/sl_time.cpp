#include "sl_time.h"

namespace Time {
    namespace {
        unsigned long long curtime, oldtime;
    }
    float deltaTime = 0.0167f;
    void Update() {
        curtime = osGetTime();
        deltaTime = (curtime - oldtime) * 0.001f;
        oldtime = curtime;
    }
}
