#include "componentmanager.h"
#include "console.h"
#include "threads.h"
#include <iostream>

LightLock ql::ComponentManager::_l = {};

bool ql::ComponentManager::addComponent(const char *name, GameObject &obj,
									const void *data) {
	LightLock_Guard l(_l);
	if (getComponentMap().find(name) == getComponentMap().end()) {
		Console::error("Unknown component %s", name);
		Console::error("Known components:");
		for(const auto& elem : getComponentMap())
			Console::log("%s: %p", elem.first.c_str(), elem.second);
		return false;
	}
	Console::error("Known components:");
	for(const auto& elem : getComponentMap())
		Console::log("%s: %p", elem.first.c_str(), elem.second);
	getComponentMap()[name](obj, data);
	return true;
}

bool ql::ComponentManager::addScript(const char *name, GameObject &obj) {
	LightLock_Guard l(_l);
	if (getScriptMap().find(name) == getScriptMap().end()) {
		Console::error("Unknown script %s", name);
		return false;
	}
	getScriptMap()[name](obj);
	return true;
}