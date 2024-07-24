#include "threads.h"

LightLock_Guard::LightLock_Guard(LightLock& lock): _lock(lock) {
	LightLock_Lock(&_lock);
}

LightLock_Guard::~LightLock_Guard() {
	LightLock_Unlock(&_lock);
}