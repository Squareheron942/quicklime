#include "slmdlloader.h"
#include <stdio.h>
#include <string>
#include "mesh.h"
#include "material.h"
#include "vertex.h"
#include "defines.h"
#include "entt.hpp"
#include "console.h"
#include "materialmanager.h"
#include "ink.h"
#include "fragmentlit.h"
#include "bone.h"
#include "meshrenderer.h"
#include "dae_default_material.h"
#include <memory>
#include <unordered_map>

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
    std::unordered_map<std::string, void*> loadedVertices;
    void meshdeleter(void* data) {
        auto it = std::find_if(std::begin(loadedVertices), std::end(loadedVertices), [& data](auto && pair) { return pair.second == data; });
        loadedVertices.erase(it);
        linearFree(data); // free the vertex data itself
        Console::log("mesh vertices freed");
    }
}

namespace mdlLoader {
    material* parseMat(FILE* f) {
        if (!f) return NULL;
        char m_name[255];
        freadstr(f, m_name, 254); // get name of material from here
        return MaterialManager::makeMaterial(m_name, f);
    }

    bone* parseBones(FILE* f) {
        unsigned char nBones = 0;
        fread(&nBones, sizeof(nBones), 1, f);
        if (!nBones) return NULL; // return null if no bones are used, prevents useless reading
        bone* bones = new bone[nBones];
        fread(bones, sizeof(bone), nBones, f);
        return bones;
    }

    bool addModel(const std::string path, GameObject& object, bool createnew) {
        char b_path[255];
        strcpy(b_path, path.c_str()); 
        *(strrchr(b_path, '/') + 1) = 0; // get parent folder path by looking for last '/' and putting a null after it (basically just changes the end of the string to be immediately after the /)
        FILE *f = fopen((path + ".slmdl").c_str(), "r");

        if (!f) {
            Console::error("Error: model not found");
            Console::error(path.c_str());
            return false;
        }

        char str[3] = "";
        fread(str, sizeof(char), 3, f);
        if (str[0] != 'm' || str[1] != 'd' || str[2] != 'l') {
            Console::error("Wrong magic word '%c%c%c', 'mdl' expected (file pointer at position %p)", str[0], str[1], str[2], (void*)ftell(f));
            return false;
        }
        
        // read header info

        // get material name
        char m_path[255], o_path[255];
        freadstr(f, m_path, 254); // get name of file from here
        snprintf(o_path, 255, "%s%s.slmtl", b_path, m_path); // max path size 255
        FILE* mf = fopen(o_path, "r");

        material* mat = parseMat(mf);

        fclose(mf);
        
        unsigned int numVerts = 0; 
        unsigned char sv = 0;
        float radius = 0.f;
        unsigned char attrnum, attrtypes[16], attrlen[16];

        // read next data
        fread(&numVerts, sizeof(int), 1, f);
        fread(&sv, sizeof(char), 1, f);
        fread(&attrnum, sizeof(char), 1, f);
        fread(&attrtypes, sizeof(char), attrnum, f);
        fread(&attrlen, sizeof(char), attrnum, f);
        fread(&radius, sizeof(float), 1, f);

        // read obj section
        fread(str, sizeof(char), 3, f);

        // read obj section
        if (str[0] != 'o' || str[1] != 'b' || str[2] != 'j') {
            Console::error("Wrong magic word '%c%c%c', 'obj' expected (file pointer at position %p)\n", str[0], str[1], str[2], (void*)ftell(f));
            return false;
        }

        bone* bones = parseBones(f);

        const size_t sizevert = sv;

        // Console::log("vs %lu nv %u, rad %f", sizevert, numVerts, radius * 0.01f);
        
        std::shared_ptr<void> vertices;
        if (loadedVertices.find(path) == loadedVertices.end() || createnew) { // only read data if needed
            void* v = linearAlloc(numVerts * sizevert);
            fread(v, sizevert, numVerts, f);
            vertices.reset(v, meshdeleter);
            
            if (!createnew) loadedVertices[path] = v;

        } else vertices.reset(loadedVertices[path], meshdeleter);

        fclose(f);
        
        // typedef struct {float position[3]; float normal[3]; float texcoord[2];} daevert;
        // for (unsigned int i = 0; i < numVerts; i++) Console::log("%u t%0.1f %0.1f\n", i, ((daevert*)vertices)[i].texcoord[0], ((daevert*)vertices)[i].texcoord[1]);
        object.reg.emplace_or_replace<mesh>(object.id, object, vertices, numVerts, sv, radius * 0.01f, attrnum, attrtypes, attrlen);
        object.reg.emplace_or_replace<MeshRenderer>(object.id, object, mat);

        return true;
    }
}