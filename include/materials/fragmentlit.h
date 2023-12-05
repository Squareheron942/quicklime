#pragma once
#include "material.h"
#include "fragvshader_shbin.h"
#include "3ds.h"
#include "materialmanager.h"
#include "lights.h"

namespace {
    struct fragment_lit_args {
        const C3D_Material mat;
        // const char* texpath; // can't have strings in here since variable length, need to parse them separately
    };
}


class fragment_lit : public material {
    public:
    C3D_Tex tex;
    C3D_Material mat;
    DVLB_s *vshader_dvlb;
	shaderProgram_s program;
    int uLoc_projection, uLoc_modelView;
	C3D_LightLut lut_Light;
    fragment_lit(FILE* args) noexcept {
        mat = { // default settings that work mostly fine for anything
            { 0.1f, 0.1f, 0.1f }, //ambient
            { 0.4f, 0.4f, 0.4f }, //diffuse
            { 0.8f, 0.8f, 0.8f }, //specular0
            { 0.0f, 0.0f, 0.0f }, //specular1
            { 0.0f, 0.0f, 0.0f }, //emission
        };
        char texpath[255];
        vshader_dvlb = DVLB_ParseFile((u32*)fragvshader_shbin, fragvshader_shbin_size);
        shaderProgramInit(&program);
        shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
        uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
        uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
        

        LightLut_Phong(&lut_Light, 300);
        C3D_LightEnvLut(&lightenv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lut_Light);

        if (!loadTextureFromFile(&tex, NULL, "romfs:/gfx/kitten.t3x", true)) // also a placeholder 
            Console::warn("unable to load texture");
        C3D_TexSetFilter(&tex, GPU_LINEAR, GPU_NEAREST);
        C3D_TexSetWrap(&tex, GPU_REPEAT, GPU_REPEAT); 

        if (args) {
            fragment_lit_args m_args = {0};
            // mat = m_args.mat;  // not yet used while i figure out the model converter stuff
            fread(&m_args, sizeof(fragment_lit_args), 1, args);
            fgets(texpath, 255, args);
            std::string path = texpath;
            if (!loadTextureFromFile(&tex, NULL, ("romfs:/gfx/" + path + ".t3x").c_str(), true)) // also a placeholder 
                Console::warn("unable to load texture");
            C3D_TexSetFilter(&tex, GPU_LINEAR, GPU_NEAREST);
            C3D_TexSetWrap(&tex, GPU_REPEAT, GPU_REPEAT); 
        }
        
    }

    ~fragment_lit() {
        shaderProgramFree(&program);
        DVLB_Free(vshader_dvlb);
    }

    void setMaterial(C3D_Mtx* modelview, C3D_Mtx* projection) override {
        C3D_BindProgram(&program);
        C3D_TexBind(0, &tex);
        C3D_LightEnvMaterial(&lightenv, &mat); // give the lighting stuff the settings for this object
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR); // multiply by diffuse lighting and add specular lighting
        C3D_TexEnvFunc(env, C3D_Both, GPU_MULTIPLY_ADD);
        

        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  modelview);
    }

    void resetMaterial() override {
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
    }
};

MATERIAL_REGISTER(fragment_lit)