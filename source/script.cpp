#include "script.h"
#include "gameobject.h"
#include <string>

GameObject* Script::find(std::string object) {
    return owner->find(object);
}

Script::Script(GameObject& _owner) : parentComponents(_owner.reg), parentID(_owner.id), owner(&_owner) {}

void Script::SetEnabled(bool enabled) {
    this->enabled = enabled;
}