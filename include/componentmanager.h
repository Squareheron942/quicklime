#pragma once

#include <unordered_map>
#include <utility>
#include "script.h"
#include "entt.hpp"
#include "transform.h"
#include <stdio.h>

// template<typename ...Args> std::unordered_map<char*, Script*(*)(entt::registry&, entt::entity, Args&& ...args)> class_map_type;

class ComponentManager {
    ComponentManager() {}
    public:
    template<typename T> static Script* createScriptInstance(entt::registry& _parent_c, entt::entity _parent_id) { return new T(_parent_c, _parent_id); }
    template<typename T> static void createComponentInstance(entt::registry& _parent_c, entt::entity _parent_id) { _parent_c.emplace<T>(_parent_id); }
    
    template<typename T> static bool registerComponent(const char* name) {
        if constexpr (std::is_base_of_v<Script, T>) {
            getScriptMap()[name] = ComponentManager::createScriptInstance<T>;
        } else {
            getComponentMap()[name] = ComponentManager::createComponentInstance<T>;
        }
        return true;
    }

    static void addComponent(const char* name, entt::registry& reg, entt::entity e) {
        getComponentMap()[name](reg, e);
    }

    static Script* addScript(const char* name, entt::registry& reg, entt::entity e) {
        return (getScriptMap()[name](reg, e));
    }

    private:
        // Use Meyer's singleton to prevent SIOF
        static std::unordered_map<const char*, Script*(*)(entt::registry&, entt::entity)>& getScriptMap() {
            static std::unordered_map<const char*, Script*(*)(entt::registry&, entt::entity)> map;
            return map;
        }
        static std::unordered_map<const char*, void(*)(entt::registry&, entt::entity)>& getComponentMap() {
            static std::unordered_map<const char*, void(*)(entt::registry&, entt::entity)> map;
            return map;
        }
};

#define COMPONENT_REGISTER(component) bool component ## _component = ComponentManager::registerComponent<component>(#component);