#pragma once
#include "3ds/errf.h"
#include <3ds.h>
#include <cstdint>

namespace ql {
	typedef void (*handler_func)(ERRF_ExceptionData *info);

	inline void installExceptionHandler(handler_func handler) {
		uint32_t *tls = (uint32_t *)getThreadLocalStorage();
		tls[16]		  = (uint32_t)handler;
		tls[17]		  = 1;
		tls[18]		  = 1;
		
	}

	inline void uninstallExceptionHandler(void) {
		uint32_t *tls = (uint32_t *)getThreadLocalStorage();
		tls[16]		  = (uint32_t)nullptr;
		tls[17]		  = 1;
		tls[18]		  = 0;
	}
} // namespace ql
