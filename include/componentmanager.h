#pragma once

#include <unordered_map>
#include <utility>
#include "script.h"
#include "entt.hpp"
#include <stdio.h>
#include "gameobject.h"
#include "console.h"
#include <string>


class ComponentManager {
    ComponentManager() {}
    template<typename T> static Script* createScriptInstance(GameObject& owner) { return new T(owner); }
    template<typename T> static void createComponentInstance(GameObject& obj, const void* data) { obj.reg.emplace_or_replace<T>(obj.id, obj, data); }
    
    public:
    template<typename T> static bool registerComponent(const char* name) {
        if constexpr (std::is_base_of_v<Script, T>) getScriptMap()[name] = ComponentManager::createScriptInstance<T>;
        else getComponentMap()[name] = ComponentManager::createComponentInstance<T>;
        return true;
    }

    static bool addComponent(const char* name, GameObject& obj, const void* data = NULL);

    static bool addScript(const char* name, GameObject& obj);

    private:
        // Use Meyer's singleton to prevent SIOF
        static std::unordered_map<std::string, Script*(*)(GameObject&)>& getScriptMap() { static std::unordered_map<std::string, Script*(*)(GameObject&)> map; return map; }
        static std::unordered_map<std::string, void(*)(GameObject&, const void*)>& getComponentMap() { static std::unordered_map<std::string, void(*)(GameObject&, const void*)> map; return map; }
};

#define COMPONENT_REGISTER(component) bool component ## _component = ComponentManager::registerComponent<component>(#component);