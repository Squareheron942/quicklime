#pragma once

#include <3ds.h>
#include <utility>

struct LightLock_Guard {
	LightLock& _lock;
	LightLock_Guard(LightLock& lock);
	~LightLock_Guard();
};

template<typename T>
struct LightLock_Mutex {
	LightLock _lock;
	T _val;
	LightLock_Mutex(T&& val): _val(std::move(val)) { LightLock_Init(&_lock); }
	T& operator->() { return _val; }
	void lock() { LightLock_Lock(&_lock); }
	unsigned int try_lock() { return LightLock_TryLock(&_lock); }
	void unlock() { LightLock_Unlock(&_lock); }
	~LightLock_Mutex() { LightLock_Unlock(&_lock); }
};