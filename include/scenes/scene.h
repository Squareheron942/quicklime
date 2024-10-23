#pragma once

#include "camera.h"
#include "gameobject.h"
#include <3ds.h>
#include <citro3d.h>
#include <entt.hpp>
#include <list>
#include <stdio.h>
#include <string>

namespace ql {
	class Scene {
		LightLock lock;
		std::string _name;
		GameObject *root;
		// needs to be a list so it doesn't get reallocated (sad)
		std::list<GameObject> objects;
		entt::registry reg;

		friend class SceneLoader;
		friend class Camera;
		friend class AudioSource;
		friend class AudioManager;
		friend class GameObject;
		friend void sceneLoadThread(void *params);

		void act_on_objects(void (GameObject::*action)()) {
			for (GameObject &child : objects)
				(child.*action)();
		}

	  public:
		const std::string &name = _name;
		~Scene();

		void awake();
		void start();
		void update();
		void fixedUpdate();
		void draw();

		Scene(std::string name);
	};
} // namespace ql