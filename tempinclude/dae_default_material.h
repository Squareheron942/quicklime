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
        float ambient[4], diffuse[4], specular0[4], specular1[4], emission[4], transparent[4];
        int opaque = true; // needs to be an int for struct packing reasons
        // const char* texpath; // can't have strings in here since variable length, need to parse them separately
    };
}


class dae_default_material : public material {
    public:
    std::shared_ptr<Texture> tex;
    C3D_Material mat;
    bool opaque = true;
    DVLB_s *vshader_dvlb;
	shaderProgram_s program;
    int uLoc_projection, uLoc_modelView;
	C3D_LightLut lut_Light;
    dae_default_material(FILE* args) noexcept;

    ~dae_default_material();
    void setMaterial(C3D_Mtx* modelview, C3D_Mtx* projection) override;
    void resetMaterial() override;
}; 