#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <string.h>

#include "scripts.inc"
#include "materials.inc"
#include "scenemanager.h"
#include "sceneloader.h"
#include "controls.h"
#include "defines.h"
#include "sl_time.h"
#include "console.h"
#include "config.h"
#include "entt.hpp"
// #include "componentmanager.h"
#include "console.h"
#include "debug.h"

void exceptionHandler(void) {
	// // uninstall handler
	*((int*)((char*)getThreadLocalStorage() + 0x48)) = 0;
	*((int*)((char*)getThreadLocalStorage() + 0x44)) = 0;
	Console::error("Game Crashed. Press [START] to exit the app.");
	while(!controls::getDown(controls::key::KEY_START) && aptMainLoop()) {
		C3D_FrameBegin(0);
		controls::update();
		Time::Update();
		Console::update();
		C3D_FrameEnd(0);
	};
	exit(0);
}

int main()
{

	// setup exception handler
	// installExceptionHandler(exceptionHandler);

	// Initialize graphics
	gfxInitDefault();
	#if CONSOLE_ENABLED
	consoleInit(GFX_BOTTOM, NULL);
	#endif
	C3D_InitEx(C3D_DEFAULT_CMDBUF_SIZE * 8, 32, true);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);

    ndspInit();
	romfsInit();
	osSetSpeedupEnable(true);
	HIDUSER_EnableGyroscope();

	Console::init();
	SceneManager::init();

	config::getInfo();

	if (config::isOnCitra) Console::error("Citra detected");
	Console::log("Model detected: %u", config::model);
	Console::log("Language detected: %u", config::lang);
	Console::log("Region detected: %u", config::region);

	SceneLoader::load("test"); // immediate scene setting

	// Main loop
	while (aptMainLoop())
	{
		controls::update();
		SceneManager::update();
		Console::update();
		Time::Update();

		if (hidKeysDown() & KEY_START) {
			break;
		}

		// Render the scene
		C3D_FrameBegin(0);

		if (SceneManager::currentScene) { // don't allow the scene to be swapped
			osTickCounterStart(&stats::profiling::cnt_supd);
			SceneManager::currentScene->update();
			osTickCounterUpdate(&stats::profiling::cnt_supd);
			osTickCounterStart(&stats::profiling::cnt_camrnd);
			SceneManager::currentScene->drawTop();
			osTickCounterUpdate(&stats::profiling::cnt_camrnd);
		} else Console::error("No scene");

		C3D_FrameEnd(0);

		stats::profiling::rnd_camrnd = osTickCounterRead(&stats::profiling::cnt_camrnd);
		stats::profiling::go_supd = osTickCounterRead(&stats::profiling::cnt_supd);
	}

	for (int i = 0; i < CONSOLE_NUM_LINES; i++) // empty the console buffer to prevent memory leakage
		delete[] Console::textbuf[i];


	HIDUSER_DisableGyroscope();
	romfsExit();
	gfxExit();
	ndspExit();

	// Deinitialize graphics
	C2D_Fini();
	C3D_Fini();

	// uninstall exception handler
	uninstallExceptionHandler();
	return 0;
}