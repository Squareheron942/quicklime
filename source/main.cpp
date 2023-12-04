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

#define CLEAR_COLOR 0x3477ebFF
#define CITRA_TYPE 0x20000
#define CITRA_VERSION 11

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

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

C3D_RenderTarget* targetWide = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);

void test() {
	C3D_RenderTargetSetOutput(targetWide,  GFX_TOP, GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);
	C3D_RenderTargetClear(targetWide, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
	C3D_FrameDrawOn(targetWide);
}

int main()
{

	// Initialize graphics
	gfxInitDefault();
	#if CONSOLE_ENABLED
		consoleInit(GFX_BOTTOM, NULL);
	#endif
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	romfsInit();

	Console::log("Running on %c3DS", osGetApplicationMemType() > 5 ? 'N' : 'O');

	

	cfguInit();
	CFGU_GetSystemModel((u8*)&config::model);
	CFGU_GetSystemLanguage((u8*)&config::lang);
	CFGU_SecureInfoGetRegion((u8*)&config::region);

	cfguExit(); // since it doesn't change at runtime we can just init, read, close

	s64 version = 0;
	svcGetSystemInfo(&version, CITRA_TYPE, CITRA_VERSION); // magic idk
	config::isOnCitra = (version != 0);

	config::wideIsUnsupported = config::isOnCitra || config::model == CFG_MODEL_2DS;

	if (config::isOnCitra) Console::error("Citra detected");
	Console::log("Model detected: %u", config::model);
	Console::log("Language detected: %u", config::lang);
	Console::log("Region detected: %u", config::region);
	if (config::wideIsUnsupported) Console::log("Wide mode unsupported, disabling");
	else Console::log("Wide mode supported, enabling");

	Scene1 s;

	
	// Initialize the render target
	// C3D_RenderTarget* targetWide  = C3D_RenderTargetCreate(240, 400 * (config::wideIsUnsupported ? 1 : 2), GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8); // double width fbuf when wide mode is used
	// C3D_RenderTarget* targetLeft  = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	// C3D_RenderTarget* targetRight = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	// C3D_RenderTargetSetOutput(targetRight, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);
	
	// #if BOTTOM_SCREEN_ENABLED
	// 	C3D_RenderTarget* targetBottom = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	// 	C3D_RenderTargetSetOutput(targetBottom, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
	// #endif

	HIDUSER_EnableGyroscope();

	// Main loop
	while (aptMainLoop())
	{
		controls::update();

		// float slider = osGet3DSliderState();
		// float iod = slider/3;

		
		if (hidKeysDown() & KEY_START)
			break; // break in order to return to hbmenu

		// SceneManager::currentScene->update();
		s.update();
		Time::Update();
		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
			
		
			// if (iod > 0.0f)
			// {
			// 	// gfxSet3D(true);
			// 	// C3D_RenderTargetSetOutput(targetLeft,  GFX_TOP, GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);
			// 	// C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
			// 	// C3D_FrameDrawOn(targetLeft);
			// 	// SceneManager::currentScene->drawTop(-iod);
			// 	// C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
			// 	// C3D_FrameDrawOn(targetRight);
			// 	// SceneManager::currentScene->drawTop(iod);
			// } else {
			//	// gfxSetWide(!config::wideIsUnsupported); // only enable if supported
			//	// C3D_RenderTargetSetOutput(targetWide,  GFX_TOP, GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);
			//	// C3D_RenderTargetClear(targetWide, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
			//	// C3D_FrameDrawOn(targetWide);
			// 	// SceneManager::currentScene->drawTop(iod);
			// }
			// SceneManager::currentScene->drawTop(0);
			s.drawTop(0);
			// C3D_RenderTargetSetOutput(targetWide,  GFX_TOP, GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);
			// C3D_RenderTargetClear(targetWide, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
			// C3D_FrameDrawOn(targetWide);
			// #if BOTTOM_SCREEN_ENABLED
            // C3D_RenderTargetClear(targetBottom, C3D_CLEAR_ALL, 0x000000, 0);
			// C3D_FrameDrawOn(targetBottom);
			// C2D_SceneTarget(targetBottom);
            // SceneManager::currentScene->drawBottom();
			// #endif

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