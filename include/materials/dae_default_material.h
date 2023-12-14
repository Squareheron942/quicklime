#pragma once
#include "material.h"
#include "fragvshader_dae_shbin.h" // for per-fragment lighting that is expecting dae models
#include "3ds.h"
#include "materialmanager.h"
#include "lights.h"
#include <memory>
#include "texture.h"

namespace {
    struct dae_default_material_args {
        float ambient[4], diffuse[4], specular0[4], specular1[4], emission[4];
        // const char* texpath; // can't have strings in here since variable length, need to parse them separately
    };
}


class dae_default_material : public material {
    public:
    std::shared_ptr<Texture> tex;
    C3D_Material mat;
    DVLB_s *vshader_dvlb;
	shaderProgram_s program;
    int uLoc_projection, uLoc_modelView;
	C3D_LightLut lut_Light;
    dae_default_material(FILE* args) noexcept {
        char texpath[255];
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
        
    }

    ~dae_default_material() {
        shaderProgramFree(&program);
        DVLB_Free(vshader_dvlb);
    }

    void setMaterial(C3D_Mtx* modelview, C3D_Mtx* projection) override {
        C3D_BindProgram(&program);
        C3D_TexBind(0, &tex->tex);
        C3D_LightEnvMaterial(&lights::lightenv, &mat); // give the lighting stuff the settings for this object
        C3D_LightEnvLut(&lights::lightenv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lut_Light); // set the shininess value
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