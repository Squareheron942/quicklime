#include "dae_default_material.h"

dae_default_material::~dae_default_material() {
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}

void dae_default_material::setMaterial(C3D_Mtx* modelview, C3D_Mtx* projection) {
    if (!opaque) 
        C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_COLOR);
    C3D_BindProgram(&program);
    C3D_TexBind(0, &tex->tex);
    C3D_LightEnvMaterial(&lights::lightenv, &mat); // give the lighting stuff the settings for this object
    C3D_LightEnvLut(&lights::lightenv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lut_Light); // set the shininess value
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR); // multiply by diffuse lighting and add specular lighting
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);


    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  modelview);
}

void dae_default_material::resetMaterial() {
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_AlphaTest(false, GPU_GEQUAL, 0);
    C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_ALL); 
}

dae_default_material::dae_default_material(FILE* args) noexcept {
    char texpath[255] = {0};
    vshader_dvlb = DVLB_ParseFile((u32*)fragvshader_dae_shbin, fragvshader_dae_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
    uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

    LightLut_Phong(&lut_Light, 300);

    dae_default_material_args m_args;

    if (args) {
        fread(&m_args, sizeof(dae_default_material_args), 1, args);
        freadstr(args, texpath, 255);
    }

    if (!(tex = loadTextureFromFile(texpath).value())) { // should load into vram which is potentially an issue for space reasons? 
        Console::error("%s unable to load texture", texpath);
    }
    mat.ambient[0] = m_args.ambient[0];
    mat.ambient[1] = m_args.ambient[1];
    mat.ambient[2] = m_args.ambient[2];

    mat.diffuse[0] = m_args.diffuse[0];
    mat.diffuse[1] = m_args.diffuse[1];
    mat.diffuse[2] = m_args.diffuse[2];

    mat.specular0[0] = m_args.specular0[0];
    mat.specular0[1] = m_args.specular0[1];
    mat.specular0[2] = m_args.specular0[2];

    mat.emission[0] = m_args.emission[0];
    mat.emission[1] = m_args.emission[1];
    mat.emission[2] = m_args.emission[2];

    opaque = m_args.opaque;
    
}

MATERIAL_REGISTER(dae_default_material)