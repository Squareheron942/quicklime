#include <3ds.h>
#include <citro3d.h>
#include <string.h>

#include "scenemanager.h"
#include "scene1.h"
#include "controls.h"
#include "defines.h"
#include "sl_time.h"
#include "console.h"
#include <citro2d.h>

#define CLEAR_COLOR 0x3477ebFF

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

void sceneExit(void)
{
	SceneManager::currentScene.reset();
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

struct example {
	template<typename T> static void emplaceComponent(entt::registry& reg, entt::entity e) {
		reg.emplace<T>(e);
	}
};


int main()
{

	// Initialize graphics
	gfxInitDefault();
	gfxSet3D(true);
	#if CONSOLE_ENABLED
		consoleInit(GFX_BOTTOM, NULL);
	#endif
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);

	romfsInit();

	Console::log("Running on %c3DS", osGetApplicationMemType() > 5 ? 'N' : 'O');

	SceneManager::load<Scene1>();

	// Initialize the render target
	C3D_RenderTarget* targetLeft  = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTarget* targetRight = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);C3D_RenderTargetSetOutput(targetLeft,  GFX_TOP, GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);
	C3D_RenderTargetSetOutput(targetRight, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);
	
	#if BOTTOM_SCREEN_ENABLED
		C3D_RenderTarget* targetBottom = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
		C3D_RenderTargetSetOutput(targetBottom, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
	#endif

	HIDUSER_EnableGyroscope();

	// Main loop
	while (aptMainLoop())
	{
		controls::update();

		float slider = osGet3DSliderState();
		float iod = slider/3;

		
		if (controls::getDown("start"))
			break; // break in order to return to hbmenu

		SceneManager::currentScene->update();
		Time::Update();
		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

			C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
			C3D_FrameDrawOn(targetLeft);
			SceneManager::currentScene->drawTop(-iod);

			if (iod > 0.0f)
			{
				C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
				C3D_FrameDrawOn(targetRight);
				SceneManager::currentScene->drawTop(iod);
			}
			#if BOTTOM_SCREEN_ENABLED
            C3D_RenderTargetClear(targetBottom, C3D_CLEAR_ALL, 0x000000, 0);
			C3D_FrameDrawOn(targetBottom);
			C2D_SceneTarget(targetBottom);
			#endif
            SceneManager::currentScene->drawBottom();

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