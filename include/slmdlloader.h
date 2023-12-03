#pragma once

#include <stdio.h>
#include <string>
#include "mesh.h"
#include "material.h"
#include "vertex.h"
#include "object3d.h"
#include "defines.h"
#include "entt.hpp"
#include "console.h"
#include "materialmanager.h"
#include "ink.h"
#include "fragmentlit.h"
#include "bone.h"
#include "meshrenderer.h"

namespace {
    static int freadstr(FILE* fid, char* str, size_t max_size)
    {
        int c;
        unsigned int count = 0;
        do {
            c = fgetc(fid);
            if (c == EOF) {
                /* EOF means either an error or end of file but
                * we do not care which. Clear file error flag
                * and return -1 */
                clearerr(fid);
                return -1;
            } else {
                /* Cast c to char */
                *str = (char) c;
                count++;
            }
        } while ((*(str++) != '\0') && (count < max_size));
        return count;
    }
    static char b_path[255];
}

namespace mdlLoader
{
    [[nodiscard]] material* parseMat(FILE* f) {
        char m_path[255], o_path[255];
        freadstr(f, m_path, 254); // get name of file from here
        sprintf(o_path, "%s%s.slmtl", b_path, m_path);
        FILE* mf = fopen(o_path, "r");
        if (!mf) return NULL;
        freadstr(mf, m_path, 254); // get name of material from here
        return MaterialManager::makeMaterial(m_path, mf);
    }

    [[nodiscard]] bone* parseBones(FILE* f) {
        unsigned char nBones = 0;
        fread(&nBones, sizeof(nBones), 1, f);
        if (!nBones) return NULL; // return null if no bones are used, prevents useless reading
        bone* bones = new bone[nBones];
        fread(bones, sizeof(bone), nBones, f);
        return bones;
    }

    inline bool addModel(const char path[], GameObject& object) {
        strcpy(b_path, path);
        *(strrchr(b_path, '/') + 1) = 0; // get parent folder path by looking for last '/' and putting a null after it

        FILE *f = fopen(path, "r");
        char str[3] = "";
        fread(str, sizeof(char), 3, f);
        if (str[0] != 'm' || str[1] != 'd' || str[2] != 'l') {
            Console::error("Wrong magic word '%c%c%c', 'mdl' expected (file pointer at position %p)", str[0], str[1], str[2], (void*)ftell(f));
            return NULL;
        }
        material* mat = parseMat(f);
        
        fread(str, sizeof(char), 3, f);

        // read obj section
        if (str[0] != 'o' || str[1] != 'b' || str[2] != 'j') {
            Console::error("Wrong magic word '%c%c%c', 'obj' expected (file pointer at position %p)\n", str[0], str[1], str[2], (void*)ftell(f));
            return NULL;
        }

        bone* bones = parseBones(f);

        int numVerts = 0; unsigned char sv = 0;char id = 0;


        fread(&id, sizeof(char), 1, f);
        fread(&numVerts, sizeof(int), 1, f);
        fread(&sv, sizeof(char), 1, f);

        const size_t sizevert = sv;

        Console::log("vs %lu nv %u", sizevert, numVerts);
        
        void* vertices = linearAlloc(numVerts * sizevert);
        Console::log("starting read at %p\n", (void*)ftell(f));
        fread(vertices, sizevert, numVerts, f);
        // for (int i = 0; i < numVerts; i++) Console::log("%u p%0.1f %0.1f %0.1f n%0.1f %0.1f %0.1f t%0.1f %0.1f\n", i, ((vertex*)vertices)[i].position[0], ((vertex*)vertices)[i].position[1], ((vertex*)vertices)[i].position[2], ((vertex*)vertices)[i].normal[0], ((vertex*)vertices)[i].normal[1], ((vertex*)vertices)[i].normal[2], ((vertex*)vertices)[i].texcoord[0], ((vertex*)vertices)[i].texcoord[1]);
        object.reg.emplace_or_replace<mesh>(object.id, vertices, numVerts, sv);
        object.reg.emplace_or_replace<MeshRenderer>(object.id, object, mat);

        return true;
    }
    
}