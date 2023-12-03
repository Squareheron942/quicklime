#include "material.h"

material::~material() {}

bool material::loadTextureFromFile(C3D_Tex* tex, C3D_TexCube* cube, const char* path, bool vram) {
    FILE* f = fopen(path, "r");
    if (!f) return false;
    Tex3DS_Texture t3x = Tex3DS_TextureImportStdio(f, tex, cube, vram);
    
    if (!t3x)
        return false;
    fclose(f);
    // Delete the t3x object since we don't need it
    Tex3DS_TextureFree(t3x);
    return true;
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