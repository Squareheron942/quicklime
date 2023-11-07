#pragma once

#include "entt.hpp"
#include "script.h"

class GameObject {
    entt::registry &reg;
    public:
    Script* scripts[1];
    entt::entity id;
    GameObject(entt::registry& registry) : reg(registry), id(registry.create()) {}
};