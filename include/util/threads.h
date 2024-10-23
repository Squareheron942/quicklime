#pragma once

#include <3ds.h>
#include <type_traits>
#include <utility>

namespace ql {
	struct LightLock_Guard {
		LightLock &_lock;
		LightLock_Guard(LightLock &lock);
		~LightLock_Guard();
	};

	template <typename T>
		requires(!std::is_pointer_v<T>)
	class LightLock_Mutex {
		LightLock _lock;
		T _val;

	  public:
		LightLock_Mutex() : _val() { LightLock_Init(&_lock); }
		LightLock_Mutex(T &&val) : _val(std::move(val)) {
			LightLock_Init(&_lock);
		}
		T *operator->() { return &_val; }
		T &operator*() { return _val; }
		void lock() { LightLock_Lock(&_lock); }
		unsigned int try_lock() { return LightLock_TryLock(&_lock); }
		void unlock() { LightLock_Unlock(&_lock); }
		~LightLock_Mutex() { LightLock_Unlock(&_lock); }
	};

	template <typename T> class LightLock_Mutex_Guard {
		LightLock_Mutex<T> &mut;

	  public:
		LightLock_Mutex_Guard(LightLock_Mutex<T> &mut) : mut(mut) {
			mut.lock();
		}
		~LightLock_Mutex_Guard() { mut.unlock(); }
	};
} // namespace ql