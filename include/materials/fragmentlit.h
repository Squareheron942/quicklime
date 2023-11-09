#pragma once
#include "material.h"
#include "vshader_shbin.h"
#include "3ds.h"

namespace {
    struct fragment_lit_args {
        const char* texpath;
        const C3D_Mtx mat;
    };
}


class fragment_lit : public material {
    public:
    C3D_Tex tex;
    C3D_Mtx mat;
    DVLB_s *vshader_dvlb;
	shaderProgram_s program;
    int uLoc_projection, uLoc_modelView;
    int uLoc_lightVec, uLoc_lightHalfVec, uLoc_lightClr, uLoc_material, texcoord_offsets;
    fragment_lit(void* args) noexcept {
        if (args) {
            fragment_lit_args m_args = *(fragment_lit_args*)args;
            loadTextureFromFile(&tex, NULL, m_args.texpath, false);
            mat = m_args.mat;
            vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
            shaderProgramInit(&program);
            shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
            uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
            uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
            uLoc_lightVec     = shaderInstanceGetUniformLocation(program.vertexShader, "lightVec");
            uLoc_lightHalfVec = shaderInstanceGetUniformLocation(program.vertexShader, "lightHalfVec");
            uLoc_lightClr     = shaderInstanceGetUniformLocation(program.vertexShader, "lightClr");
            uLoc_material     = shaderInstanceGetUniformLocation(program.vertexShader, "material");
            texcoord_offsets  = shaderInstanceGetUniformLocation(program.vertexShader, "texcoordoffsets");
        }
        
    }

    ~fragment_lit() {}

    void setMaterial(C3D_Mtx* modelview, C3D_Mtx* projection) override {
        C3D_TexBind(0, &tex);
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR);
        C3D_TexEnvFunc(env, C3D_Both, GPU_ADD_MULTIPLY);

        shaderProgramConfigure(&program, true, false);
        C3D_BindProgram(&program);

        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  modelview);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_material,   &mat);
        C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightVec,     0.0f, 0.0f, -1.0f, 0.0f);
        C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightHalfVec, 0.0f, 0.0f, -1.0f, 0.0f);
        C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightClr,     1.0f, 1.0f,  1.0f, 1.0f);
    }

    void resetMaterial() override {
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
    }
};