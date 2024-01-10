#include <3ds.h>
#include <citro3d.h>
#include <string.h>

#include "scenemanager.h"
#include "sceneloader.h"
#include "controls.h"
#include "defines.h"
#include "sl_time.h"
#include "console.h"
#include "config.h"
#include "entt.hpp"
#include "componentmanager.h"
#include "scripts.inc"

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
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE * 5);

	romfsInit();

	config::getInfo();

	if (config::isOnCitra) Console::error("Citra detected");
	Console::log("Model detected: %u", config::model);
	Console::log("Language detected: %u", config::lang);
	Console::log("Region detected: %u", config::region);

	SceneManager::currentScene = SceneLoader::load("scene1");

	HIDUSER_EnableGyroscope();

	// Main loop
	while (aptMainLoop())
	{
		controls::update();
		
		if (hidKeysDown() & KEY_START)
			break; // break in order to return to hbmenu

		SceneManager::currentScene->update();
		Time::Update();
		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		SceneManager::currentScene->drawTop();

		C3D_FrameEnd(0);
	}

	for (int i = 0; i < CONSOLE_NUM_LINES; i++) // empty the console buffer to prevent memory leakage
		delete[] Console::textbuf[i];
	

	// Deinitialize the scene
	sceneExit();

	// Deinitialize graphics
	C3D_Fini();
	gfxExit();
	return 0;
}