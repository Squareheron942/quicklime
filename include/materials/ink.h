#pragma once
#include "material.h"
#include "fragvshader_shbin.h"
#include "3ds.h"
#include "materialmanager.h"
#include "lights.h"

namespace {
    struct ink_args {
        const C3D_Mtx ink1mat, ink2mat;
        uint8_t alpha;
        float phase, frequency, amplitude, specular;
        unsigned int team1color, team2color;
        // can only include fixed size variables, strings need to be parsed separately
        // const char* tex1path; // if we want to have a special texture to overlay onto the ink (decals etc)
        // const char* tex2path;
        // const char* mask1path; // ink mask (dictating how the ink is on the ground)
        // const char* mask2path;
    };
}

#ifndef INK_NUM_COLORS
#define INK_NUM_COLORS 4
#endif


class ink : public material {
    public:
    C3D_LightLut lut_Phong;
    C3D_Tex team1tex, team2tex, team1mask, team2mask;
    C3D_Mtx ink1mat, ink2mat; // separate so that each ink can have a different specular color etc
    DVLB_s *vshader_dvlb;
	shaderProgram_s program;
    C3D_ProcTex pt;
    C3D_ProcTexLut pt_map;
    C3D_ProcTexLut pt_noise;
    C3D_ProcTexColorLut pt_clr;
    bool ink1hastex, ink2hastex;
    int uLoc_projection, uLoc_modelView;
    float data[129], specular;
    unsigned int team1color, team2color;
    ink(FILE* args) noexcept {
        if (args) {
            ink_args m_args = {0};
            fread(&m_args, sizeof(ink_args), 1, args);
            char tex1path[255], tex2path[255], mask1path[255], mask2path[255];
            
            fgets(tex1path, 255, args);
            fgets(tex2path, 255, args);


            // load textures
            // ink1hastex = loadTextureFromFile(&team1tex, NULL, tex1path, false);
            // ink2hastex = loadTextureFromFile(&team2tex, NULL, tex2path, false);
            // loadTextureFromFile(&team1mask, NULL, mask1path, false);
            // loadTextureFromFile(&team2mask, NULL, mask2path, false);

            C3D_TexSetFilter(&team1tex, GPU_LINEAR, GPU_NEAREST);
            C3D_TexSetFilter(&team2tex, GPU_LINEAR, GPU_NEAREST);

            ink1mat = m_args.ink1mat;
            ink2mat = m_args.ink2mat;
            vshader_dvlb = DVLB_ParseFile((u32*)fragvshader_shbin, fragvshader_shbin_size);
            shaderProgramInit(&program);
            shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
            shaderProgramConfigure(&program, true, false);
            uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
            uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

            int i;
            // Noise smooth step equation
            for (i = 0; i <= 128; i ++)
            {
                float x = i/128.0f;
                data[i] = x*x*(3-2*x); // removes the weird line artifacts from linear mapping
                // data[i] = x;
            }

            ProcTexLut_FromArray(&pt_noise, data);
            ProcTexLut_FromArray(&pt_map, data);

            C3D_ProcTexInit(&pt, 0, INK_NUM_COLORS);
            C3D_ProcTexClamp(&pt, GPU_PT_MIRRORED_REPEAT, GPU_PT_MIRRORED_REPEAT);
            C3D_ProcTexNoiseCoefs(&pt, C3D_ProcTex_UV, m_args.amplitude, m_args.frequency, m_args.phase);
            C3D_ProcTexFilter(&pt, GPU_PT_LINEAR);
        }
        
    }

    ~ink() {
        // delete anything allocated with new here
    }

    void setMaterial(C3D_Mtx* modelview, C3D_Mtx* projection) override {
        // set up vertex shader
        C3D_BindProgram(&program);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  modelview);

        // bind proctex values
        C3D_ProcTexLutBind(GPU_LUT_NOISE, &pt_noise);
        C3D_ProcTexLutBind(GPU_LUT_RGBMAP, &pt_map);
        C3D_ProcTexBind(0, &pt);
        u32 colors[INK_NUM_COLORS];
        // colors[0] = 0x00FF8080;
        // colors[1] = 0x2080FF80;
        // colors[2] = 0x408080FF;
        // colors[3] = 0x80808080;
        colors[0] = 0x00ff8080;
        colors[1] = 0x20f64280;
        colors[2] = 0x40f68042; // normals angled at approx 15º from vertical
        colors[3] = 0x80ff8080; // have 2 up vectors to make it more bright

