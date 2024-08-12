#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>

#include "physics.h"
#include "sceneloader.h"
// #include "scripts.inc"
// #include "shaders.inc"

#include "controls.h"
#include "defines.h"
#include "sl_time.h"
#include "console.h"
#include "componentmanager.h"
#include "scenemanager.h"

#include "shader.h"
#include "quit.h"
#include "physics.h"
#include "scenename.h"

namespace {
	void prgrminit() {
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
		ql::physicsInit(21887825); // 20ms tick speed
		// ql::physicsInit(54719563); // 50ms tick speed

		SceneLoader::load(ql::scenename);
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
	void prgrmexit() {
		ql::physicsExit();
		gfxExit();
		C3D_Fini();
		ndspExit();
		romfsExit();
	}
}

int main() {
	prgrminit();
	while (aptMainLoop() && !_quit) {
		update();
		draw();
	}
	prgrmexit();
	return 0;
}