#pragma once
#include <3ds.h>

struct handler_inf {
	u32 padding[16];
	void (*handler)();
	int arg1 = 1, arg2 = 0;
};

inline void installExceptionHandler(void (*handler)()) {
	static_assert(offsetof(handler_inf, handler) == 0x40,
				  "Invalid handler offset");
	*(handler_inf *)getThreadLocalStorage() = handler_inf{{}, handler, 1, 0};
}

inline void uninstallExceptionHandler(void) {
	// setup exception handler
	// void* tls = getThreadLocalStorage();
	// *(void(**)())((char*)tls + 0x40) = NULL;
	// *((int*)((char*)tls + 0x44)) = 0;
	// *((int*)((char*)tls + 0x48)) = 0;
	static_assert(offsetof(handler_inf, handler) == 0x40,
				  "Invalid handler offset");
	*(handler_inf *)getThreadLocalStorage() = handler_inf{{}, NULL, 1, 0};
}