        ProcTexColorLut_Write(&pt_clr, colors, 0, INK_NUM_COLORS);
        C3D_ProcTexColorLutBind(&pt_clr);

        // set alpha test cutoff for sharp edges
        // C3D_AlphaTest(true, GPU_GREATER, alpha);

        C3D_TexBind(0, &team1mask);
        C3D_TexBind(1, &team1tex);

        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, GPU_TEXTURE3, (GPU_TEVSRC)0);
        C3D_TexEnvFunc(env, C3D_Alpha, GPU_SUBTRACT);
        
        if (ink1hastex) {
            env = C3D_GetTexEnv(1);
            C3D_TexEnvInit(env);
            C3D_TexEnvSrc(env, C3D_RGB, GPU_PREVIOUS, GPU_TEXTURE1, (GPU_TEVSRC)0);
            C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
        } else {
            env = C3D_GetTexEnv(1);
            C3D_TexEnvInit(env);
            C3D_TexEnvColor(env, team1color);
            C3D_TexEnvSrc(env, C3D_RGB, GPU_PREVIOUS, GPU_CONSTANT, (GPU_TEVSRC)0);
            C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
        }

        env = C3D_GetTexEnv(3);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_PREVIOUS, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR);
        C3D_TexEnvFunc(env, C3D_RGB, GPU_MULTIPLY_ADD);

        C3D_LightEnvBumpNormalZ(&lights::lightenv, true);
        C3D_LightEnvBumpMode(&lights::lightenv, GPU_BUMP_AS_BUMP);
        C3D_LightEnvBumpSel(&lights::lightenv, 3);

        u32 colors2[INK_NUM_COLORS];
        colors2[3] = colors[0];
        colors2[2] = colors[1];
        colors2[1] = colors[2];
        colors2[0] = colors[3];

        ProcTexColorLut_Write(&pt_clr, colors2, 0, INK_NUM_COLORS);
        C3D_ProcTexColorLutBind(&pt_clr);

        // INSERT THE DRAWING PART HERE
        // OR ELSE DO SOME 2 PASS GOOFINESS
        // EITHER ONE I GUESS

        C3D_TexBind(0, &team2mask);
        C3D_TexBind(1, &team2tex);

        env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, GPU_TEXTURE3, (GPU_TEVSRC)0);
        C3D_TexEnvFunc(env, C3D_Alpha, GPU_SUBTRACT);
        
        if (ink1hastex) {
            env = C3D_GetTexEnv(1);
            C3D_TexEnvInit(env);
            C3D_TexEnvSrc(env, C3D_RGB, GPU_PREVIOUS, GPU_TEXTURE1, (GPU_TEVSRC)0);
            C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
        } else {
            env = C3D_GetTexEnv(1);
            C3D_TexEnvInit(env);
            C3D_TexEnvColor(env, team2color);
            C3D_TexEnvSrc(env, C3D_RGB, GPU_PREVIOUS, GPU_CONSTANT, (GPU_TEVSRC)0);
            C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
        }

        env = C3D_GetTexEnv(3);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_PREVIOUS, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR);
        C3D_TexEnvFunc(env, C3D_RGB, GPU_MULTIPLY_ADD);

        C3D_LightEnvBumpNormalZ(&lights::lightenv, true);
        C3D_LightEnvBumpMode(&lights::lightenv, GPU_BUMP_AS_BUMP);
        C3D_LightEnvBumpSel(&lights::lightenv, 3);
    }

    void resetMaterial() override {
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        env = C3D_GetTexEnv(1);
        C3D_TexEnvInit(env);
        env = C3D_GetTexEnv(2);
        C3D_TexEnvInit(env);
        env = C3D_GetTexEnv(3);
        C3D_TexEnvInit(env);

        C3D_LightEnvBumpNormalZ(&lights::lightenv, false);
        C3D_LightEnvBumpMode(&lights::lightenv, GPU_BUMP_NOT_USED);
        C3D_LightEnvBumpSel(&lights::lightenv, 0);
    }
};

MATERIAL_REGISTER(ink)