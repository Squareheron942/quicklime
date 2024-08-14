#include "physics/physics.h"
#include "3ds/os.h"
#include "3ds/svc.h"
#include "config.h"
#include "console.h"
#include "defines.h"
#include "scenemanager.h"
#include "sl_assert.h"
#include "threads.h"
#include <3ds.h>
#include <cstdint>

namespace {
	static Handle event, timer;
	static bool exitphys = false;
	// static TickCounter cnt = {};
	// u64 convert_ticks_to_nanos(u64 ticks) {
	// 	return config::newmodel ? ((UINT64_C(0x13E466E50) * ticks) >> 32) :
	// ((UINT64_C(0x3BAD34AEF) * ticks) >> 32);
	// }
} // namespace

void ql::physicsThread(void *) {
	Console::log("Physics thread start");
	// u64 timestep = 20000000;
	// u64 maxstep = 100000000;
	// u64 before = svcGetSystemTick();
	while (!exitphys) {
		{
			LightLock_Guard l(SceneManager::lock);
			if (SceneManager::currentScene)
				SceneManager::currentScene->fixedUpdate();
		}

		svcWaitSynchronization(timer, -1);

		// u64 after = svcGetSystemTick();
		// s64 diff = (s64)(after - before);
		// if(diff > maxstep)
		// {
		//     diff = maxstep;
		//     before = after;
		// }
		// if(diff > 0)
		// {
		//     u64 t = convert_ticks_to_nanos(diff);
		// 	Console::log("New wait time %llu", t);
		//     Result res = svcWaitSynchronization(event, t);
		// 	ASSERT((res & 0x3FF) == 0x3FE, "Invalid wait result")

		//     before += timestep;
		// }
		// else if(diff <= -maxstep)
		// {
		//     before = after - maxstep;
		// }
	}
	Console::log("Physics thread exit");
}

void ql::physicsInit(uint64_t tickspeed) {
	svcCreateTimer(&timer, RESET_PULSE);
	svcSetTimer(timer, 0, tickspeed);
	svcCreateEvent(&event, RESET_ONESHOT);
	threadCreate(physicsThread, NULL, PHYSICS_THREAD_STACK_SZ, 0x18, -1, true);
}

void ql::physicsExit() { exitphys = true; }