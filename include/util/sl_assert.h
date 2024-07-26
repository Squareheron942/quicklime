#pragma once

#include <stdio.h>
#include "defines.h"
#include <3ds.h>

#if DEBUG
#define ASSERT(condition, msg) if (!(condition)) { printf("Assertion failed: %s\nIn function: %s\n%s:%u", #condition, __func__, __FILE_NAME__, __LINE__); for(;;); }
#else
#define ASSERT(condition, msg)
#endif