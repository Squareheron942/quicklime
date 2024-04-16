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
#include "componentmanager.h"
#include "console.h"

void sceneExit(void)
{
	// SceneManager::currentScene.reset();
	HIDUSER_DisableGyroscope();
	romfsExit();
}

void exceptionHandler(void) {
	// uninstall handler
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
	void* tls = getThreadLocalStorage();

	void (**handlerptr)(void) = (void(**)())((char*)tls + 0x40);
	*handlerptr = exceptionHandler;
	*((int*)((char*)tls + 0x44)) = 1;
	*((int*)((char*)tls + 0x48)) = 0;

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

	config::getInfo();

	if (config::isOnCitra) Console::error("Citra detected");
	Console::log("Model detected: %u", config::model);
	Console::log("Language detected: %u", config::lang);
	Console::log("Region detected: %u", config::region);

	SceneManager::currentScene = SceneLoader::load("test");

	int scene = 0;

	// Main loop
	while (aptMainLoop())
	{
		controls::update();

		if (hidKeysDown() & KEY_START) {
			++scene;
			if (scene == 1) break; // break in order to return to hbmenu
		}


		osTickCounterStart(&stats::profiling::cnt_supd);
		if (SceneManager::currentScene) SceneManager::currentScene->update();
		osTickCounterUpdate(&stats::profiling::cnt_supd);
		stats::profiling::go_supd = osTickCounterRead(&stats::profiling::cnt_supd);
		Time::Update();
		// Render the scene
		C3D_FrameBegin(0);

		osTickCounterStart(&stats::profiling::cnt_camrnd);
		if (SceneManager::currentScene) SceneManager::currentScene->drawTop();
		else Console::error("No scene");
		osTickCounterUpdate(&stats::profiling::cnt_camrnd);
		stats::profiling::rnd_camrnd = osTickCounterRead(&stats::profiling::cnt_camrnd);

		C3D_FrameEnd(0);
	}

	for (int i = 0; i < CONSOLE_NUM_LINES; i++) // empty the console buffer to prevent memory leakage
		delete[] Console::textbuf[i];


	// Deinitialize the scene
	sceneExit();

	// Deinitialize graphics
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	ndspExit();

	// uninstall exception handler
	*handlerptr = NULL;
	*((int*)((char*)tls + 0x44)) = 0;
	*((int*)((char*)tls + 0x48)) = 0;
	return 0;
}