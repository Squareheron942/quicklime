#include "scene.h"
#include "audiosource.h"
#include "camera.h"
#include "console.h"

namespace ql {
	Scene::Scene(std::string name) : _name(name), name(_name) {
		LightLock_Init(&lock);
	}

	Scene::~Scene() {
		Console::log("Scene destructor");
		objects.clear();
	}

	void Scene::awake() {
		LightLock_Lock(&lock);
		act_on_objects(&GameObject::Awake); // call awake() on every gameobject
											// and enable them (to self disable
											// do it when this is called)
		LightLock_Unlock(&lock);
	}

	void Scene::start() {
		LightLock_Init(&lock);
		LightLock_Guard l(lock);
		act_on_objects(&GameObject::Start); // start all scripts
	}

	void Scene::update() {
		LightLock_Guard l(lock);
		act_on_objects(
			&GameObject::Update); // call update() on every gameobject
								  // (propagates from root)

		// whatever other per frame logic stuff will get called here
		// reg.view<AudioSource>().each([](auto &as) { as.update(); });

		// call lateupdate() on every gameobject (propagates from root).
		// Used to ensure stuff like cameras move only when everything else is
		// done moving
		act_on_objects(&GameObject::LateUpdate);
	};

	void Scene::fixedUpdate() {
		LightLock_Guard l(lock);
		// all the physics stuff will go here
		act_on_objects(&GameObject::FixedUpdate);
	};

	void Scene::draw() {
		LightLock_Lock(&lock);
		reg.view<Camera>().each([](auto &cam) { cam.Render(); });
		LightLock_Unlock(&lock);
	};
} // namespace ql