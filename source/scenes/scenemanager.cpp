#include "scenemanager.h"
// #include "camera.h"
#include "3ds/synchronization.h"
#include "threads.h"
#include <memory>

bool SceneManager::loadSceneNextFrame = false;
std::unique_ptr<Scene> SceneManager::currentScene, SceneManager::sceneToBeLoaded;
LightLock SceneManager::lock;

void SceneManager::init() {
  // disallows setting a new scene while also updating what the new scene will be
  LightLock_Init(&lock);
}

void SceneManager::setScene(std::unique_ptr<Scene>& s) {
	if (!s)	return; // make sure it is a real scene (non null)

	LightLock_Guard l(lock);
    sceneToBeLoaded = std::move(s);
    loadSceneNextFrame = true;
}

void SceneManager::setScene(std::unique_ptr<Scene> &&s) {
	LightLock_Guard l(lock);
	if (!s)	return; // make sure it is a real scene (non null)
    sceneToBeLoaded = std::move(s);
    loadSceneNextFrame = true;
}

// must only be called from main thread
void SceneManager::update() {
	LightLock_Guard l(lock);
	if (currentScene) currentScene->update();
	
	if (!loadSceneNextFrame)
    	return;

    //changing the scene

    // don't reload it again next frame
    loadSceneNextFrame = false;

    // first delete old scene
    Console::log("Scene try reset");
    currentScene.reset();
    Console::success("Scene reset");

    // then swap in new scene
    currentScene = std::move(sceneToBeLoaded);

    // start the scene objects
    Console::log("Try wake up");
    SceneManager::currentScene->awake();
    Console::success("Scene woke up");
    Console::log("Scene starting");
    SceneManager::currentScene->start();
    Console::success("Scene started");
}

void SceneManager::draw() {
	LightLock_Guard l(lock);
	if (!currentScene) return;
	currentScene->draw();
}