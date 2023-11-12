#pragma once
#include "material.h"
#include "fragvshader_shbin.h"
#include "3ds.h"

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
        if (args) {
            fragment_lit_args m_args = {0};
            fread(&m_args, sizeof(fragment_lit_args), 1, args);
            char texpath[255];
            fgets(texpath, 255, args);
            loadTextureFromFile(&tex, NULL, texpath, false);
            mat = m_args.mat;
            vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
            shaderProgramInit(&program);
            shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
            uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
            uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
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

        shaderProgramConfigure(&program, true, false);
        C3D_BindProgram(&program);

        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  modelview);
    }

    void resetMaterial() override {
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
    }
};