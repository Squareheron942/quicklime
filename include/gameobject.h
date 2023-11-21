#pragma once

#include "entt.hpp"
#include "script.h"
#include <vector>

class GameObject {
    public:
    entt::registry &reg;
    // std::vector<Script*> scripts;
    std::vector<Script*> scripts;
    // Script* scripts[2];
    entt::entity id;
    GameObject(entt::registry& registry) : reg(registry), id(registry.create()) {}
    operator entt::entity() { return id; }
};