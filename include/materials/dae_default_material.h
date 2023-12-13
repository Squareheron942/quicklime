#pragma once
#include "material.h"
#include "fragvshader_dae_shbin.h" // for per-fragment lighting that is expecting dae models
#include "3ds.h"
#include "materialmanager.h"
#include "lights.h"

namespace {
    struct dae_default_material_args {
        const float ambient[4], diffuse[4], specular0[4], specular1[4], emission[4];
        // const char* texpath; // can't have strings in here since variable length, need to parse them separately
    };
}


class dae_default_material : public material {
    public:
    C3D_Tex tex;
    C3D_Material mat;
    DVLB_s *vshader_dvlb;
	shaderProgram_s program;
    int uLoc_projection, uLoc_modelView;
	C3D_LightLut lut_Light;
    dae_default_material(FILE* args) noexcept {
        mat = { // default settings that work mostly fine for anything
            { 0.1f, 0.1f, 0.1f }, //ambient
            { 0.4f, 0.4f, 0.4f }, //diffuse
            { 0.8f, 0.8f, 0.8f }, //specular0
            { 0.0f, 0.0f, 0.0f }, //specular1
            { 0.0f, 0.0f, 0.0f }, //emission
        };
        char texpath[255];
        vshader_dvlb = DVLB_ParseFile((u32*)fragvshader_dae_shbin, fragvshader_dae_shbin_size);
        shaderProgramInit(&program);
        shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
        uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
        uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
        

        LightLut_Phong(&lut_Light, 300);
        C3D_LightEnvLut(&lights::lightenv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lut_Light);

        if (!loadTextureFromFile(&tex, NULL, "romfs:/gfx/kitten.t3x", true)) // also a placeholder 
            Console::warn("unable to load kitten backup texture");
        C3D_TexSetFilter(&tex, GPU_LINEAR, GPU_NEAREST);
        C3D_TexSetWrap(&tex, GPU_REPEAT, GPU_REPEAT); 

        if (args) {
            dae_default_material_args m_args = {0};
            // mat = m_args.mat;  // not yet used while i figure out the model converter stuff
            fread(&m_args, sizeof(dae_default_material_args), 1, args);
            freadstr(args, texpath, 255);
            std::string path = texpath;
            if (!loadTextureFromFile(&tex, NULL, ("romfs:/gfx/" + path + ".t3x").c_str(), true)) // should load into vram which is potentially an issue for space reasons? 
                Console::warn("unable to load texture");
            Console::warn("Tried to load %s", ("romfs:/gfx/" + path + ".t3x").c_str());
            C3D_TexSetFilter(&tex, GPU_LINEAR, GPU_NEAREST);
            C3D_TexSetWrap(&tex, GPU_REPEAT, GPU_REPEAT);
        }
        
    }

    ~dae_default_material() {
        shaderProgramFree(&program);
        DVLB_Free(vshader_dvlb);
    }

    void setMaterial(C3D_Mtx* modelview, C3D_Mtx* projection) override {
        C3D_BindProgram(&program);
        C3D_TexBind(0, &tex);
        C3D_LightEnvMaterial(&lights::lightenv, &mat); // give the lighting stuff the settings for this object
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR); // multiply by diffuse lighting and add specular lighting
        C3D_TexEnvFunc(env, C3D_Both, GPU_MULTIPLY_ADD);
        

        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  modelview);

        C3D_CullFace(GPU_CULL_NONE);
    }

    void resetMaterial() override {
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
    }
};

MATERIAL_REGISTER(dae_default_material)