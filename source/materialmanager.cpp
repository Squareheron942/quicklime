#include "materialmanager.h"
#include "console.h"
#include "material.h"
#include "gameobject.h"

material* MaterialManager::makeMaterial(const char* name, FILE* matF) {
    if (getMaterialMap().find(name) == getMaterialMap().end()) {
        Console::error("Unknown material %s", name);
        return NULL;
    }
    return (getMaterialMap()[name](matF));
}