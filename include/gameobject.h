#pragma once

#include "entt.hpp"
#include <forward_list>
#include <string>
#include <sstream>
#include "console.h"

class Script;

class GameObject {
    std::forward_list<GameObject*> children;
    GameObject* parent;

    GameObject* r_search(std::string name);

    public:
    entt::registry &reg;
    std::list<Script*> scripts;
    entt::entity id;
    std::string name;
    GameObject(entt::registry& registry) : reg(registry), id(registry.create()) {}
    operator entt::entity() { return id; }
    /**
     * @brief Adds child to self
     * 
     * @param object Reference to GameObject to add as child
     */
    inline void addChild(GameObject& object) {
        object.setParent(*this);
        children.push_front(&object);
    }

    /**
     * @brief Removes object from list of children
     * 
     * @param object GameObject to remove
     */
    inline void removeChild(GameObject& object) { 
        children.remove(&object);
        object.parent = NULL;
    }

    inline void setParent(GameObject& object) { 
        this->parent = &object;
    }

    /**
     * @brief Finds an object within the scene.
     * 
     * WARNING very slow, it is only intended to find the object in the Start() function of a script (do NOT use it every frame).
     * Instead, store the pointer in a variable and reuse it in the future.
     * 
     *
     * " "   in front of name will search top down.
     * "/"   in front of name will only search root (then find children based on '/' "subdirectories").
     * "./"  in front of name will only search children.
     * "../" in front of name will only search children of parent (do ../../ to get level above etc).
     * 
     * @param name The name of the GameObject to search for
     * @return A pointer to the found GameObject, or null if not found
     */
    GameObject* find(std::string name);

    ~GameObject();
};