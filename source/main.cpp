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

void sceneExit(void)
{
	// SceneManager::currentScene.reset();
	HIDUSER_DisableGyroscope();
	romfsExit();
}

int main()
{

	// Initialize graphics
	gfxInitDefault();
	#if CONSOLE_ENABLED
		consoleInit(GFX_BOTTOM, NULL);
	#endif
	C3D_InitEx(C3D_DEFAULT_CMDBUF_SIZE * 8, 32, true);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);

	romfsInit();

	config::getInfo();

	if (config::isOnCitra) Console::error("Citra detected");
	Console::log("Model detected: %u", config::model);
	Console::log("Language detected: %u", config::lang);
	Console::log("Region detected: %u", config::region);

	osSetSpeedupEnable(true);

	SceneManager::currentScene = SceneLoader::load("Fld_Plaza");

	HIDUSER_EnableGyroscope();

	int scene = 0;

	// Main loop
	while (aptMainLoop())
	{
		controls::update();
		
		if (hidKeysDown() & KEY_START) {
			++scene;
			if (scene == 1) break; // break in order to return to hbmenu
			if (scene == 1) {
				SceneManager::currentScene.reset(); 
				SceneManager::currentScene = SceneLoader::load("Fld_Plaza");
			}
		}
			

		osTickCounterStart(&stats::profiling::cnt_supd);
		SceneManager::currentScene->update();
		osTickCounterUpdate(&stats::profiling::cnt_supd);
		stats::profiling::go_supd = osTickCounterRead(&stats::profiling::cnt_supd);
		Time::Update();
		// Render the scene
		C3D_FrameBegin(0);

		osTickCounterStart(&stats::profiling::cnt_camrnd);
		SceneManager::currentScene->drawTop();
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
	return 0;
}