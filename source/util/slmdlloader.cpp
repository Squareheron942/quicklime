#include "slmdlloader.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <string.h>

#include "mesh.h"
#include "shader.h"
#include "console.h"
#include "materialmanager.h"
#include "bone.h"
#include "threads.h"
#include "sl_assert.h"

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
    LightLock_Mutex<std::unordered_map<std::string, std::weak_ptr<mesh>>> loadedMeshes;
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

    std::optional<std::shared_ptr<mesh>> parseModel(const std::string& path, bool createnew) {
    	LightLock_Guard ll(_loader_l);
        LightLock_Guard ml(_mesh_l);
        LightLock_Mutex_Guard mut(loadedMeshes);
        
        if (path.size() == 0) return std::nullopt; // empty name so nothing to be done
        if (loadedMeshes->find(path) != loadedMeshes->end() && !loadedMeshes->find(path)->second.expired()) 
        	return (*loadedMeshes)[path].lock(); // already been loaded and is valid
         
        // not already loaded or previously loaded but expired
        FILE *f;
        {
	        std::shared_ptr<mesh> m;
	        
			f = fopen(path.c_str(), "r");
			ASSERT(f != nullptr, "Model not found");
			if (!f) goto exit_0;
			
			char str[4] = "";
			fread(str, sizeof(char), 3, f);
			ASSERT(strcmp(str, "mdl"), "Error loading model");
			if (!strcmp(str, "mdl")) goto exit_1;
			
			// read header info
			
			mdl_header mdlheader;
	
			// read basic info
			fread(&mdlheader, sizeof(mdl_header), 1, f);
			
	        // read obj section
	        fread(str, sizeof(char), 3, f);
	        ASSERT(strcmp(str, "obj"), "Error loading model");
	        if (!strcmp(str, "obj")) goto exit_1;
			
			// read bones
			[[maybe_unused]] bone* bones = parseBones(f);
			
			void* v = linearMemAlign(mdlheader.numVerts * mdlheader.sv, 0x1000); // aligned to page size
			
			fread(v, mdlheader.sv, mdlheader.numVerts, f);
			fclose(f);
	
			m = std::make_shared<mesh>(v, mdlheader);
			
			// If the user explicitly wants to create a new mesh, make it completely independent
			if (!createnew) (*loadedMeshes)[path] = m;
			
	        return m;
        }
        exit_1:
        fclose(f);
        exit_0:
        return std::nullopt;
    }
} 