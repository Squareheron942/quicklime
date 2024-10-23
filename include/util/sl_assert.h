#pragma once

#include "defines.h"
#include <3ds.h>
#include <stdio.h>

#if DEBUG
#define ASSERT(condition, msg)                                                 \
	do {                                                                       \
		if (!(condition)) {                                                    \
			printf("Assertion failed: %s\nIn function: %s\n%s:%u\n%s",         \
				   #condition, __func__, __FILE_NAME__, __LINE__, msg);        \
			for (;;)                                                           \
				;                                                              \
		}                                                                      \
	} while (0)
#else
#define ASSERT(condition, msg) (void)(condition)
#endif