#include "script.h"

Script::Script(entt::registry &_parent_c, entt::entity _parent_id) : parentComponents(_parent_c), parentID(_parent_id) {}
void Script::SetEnabled(bool enabled) {this->enabled = enabled;}