#pragma once

#include <3ds.h>

struct LightLock_Guard {
	LightLock& _lock;
	LightLock_Guard(LightLock& lock);
	~LightLock_Guard();
};