#include "entt.hpp"
#include "componentmanager.h"
#include "gameobject.h"
#include "console.h"
#include "script.h"

bool ComponentManager::addComponent(const char* name, GameObject& obj, void* data) {
    if (getComponentMap().find(name) == getComponentMap().end()) {
        Console::error("Unknown component %s", name);
        return false;
    }
    getComponentMap()[name](obj, data);
    return true;
}

bool ComponentManager::addScript(const char* name, GameObject& obj) {
    if (getScriptMap().find(name) == getScriptMap().end()) {
        Console::error("Unknown script %s", name);
        return false;
    }
    obj.scripts.push_back((getScriptMap()[name](obj)));
    return true;
}