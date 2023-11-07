// #include "entt.hpp"
// #include "componentmanager.h"
// #include "script.h"
// #include <unordered_map>

// std::unordered_map<const char*, Script*(*)(entt::registry&, entt::entity)> ComponentManager::map;

// template<typename T> bool ComponentManager::registerComponent(const char* name) {
//     map[name] = ComponentManager::createInstance<T>;
//     return true;
// }

// template<typename T> Script* ComponentManager::createInstance(entt::registry& _parent_c, entt::entity _parent_id) { return new T(_parent_c, _parent_id);}