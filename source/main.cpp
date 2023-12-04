#include <3ds.h>
#include <citro3d.h>
#include <string.h>

#include "scenemanager.h"
#include "scene1.h"
#include "controls.h"
#include "defines.h"
#include "sl_time.h"
#include "console.h"
#include "config.h"

void sceneExit(void)
{
	// SceneManager::currentScene.reset();
	HIDUSER_DisableGyroscope();
	romfsExit();
}

void printfile(const char* path)
{
	FILE* f = fopen(path, "r");
	if (f)
	{
		char mystring[100];
		while (fgets(mystring, sizeof(mystring), f))
		{
			int a = strlen(mystring);
			if (mystring[a-1] == '\n')
			{
				mystring[a-1] = 0;
				if (mystring[a-2] == '\r')
					mystring[a-2] = 0;
			}
			puts(mystring);
		}
		printf(">>EOF<<\n");
		fclose(f);
	}
}

#include "entt.hpp"
#include "componentmanager.h"

int main()
{

	// Initialize graphics
	gfxInitDefault();
	#if CONSOLE_ENABLED
		consoleInit(GFX_BOTTOM, NULL);
	#endif
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	romfsInit();

	config::getInfo();

	if (config::isOnCitra) Console::error("Citra detected");
	Console::log("Model detected: %u", config::model);
	Console::log("Language detected: %u", config::lang);
	Console::log("Region detected: %u", config::region);
	if (config::wideIsUnsupported) Console::log("Wide mode unsupported, disabling");
	else Console::log("Wide mode supported, enabling");

	SceneManager::load<Scene1>();

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

		SceneManager::currentScene->drawTop(0);

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