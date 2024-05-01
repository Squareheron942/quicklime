#pragma once

#include "entt.hpp"
#include <vector>
#include <string>
#include <sstream>
#include "console.h"

class Script;

class GameObject {

    GameObject* r_search(std::string name);

    public:
    std::vector<GameObject*> children;
    GameObject* parent = NULL;
    entt::registry &reg;
    std::list<Script*> scripts; // cannot be component since you can't have more than 1 object of type per entity
    entt::entity id;
    std::string name; // saved in scene file
    unsigned short layer, renderer; // by default on base layer (layer 1)
    GameObject(entt::registry& registry) :
        reg(registry),
        id(registry.create()),
        layer(0x1),
        renderer(0x0)
    {
    	Console::log("GameObject constructor");
    }
    GameObject(GameObject& other) :
        reg(other.reg),
        id(other.id),
        layer(other.layer),
        renderer(other.renderer),
        name(other.name),
        parent(other.parent),
        children(other.children),
        scripts(other.scripts)
    {
    	Console::log("GameObject constructor");
    }
    operator entt::entity() { return id; }

    void Start(void);
    void Update(void);
    void FixedUpdate(void);
    void LateUpdate(void);
    void Awake(void);

    /**
     * @brief Adds a component to GameObject at runtime. If the GameObject already has the component, this one will replace it.
     *
     * @tparam T Component to add
     * @tparam Args Argument types
     *
     * @param args Arguments to pass to the component constructor
     */
    template<typename T, typename ...Args>
    inline void addComponent(Args&& ...args) {
        reg.emplace_or_replace<T>(id, std::forward<Args>(args)...);
    }

    /**
     * @brief Get the Component object
     *
     * @tparam T Component to get
     * @return T* Pointer to the component instance
     */
    template<typename T> inline T* getComponent() { return reg.try_get<T>(id); }

    /**
     * @brief Adds child to self
     *
     * @param object Reference to GameObject to add as child
     */
    inline void addChild(GameObject& object) {
        object.setParent(*this);
        children.push_back(&object);
    }

    /**
     * @brief Removes object from list of children
     *
     * @param object GameObject to remove
     */
    inline void removeChild(GameObject& object) {
        children.erase(std::remove(children.begin(), children.end(), &object), children.end());
        object.parent = NULL;
    }

    /**
     * @brief Removes object from list of children
     *
     * @param object GameObject to remove
     */
    inline void removeChild(GameObject* object) {
        children.erase(std::remove(children.begin(), children.end(), object), children.end());
        object->parent = NULL;
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