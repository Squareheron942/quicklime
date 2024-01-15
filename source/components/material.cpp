#include "material.h"
#include "tex3ds.h"
#include <string>
#include "console.h"
#include <unordered_map>
#include <memory>
#include <optional>
#include "texture.h"

namespace {
    struct t3xcfg_t {
        bool vram : 1 = false; // defaults to not in VRAM since less space there
        GPU_TEXTURE_FILTER_PARAM magFilter : 1 = GPU_LINEAR, minFilter: 1 = GPU_NEAREST;
        GPU_TEXTURE_WRAP_PARAM wrapS: 2 = GPU_REPEAT, wrapT: 2 = GPU_REPEAT;
    };
    std::unordered_map<std::string, Texture*> loadedTex;
    void texdeleter(Texture* tex) {
        if (tex) C3D_TexDelete(&tex->tex);
        delete loadedTex[tex->name];
        loadedTex.erase(tex->name);
        Console::log("texture deleted");
    }
}

material::~material() {}

std::optional<std::shared_ptr<Texture>> material::loadTextureFromFile(std::string name) {
    if (name.size() == 0) name = "blank"; // no texture, load blank white texture
    if (loadedTex.find(name) == loadedTex.end()) {
        FILE* f = fopen(("romfs:/gfx/" + name + ".t3x").c_str(), "r");
        if (!f) {
            // Console::warn("File not found:");
            // Console::warn(name.c_str());
            name = "blank";
            f = fopen(("romfs:/gfx/" + name + ".t3x").c_str(), "r");
            // return {};
        } // texture not found, return nothing

        FILE* cfg = fopen(("romfs:/gfx/" + name + ".t3xcfg").c_str(), "r");
        
        t3xcfg_t texcfg;

        if (cfg) fread(&texcfg, sizeof(t3xcfg_t), 1, cfg);

        Texture* tex = new Texture(name); 

        loadedTex[name] = tex;

        Tex3DS_Texture t3x = Tex3DS_TextureImportStdio(f, &tex->tex, &tex->cube, texcfg.vram);
        if (!t3x) return {};

        C3D_TexSetFilter(&tex->tex, texcfg.magFilter, texcfg.minFilter);
        C3D_TexSetWrap(&tex->tex, texcfg.wrapS, texcfg.wrapT);

        
        // Delete the t3x object since we don't need it
        Tex3DS_TextureFree(t3x);
        // Close the files since we are done with them
        fclose(f);
        fclose(cfg);
    }
    return std::shared_ptr<Texture>(loadedTex[name], texdeleter); // if it already exists just return a pointer to it
}

bool material::loadTextureFromMem(C3D_Tex* tex, C3D_TexCube* cube, const void* data, size_t size)
{
    Tex3DS_Texture t3x = Tex3DS_TextureImport(data, size, tex, cube, false);
    if (!t3x)
        return false;

    // Delete the t3x object since we don't need it
    Tex3DS_TextureFree(t3x);
    return true;
}

int material::freadstr(FILE* fid, char* str, size_t max_size)
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