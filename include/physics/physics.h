#pragma once

#include "3ds/types.h"

namespace ql {
	void physicsThread(void*);
	void physicsInit(u64 tickspeed);
	void physicsExit();
}