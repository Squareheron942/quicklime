#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

#include "physics.h"
#include "sceneloader.h"

#include "audiomanager.h"
#include "componentmanager.h"
#include "console.h"
#include "controls.h"
#include "defines.h"
#include "scenemanager.h"
#include "ql_time.h"

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

		ndspInit();
		romfsInit();
		osSetSpeedupEnable(true);
		ql::controls::init();
		ql::Console::init();
		ql::ComponentManager::init();
		ql::AudioManager::init();
		ql::physicsInit(21887825); // 20ms tick speed
		// ql::physicsInit(54719563); // 50ms tick speed

		ql::SceneLoader::load(ql::scenename);
	}
	void update() {
		ql::controls::update();
		ql::SceneManager::update();
		ql::Console::update();
		ql::Time::Update();
	}

	void draw() {
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		ql::SceneManager::draw();
		C3D_FrameEnd(GX_CMDLIST_FLUSH);
	}
	void prgrmexit() {
		ql::physicsExit();
		gfxExit();
		C3D_Fini();
		ndspExit();
		romfsExit();
	}
} // namespace

int main() {
	prgrminit();
	while (aptMainLoop()) {
		update();
		draw();
	}
	prgrmexit();
	return 0;
}