#pragma once

#include <type_traits>
#include <unordered_map>
#include <utility>
#include <stdio.h>
#include <3ds.h>
#include "shader.h"
#include "shader.h"
#include <memory>

typedef shader*(*shaderInstantiationFunc)(FILE*);
template<typename T>
concept validshader = requires(FILE* f) { T(f); } && std::is_base_of_v<shader, T>;

class MaterialManager {
	static LightLock _l;
    MaterialManager() = delete;
    template<typename T> static shader* createMaterialInstance(FILE* matFile) { return new T(matFile); }
    static std::unordered_map<std::string, shaderInstantiationFunc>& getMaterialMap() { static std::unordered_map<std::string, shaderInstantiationFunc> map; return map; }
    public:
    template<typename T> requires validshader<T> static bool registerMaterial(const char* name) { getMaterialMap()[name] = MaterialManager::createMaterialInstance<T>; return true; }
    static std::unique_ptr<shader> makeMaterial(const char* name, FILE* matF);
};

#define MATERIAL_REGISTER(material) bool material ## _material = MaterialManager::registerMaterial<material>(#material);