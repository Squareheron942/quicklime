#pragma once

#include <stdio.h>
#include <string>
#include "mesh.h"
#include "material.h"
#include "vertex.h"
#include "object3d.h"
#include "defines.h"
#include "entt.hpp"
#include "fragmentlit.h"
#include "ink.h"
#include "console.h"

namespace mdlLoader
{
    material* parseMat(FILE* f) {
        char m_path[255]; // max path length is 255 chars (probably fine)
        fgets(m_path, 255, f); // get name of file from here
        return new fragment_lit(fopen(m_path, "r"));
    }

    inline object3d* load(const char path[], entt::registry &reg) {
        FILE *f = fopen(path, "r");
        char str[3] = "";
        fread(str, sizeof(char), 3, f);
        if (str[0] != 'm' || str[1] != 'd' || str[2] != 'l') {
            Console::error("Wrong magic word '%c%c%c', 'mdl' expected (file pointer at position %p)", str[0], str[1], str[2], (void*)ftell(f));
            return NULL;
        }
        // fseek(f, 0, SEEK_END);
        fseek(f, 5, SEEK_CUR);
        // material *m = parseMat(f);
        material* m = NULL;
        
        fread(str, sizeof(char), 3, f);

        // read obj section
        if (str[0] != 'o' || str[1] != 'b' || str[2] != 'j') {
            Console::error("Wrong magic word '%c%c%c', 'obj' expected (file pointer at position %p)\n", str[0], str[1], str[2], (void*)ftell(f));
            return NULL;
        }
        int numVerts = 0; size_t sv = 0;char id = 0;

        fread(&id, sizeof(char), 1, f);
        fread(&numVerts, sizeof(int), 1, f);
        fread(&sv, sizeof(char), 1, f);

        const size_t sizevert = sv;
        
        void* vertices = linearAlloc(numVerts * sizevert);
        Console::log("starting obj read at position %p\n", (void*)ftell(f));
        fseek(f, 0x28, SEEK_SET);
        fread(vertices, sizevert, numVerts, f);
        #if CONSOLE_ENABLED
            for (int i = 0; i < numVerts; i++) Console::log("%u p%0.1f %0.1f %0.1f n%0.1f %0.1f %0.1f t%0.1f %0.1f\n", i, ((vertex*)vertices)[i].position[0], ((vertex*)vertices)[i].position[1], ((vertex*)vertices)[i].position[2], ((vertex*)vertices)[i].normal[0], ((vertex*)vertices)[i].normal[1], ((vertex*)vertices)[i].normal[2], ((vertex*)vertices)[i].texcoord[0], ((vertex*)vertices)[i].texcoord[1]);
        #endif
        Mesh *mesh = new Mesh(vertices, numVerts, sizevert);
        object3d *object = new object3d(reg, mesh, m);
        return object;
    }
    
} // namespace mdlLoader