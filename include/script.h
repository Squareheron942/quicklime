#pragma once

#include "entt.hpp"

class Script {
    entt::registry &parentComponents;
    entt::entity parentID;

    protected:
    template<class T> inline T *GetComponent() {
        return parentComponents.try_get<T>(parentID);
    }

    public:
    Script(entt::registry &_parent_c, entt::entity _parent_id);
    bool enabled = true;
    void SetEnabled(bool enabled);
    virtual void Start(void) = 0;
    virtual void Update(void) = 0;
    virtual void FixedUpdate(void) = 0;
};