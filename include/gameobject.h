#pragma once

#include <entt.hpp>
#include "console.h"
#include "threads.h"
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

class Scene;
class Script;

class GameObject {
	friend class Camera;
    friend class ComponentManager;
    friend class AudioSource;
    
    GameObject* r_search(std::string name);
    LightLock _l, _componentL, _scriptL;
    Scene& s;
    public:
    std::vector<GameObject*> children; // non owning, only viewer
    std::vector<Script*> scripts; // cannot be component since you can't have more than 1 object of type per entity
    GameObject* parent = NULL;
    entt::registry &reg;
    entt::entity id;
    std::string name; // saved in scene file
    unsigned int layer; // by default on base layer (layer 1)
    GameObject(entt::registry& registry, std::string name, Scene& s) :
    	s(s),
    	reg(registry),
        id(registry.create()),
        layer(0x1)
    {
    	LightLock_Init(&_l);
	    LightLock_Init(&_componentL);
	    LightLock_Init(&_scriptL);
    }
    GameObject(GameObject&& other) :
    	_l(other._l),
        s(other.s),
	    children(other.children),
		scripts(other.scripts),
	    parent(other.parent),
    	reg(other.reg),
        id(other.id),
        name(other.name),
        layer(other.layer)
    {
    	other.parent = nullptr;
        other.id = entt::entity{entt::null};
        other.scripts.clear();
        other.children.clear();
        other.name = "";
        other.layer = 0;
    	Console::log("GameObject move constructor");
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
    inline void addComponent(Args&& ...args) { LightLock_Guard l(_componentL); reg.emplace_or_replace<T>(id, std::forward<Args>(args)...); }

    /**
     * @brief Get the Component object
     *
     * @tparam T Component to get
     * @return T* Pointer to the component instance
     */
    template<typename T> inline T* getComponent() { LightLock_Guard l(_componentL); return reg.try_get<T>(id); }

    /**
     * @brief Adds child to self
     *
     * @param object Reference to GameObject to add as child
     */
    inline void addChild(GameObject& object) {
    	LightLock_Guard l(_l); 
        object.setParent(*this);
        children.push_back(&object);
    }

    /**
     * @brief Removes object from list of children
     *
     * @param object GameObject to remove
     */
    inline void removeChild(GameObject& object) {
    	LightLock_Guard l(_l); 
        children.erase(std::remove(children.begin(), children.end(), &object), children.end());
        // children.remove(&object);
        object.parent = nullptr;
    }

    /**
     * @brief Removes object from list of children
     *
     * @param object GameObject to remove
     */
    inline void removeChild(GameObject* object) {
    	LightLock_Guard l(_l); 
        if (!object) return;
        // if (children.front() == children.end()) return;
        children.erase(std::remove(children.begin(), children.end(), object), children.end());
        // children.remove(object);
        object->parent = nullptr;
    }

    inline void setParent(GameObject& object) {
    	LightLock_Guard l(_l); 
     	if (this->parent) this->parent->removeChild(this);
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