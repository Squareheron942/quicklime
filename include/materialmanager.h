#pragma once

#include <unordered_map>
#include <utility>
#include <stdio.h>
#include "material.h"
#include "console.h"
#include "gameobject.h"

typedef material*(*matInstantiationFunc)(FILE*);

class MaterialManager {
    MaterialManager() {}
    public:
    template<typename T> static material* createMaterialInstance(FILE* matFile) { return new T(matFile); }
    // template<typename T> static void createMaterialInstance(GameObject& obj, FILE* matF) { obj.reg.emplace_or_replace<T>(obj.id, matF); }
    
    template<typename T> static bool registerMaterial(const char* name) {
        getMaterialMap()[name] = MaterialManager::createMaterialInstance<T>;
        // Console::log("Material %s added", name);
        return true;
    }

    static material* makeMaterial(const char* name, FILE* matF);

    private:
        // Use Meyer's singleton to prevent SIOF
        static std::unordered_map<std::string, matInstantiationFunc>& getMaterialMap() { static std::unordered_map<std::string, matInstantiationFunc> map; return map; }
};

#define MATERIAL_REGISTER(material) bool material ## _material = MaterialManager::registerMaterial<material>(#material);