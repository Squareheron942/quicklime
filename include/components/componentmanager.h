#pragma once

#include "gameobject.h"
#include "script.h"
#include <3ds.h>
#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

template <typename T>
concept validcomponent = requires(GameObject &obj, const void *data) {
							 T(obj, data);
						 } || std::is_base_of_v<Script, T>;

class ComponentManager {
	static LightLock _l;
	ComponentManager() {}
	template <typename T> static void attachScript(GameObject &owner) {
		owner.scripts.push_back(std::make_unique<T>(owner));
	}
	template <typename T>
	static void attachComponent(GameObject &obj, const void *data) {
		obj.reg.emplace_or_replace<T>(obj.id, obj, data);
	}

  public:
	template <typename T>
		requires validcomponent<T>
	static bool registerComponent(const char *name) {
		if constexpr (std::is_base_of_v<Script, T>)
			getScriptMap()[name] = ComponentManager::attachScript<T>;
		else
			getComponentMap()[name] = ComponentManager::attachComponent<T>;
		return true;
	}

	static inline bool init() {
		LightLock_Init(&_l);
		return true;
	}

	static bool addComponent(const char *name, GameObject &obj,
							 const void *data = nullptr);
	static bool addScript(const char *name, GameObject &obj);

  private:
	// Use Meyer's singleton to prevent SIOF
	static std::unordered_map<std::string, void (*)(GameObject &)> &
	getScriptMap() {
		static std::unordered_map<std::string, void (*)(GameObject &)> map;
		return map;
	}
	static std::unordered_map<std::string, void (*)(GameObject &, const void *)>
		&getComponentMap() {
		static std::unordered_map<std::string,
								  void (*)(GameObject &, const void *)>
			map;
		return map;
	}
};

#define COMPONENT_REGISTER(component)                                          \
	bool component##_component =                                               \
		ComponentManager::registerComponent<component>(#component);