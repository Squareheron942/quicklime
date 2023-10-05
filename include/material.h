#pragma once

#include "color.h"
#include "citro3d.h"
#include "vshader_shbin.h"
#include "tex3ds.h"
#include "waterwave_t3x.h"

class material {
    C3D_Tex bottom_tex, top_tex;
    public:
    color ambient = {1, 1, 1}, diffuse = {1, 1, 1}, specular = {1, 1, 1}, transmission = {1, 1, 1};
    float specexp = 100, alpha = 1;
    char *albedoPath = nullptr, *specularPath = nullptr, *specexpPath = nullptr, *decalPath = nullptr;
    DVLB_s *vshader_dvlb;
	shaderProgram_s program;
    int uLoc_projection, uLoc_modelView;
    int uLoc_lightVec, uLoc_lightHalfVec, uLoc_lightClr, uLoc_material, texcoord_offsets;
    C3D_Mtx projection;
    C3D_Mtx mat;

    bool loadTextureFromMem(C3D_Tex* tex, C3D_TexCube* cube, const void* data, size_t size)
    {
        Tex3DS_Texture t3x = Tex3DS_TextureImport(data, size, tex, cube, false);
        if (!t3x)
            return false;

        // Delete the t3x object since we don't need it
        Tex3DS_TextureFree(t3x);
        return true;
    }

    material() {
        vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
		shaderProgramInit(&program);
		shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
        if (!loadTextureFromMem(&bottom_tex, NULL, waterwave_t3x, waterwave_t3x_size))
            svcBreak(USERBREAK_PANIC);
        C3D_TexSetFilter(&bottom_tex, GPU_LINEAR, GPU_NEAREST);
        C3D_TexSetWrap(&bottom_tex, GPU_REPEAT, GPU_REPEAT);
        top_tex = bottom_tex;
        printf("material created\n");
    };
    ~material() {
        // Free the shader program
        printf("material destroyed\n");
		shaderProgramFree(&program);
		DVLB_Free(vshader_dvlb);
    }
    void setMaterial(C3D_Mtx *modelView) {
        C3D_TexBind(0, &bottom_tex);
        C3D_TexBind(1, &bottom_tex);

        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_Both, GPU_ADD_MULTIPLY);
        env = C3D_GetTexEnv(1);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_PREVIOUS, GPU_FRAGMENT_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_Both, GPU_ADD);
        shaderProgramConfigure(&program, true, false);
        C3D_BindProgram(&program);
        uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
        uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
        uLoc_lightVec     = shaderInstanceGetUniformLocation(program.vertexShader, "lightVec");
        uLoc_lightHalfVec = shaderInstanceGetUniformLocation(program.vertexShader, "lightHalfVec");
        uLoc_lightClr     = shaderInstanceGetUniformLocation(program.vertexShader, "lightClr");
        uLoc_material     = shaderInstanceGetUniformLocation(program.vertexShader, "material");
        // texcoord_offsets  = shaderInstanceGetUniformLocation(program.vertexShader, "texcoordoffsets");

        // Update the uniforms
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  modelView);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_material,   &mat);
        C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightVec,     0.0f, 0.0f, -1.0f, 0.0f);
        C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightHalfVec, 0.0f, 0.0f, -1.0f, 0.0f);
        C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightClr,     1.0f, 1.0f,  1.0f, 1.0f);
        // C3D_FVUnifSet(GPU_VERTEX_SHADER, texcoord_offsets,     offsetX * 0.1f, offsetY * 0.1f,  offset2 * 0.1f, 1.0f);
    }
};