#pragma once

#include <stdio.h>
#include <string>
#include "mesh.h"
#include "material.h"
#include "vertex.h"
#include "object3d.h"
#include "defines.h"
#include "entt.hpp"

namespace mdlLoader
{
    material *parseMat(FILE* f) {
        int numbers[4] = { 0 };
        fread(numbers, sizeof(int), 4, f);
        return new material();
    }
    inline object3d* load(const char path[], entt::registry &reg) {
        FILE *f = fopen(path, "rb");
        char str[3] = "";
        fread(str, sizeof(char), 3, f);
        if (str[0] != 'm' || str[1] != 'd' || str[2] != 'l') {
            #if CONSOLE_ENABLED
                printf("error: wrong magic word '%c%c%c', 'mdl' expected\n(file pointer at position %p)", str[0], str[1], str[2], (void*)ftell(f));
            #endif
            return NULL;
        }
        // fseek(f, 0, SEEK_END);
        fseek(f, 5, SEEK_CUR);
        material *m = parseMat(f);
        
        fread(str, sizeof(char), 3, f);
        

        // read obj section
        if (str[0] != 'o' || str[1] != 'b' || str[2] != 'j') {
            #if CONSOLE_ENABLED
                printf("error: wrong magic word '%c%c%c', 'obj' expected\n(file pointer at position %p)", str[0], str[1], str[2], (void*)ftell(f));
            #endif
            return NULL;
        }
        int numVerts = 0; size_t sv = 0;char id = 0;

        fread(&id, sizeof(char), 1, f);
        fread(&numVerts, sizeof(int), 1, f);
        fread(&sv, sizeof(char), 1, f);

        const size_t sizevert = sv;
        #if CONSOLE_ENABLED
            printf(" 1 id %u nv %u sv %u\n", id, numVerts, sizevert);
        #endif
        
        void* vertices = linearAlloc(numVerts * sizevert);
        printf("starting obj read at position %p", (void*)ftell(f));
        fseek(f, 0x28, SEEK_SET);
        fread(vertices, sizevert, numVerts, f);
        printf("pos1 %f\n", ((vertex*)vertices)->position[0]);
        Mesh mesh(vertices, numVerts, sizevert);
        object3d *object = new object3d(reg, &mesh, m);
        return object;
    }
    
} // namespace mdlLoader