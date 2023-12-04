#pragma once
#include "material.h"
#include "fragvshader_shbin.h"
#include "3ds.h"
#include "materialmanager.h"

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
    fragment_lit(FILE* args) noexcept {
        mat = {
            { 0.1f, 0.1f, 0.1f }, //ambient
            { 0.4f, 0.4f, 0.4f }, //diffuse
            { 0.8f, 0.8f, 0.8f }, //specular0
            { 0.0f, 0.0f, 0.0f }, //specular1
            { 0.0f, 0.0f, 0.0f }, //emission
        };
        char texpath[255];
        vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
        shaderProgramInit(&program);
        shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
        C3D_BindProgram(&program);
        uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
        uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
        if (args) {
            fragment_lit_args m_args = {0};
            // mat = m_args.mat;  // not yet used while i figure out the model converter stuff
            fread(&m_args, sizeof(fragment_lit_args), 1, args);
            fgets(texpath, 255, args);
            if (!loadTextureFromFile(&tex, NULL, "romfs:/gfx/kitten.t3x", false)) // also a placeholder 
                Console::warn("unable to load texture");
        }
        
    }

    ~fragment_lit() {
        shaderProgramFree(&program);
        DVLB_Free(vshader_dvlb);
    }

    void setMaterial(C3D_Mtx* modelview, C3D_Mtx* projection) override {
        C3D_TexBind(0, &tex);
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR); // multiply by diffuse lighting and add specular lighting
        C3D_TexEnvFunc(env, C3D_Both, GPU_MULTIPLY_ADD);
        // C3D_BindProgram(&program);
        C3D_CullFace(GPU_CULL_NONE);


        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  modelview);
    }

    void resetMaterial() override {
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
    }
};

MATERIAL_REGISTER(fragment_lit)