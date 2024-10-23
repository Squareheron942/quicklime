#pragma once

#include "scene.h"
#include "sceneloader.h"
#include "threads.h"
#include <citro3d.h>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

// TODO needs to be basically completely rewritten/remade
class SceneManager {
	SceneManager() = delete;
	static bool loadSceneNextFrame;
	static void setScene(std::unique_ptr<Scene> &s);
	static void setScene(std::unique_ptr<Scene> &&s);
	friend class SceneLoader;
	friend class Console; // given access to read scene name
	friend class AudioManager;
	friend void sceneLoadThread(void *params);

  public:
	static LightLock lock;
	static std::unique_ptr<Scene> currentScene, sceneToBeLoaded;
	static void init();
	static void update();
	static void draw();
};