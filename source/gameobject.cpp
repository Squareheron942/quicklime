#include "gameobject.h"
#include "console.h"
#include "script.h"
#include "threads.h"

void GameObject::Awake() {
	LightLock_Guard l{_scriptL};
    for (Script* s : scripts) {
        s->Awake();
        s->SetEnabled(true);
    }
}

void GameObject::Start() {
	LightLock_Guard l{_scriptL};
    for (Script* s : scripts) if (s->enabled) s->Start();
}

void GameObject::Update() {
	LightLock_Guard l{_scriptL};
    for (Script* s : scripts) if (s->enabled) s->Update();
}

void GameObject::LateUpdate() {
	LightLock_Guard l{_scriptL};
    for (Script* s : scripts) if (s->enabled) s->LateUpdate();
}

void GameObject::FixedUpdate() {
	LightLock_Guard l{_scriptL};
    for (Script* s : scripts) if (s->enabled) s->FixedUpdate();
}


GameObject* GameObject::r_search(std::string name) {
	LightLock_Guard l{_l};
    GameObject* out = NULL;
    for (GameObject* child : children) {
        if (child->name == name) return child;
        out = child->r_search(name);
        if (out) return out;
    }
    return out;
}

GameObject* GameObject::find(std::string name) {
	LightLock_Guard l{_l};
    /**
     *     in front of name will search top down
     * /   in front of name will only search root (then find children based on '/' "subdirectories")
     * ./  in front of name will only search children
     * ../ in front of name will only search children of parent (do ../../ to get level above etc)
     */
    if (name[0] == '.')
    {
        if (name[1] == '.')
        {
            if (name[2] == '/')
            { // find in parent's children
                if (parent)
                {
                    /**
                     * Go back one level by telling parent to search its children
                     * change beginning to './' and tell parent to find it
                    */
                    return parent->find(name.substr(1));
                } else Console::warn("Object %s not found");
                return nullptr;
            }
        } else if (name[1] == '/') // find in own children
        {
            std::stringstream ss;
            ss << name.substr(2); // remove the './'
            std::string subname;
            GameObject* out = this; // start at current spot

            while (std::getline(ss, subname, '/') && out) { // for each level
                GameObject* temp = out;
                for (GameObject* obj : out->children) {
                    if (obj->name == subname) {
                        out = obj;
                        break;
                    }
                }
                if (temp == out) return nullptr; // if a child was not found in one level, it was not found so return nothing
            }


            return out;
        }
    } else if (name[0] == '/') // find at root
    {
        GameObject* root = this;
        while (root->parent) root = root->parent;

        return root->find(name.substr(1));
    } else // find anywhere
    {
        GameObject* root = this;
        while (root->parent) root = root->parent; // get root of all objects (that way we don't need to store this at all times)

        int i = 0;
        bool hasChild = false;

        while (name[i] != 0) {
            if (name[i] == '/') {
                hasChild = true;
                break;
            }
            i++;
        }

        root = root->r_search(name.substr(0, i)); // recursively search for the first item

        if (root && hasChild) root = root->find("./" + name.substr(i));

        return root;
    }
    return nullptr;
};

GameObject::~GameObject() {
	// Console::log("GameObject destructor");
    if (parent) parent->removeChild(this);
    if (id != entt::null) reg.destroy(id);
    for (auto* s : scripts) delete s;
    // Console::log("End of gameobject destructor code");
}