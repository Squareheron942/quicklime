#pragma once

#include "entt.hpp"
#include <string>

class GameObject;

class Script {
    entt::registry &parentComponents;
    entt::entity parentID;
    

    protected:
    GameObject* owner;
    template<class T> inline T *GetComponent() {
        return parentComponents.try_get<T>(parentID);
    }

    GameObject* find(std::string object);

    public:
    Script(GameObject& owner);
    bool enabled = true;
    void SetEnabled(bool enabled);
    virtual void Start(void) = 0;
    virtual void Update(void) = 0;
    virtual void FixedUpdate(void) = 0;
};