#include "scenemanager.h"
#include "camera.h"
#include "threads.h"

bool SceneManager::loadSceneNextFrame = false;
std::unique_ptr<Scene> SceneManager::currentScene, SceneManager::sceneToBeLoaded;
LightLock SceneManager::lock;

void SceneManager::init() {
  // disallows setting a new scene while also updating what the new scene will be
  LightLock_Init(&lock);
}

void SceneManager::setScene(std::unique_ptr<Scene> &s) {
	if (!s)	return; // make sure it is a real scene (non null)

	LightLock_Guard l(lock);

    // cursed old solution which was stupid and bad and stupid
    // memcpy(&sceneToBeLoaded, &s, sizeof(std::unique_ptr<Scene>));

    sceneToBeLoaded = std::move(s);
    loadSceneNextFrame = true;
}

void SceneManager::setScene(std::unique_ptr<Scene> &&s) {
	if (!s)	return; // make sure it is a real scene (non null)

	LightLock_Guard l(lock);
    sceneToBeLoaded = std::move(s);
    loadSceneNextFrame = true;
}

// must only be called from main thread
void SceneManager::update() {
	if (!loadSceneNextFrame)
    	return;

    //changing the scene
    LightLock_Guard l(lock);

    // don't reload it again next frame
    loadSceneNextFrame = false;

    // first delete old scene
    Console::log("Scene try reset");
    SceneManager::currentScene.reset();
    Console::success("Scene reset");

    // then swap in new scene
    SceneManager::currentScene = std::move(sceneToBeLoaded);

    // start the scene objects
    Console::log("Try wake up");
    SceneManager::currentScene->awake();
    Console::success("Scene woke up");
    Console::log("Scene starting");
    SceneManager::currentScene->start();
    Console::success("Scene started");
    if (SceneManager::currentScene->cameras.size() == 0) Console::error("No cameras found");
    Camera::mainTop = SceneManager::currentScene->cameras[0]; // set current camera to be first camera
    Camera::mainTop->sceneRoot = SceneManager::currentScene->root;
    Console::success("Main camera initialized");
}