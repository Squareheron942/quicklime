#include "slmdlloader.h"
#include <stdio.h>
#include <string>
#include "mesh.h"
#include "shader.h"
#include "defines.h"
#include "entt.hpp"
#include "console.h"
#include "materialmanager.h"
#include "bone.h"
#include "meshrenderer.h"
#include "threads.h"
#include <memory>
#include <unordered_map>
#include "sl_assert.h"
#include <string.h>

namespace {
	LightLock _loader_l = {1}, _mesh_l = {1};
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
    std::unordered_map<std::string, LightLock_Mutex<mesh>*> loadedMeshes;
    void meshdeleter(LightLock_Mutex<mesh>* data) {
    	LightLock_Guard l(_mesh_l);
        data->lock();
        auto it = std::find_if(std::begin(loadedMeshes), std::end(loadedMeshes), [& data](auto && pair) { return pair.second == data; });
        loadedMeshes.erase(it);
    }
    struct mdl_header {
   		unsigned int numVerts = 0;
	    unsigned char sv = 0;
	    float radius = 0.f;
	    unsigned char attrnum, attrtypes[16], attrlen[16];
    };
}

namespace mdlLoader {
    std::unique_ptr<shader> parseMat(const std::string file) {
    	FILE* f = fopen(file.c_str(), "r");
        ASSERT(f != nullptr, "Material file not found")
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

    std::shared_ptr<LightLock_Mutex<mesh>> parseModel(const std::string path, bool createnew) {
    	LightLock_Guard ll(_loader_l);
        LightLock_Guard ml(_mesh_l);
        if (loadedMeshes.find(path) == loadedMeshes.end()) {
			FILE *f = fopen(path.c_str(), "r");
			ASSERT(f != nullptr, "Model not found");
			char str[4] = "";
			fread(str, sizeof(char), 3, f);
			ASSERT(strcmp(str, "mdl"), "Error loading model");
			
			// read header info
			
			mdl_header mh = {0};

			// read basic info
			fread(&mh, sizeof(mdl_header), 1, f);
			
	        // read obj section
	        fread(str, sizeof(char), 3, f);

	        // read obj section
	        ASSERT(strcmp(str, "obj"), "Error loading model");
			
			// read bones
			[[maybe_unused]] bone* bones = parseBones(f);
			
			void* v = linearMemAlign(mh.numVerts * mh.sv, 0x1000); // aligned to page size
			
			fread(v, mh.sv, mh.numVerts, f);

			loadedMeshes[path] = new LightLock_Mutex<mesh>(mesh(v, mh.numVerts, mh.sv, mh.radius, mh.attrnum, mh.attrtypes, mh.attrlen));

			fclose(f);
        }
        std::shared_ptr<LightLock_Mutex<mesh>> m;
        m.reset(loadedMeshes[path], meshdeleter);
        return m;
    }
} 