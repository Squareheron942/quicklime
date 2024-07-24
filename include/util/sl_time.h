#pragma once

class Time {
	Time() = delete;
    static unsigned long long curtime, oldtime;
    static float _delta;
    public:
    static const float& deltaTime;
    static void Update();
};
