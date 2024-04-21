#pragma once
#include <3ds.h>

inline void installExceptionHandler(void(*handler)()) {
	// setup exception handler
	void* tls = getThreadLocalStorage();
	*(void(**)())((char*)tls + 0x40) = handler;
	*((int*)((char*)tls + 0x44)) = 1;
	*((int*)((char*)tls + 0x48)) = 0;
}

inline void uninstallExceptionHandler(void) {
	// setup exception handler
	void* tls = getThreadLocalStorage();
	*(void(**)())((char*)tls + 0x40) = NULL;
	*((int*)((char*)tls + 0x44)) = 0;
	*((int*)((char*)tls + 0x48)) = 0;
}