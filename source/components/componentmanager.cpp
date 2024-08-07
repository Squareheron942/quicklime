#include "componentmanager.h"
#include "console.h"
#include <iostream>
#include "threads.h"

LightLock ComponentManager::_l = {};

bool ComponentManager::addComponent(const char* name, GameObject& obj, const void* data) {
	LightLock_Guard l(_l);
    if (getComponentMap().find(name) == getComponentMap().end()) {
        Console::error("Unknown component %s", name);
        return false;
    }
    getComponentMap()[name](obj, data);
    return true;
}

bool ComponentManager::addScript(const char* name, GameObject& obj) {
	LightLock_Guard l(_l);
    if (getScriptMap().find(name) == getScriptMap().end()) {
        Console::error("Unknown script %s", name);
        return false;
    }
    obj.scripts.push_back((getScriptMap()[name](obj)));
    return true;
}