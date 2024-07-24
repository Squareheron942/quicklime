#pragma once

#include <stdio.h>
#include "defines.h"
#include <3ds.h>

#define ASSERT(condition, msg) if (!(condition)) { printf("Assertion failed: %s\nIn function: %s\n%s:%u", #condition, __func__, __FILE_NAME__, __LINE__); svcSleepThread(10000000); svcBreak(USERBREAK_ASSERT); }