#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>

#include "sceneloader.h"
#include "scripts.inc"
#include "shaders.inc"

#include "controls.h"
#include "defines.h"
#include "sl_time.h"
#include "console.h"
#include "componentmanager.h"
#include "scenemanager.h"

#include "shader.h"
#include "quit.h"

void init();
void update();
void draw();

int main() {
	init();
	while (aptMainLoop() && !_quit) {
		update();
		draw();
	}
	return 0;
}

void init() {
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE * 64);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);

	#if CONSOLE
	consoleInit(GFX_BOTTOM, NULL);
	#endif

	controls::init();
	Console::init();
	ComponentManager::init();
	ndspInit();
	romfsInit();
	osSetSpeedupEnable(true);

	SceneLoader::load("test_loading_perf");

}

void update() {
	controls::update();
	SceneManager::update();
	Console::update();
	Time::Update();
}

void draw() {
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	SceneManager::draw();
	C3D_FrameEnd(0);
}