#include "shader.h"
#include "tex3ds.h"
#include <string>
#include "console.h"
#include <unordered_map>
#include <memory>
#include <optional>
#include "texture.h"
#include "threads.h"

namespace {
    struct t3xcfg_t {
        bool vram : 1 = false; // defaults to not in VRAM since less space there
        GPU_TEXTURE_FILTER_PARAM magFilter : 1 = GPU_LINEAR, minFilter: 1 = GPU_NEAREST;
        GPU_TEXTURE_WRAP_PARAM wrapS: 2 = GPU_REPEAT, wrapT: 2 = GPU_REPEAT;
    };
    std::unordered_map<std::string, std::weak_ptr<Texture>> loadedTex;
}

std::optional<std::shared_ptr<Texture>> createTextureInstance(const std::string& name) {
	std::shared_ptr<Texture> tex = std::make_shared<Texture>(name);
    
	FILE* cfg = fopen(("romfs:/gfx/" + name + ".t3xcfg").c_str(), "r");
    if (!cfg) return std::nullopt;
    t3xcfg_t texcfg;
    fread(&texcfg, sizeof(t3xcfg_t), 1, cfg);
    fclose(cfg);
	
    FILE* f = fopen(("romfs:/gfx/" + name + ".t3x").c_str(), "r");
    if (!f) return std::nullopt;  // texture not found, return nothing
    
    Tex3DS_Texture t3x = Tex3DS_TextureImportStdio(f, &tex->tex, &tex->cube, texcfg.vram);
    fclose(f); // no more use for the texture file
    if (!t3x) return std::nullopt;
    
    C3D_TexSetFilter(&tex->tex, texcfg.magFilter, texcfg.minFilter);
    C3D_TexSetWrap(&tex->tex, texcfg.wrapS, texcfg.wrapT);

    // Delete the t3x object since we don't need it
    Tex3DS_TextureFree(t3x);
    
    return tex;
}

shader::~shader() {}

[[nodiscard]] std::optional<std::shared_ptr<Texture>> shader::loadTextureFromFile(std::string name) {
	if (name.size() == 0) return std::nullopt; // no texture so do nothing
	
	std::optional<std::shared_ptr<Texture>> out;
	
	auto it = loadedTex.find(name);
    if (it == loadedTex.end() || it->second.expired()) { // texture needs to be loaded from scratch
    	out = createTextureInstance(name);
        if (!out.has_value()) return std::nullopt; // texture couldn't be found
        loadedTex[name] = out.value(); 
    }
    return out; // if it already exists just return a pointer to it
}

bool shader::loadTextureFromMem(C3D_Tex* tex, C3D_TexCube* cube, const void* data, size_t size)
{
    Tex3DS_Texture t3x = Tex3DS_TextureImport(data, size, tex, cube, false);
    if (!t3x)
        return false;

    // Delete the t3x object since we don't need it
    Tex3DS_TextureFree(t3x);
    return true;
}

int shader::freadstr(FILE* fid, char* str, size_t max_size)
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