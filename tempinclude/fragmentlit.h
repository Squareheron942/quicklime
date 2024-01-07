#pragma once
#include "material.h"
#include "fragvshader_shbin.h"
#include "3ds.h"
#include "materialmanager.h"
#include "lights.h"
#include "texture.h"
#include <memory>


class fragment_lit : public material {
    public:
    std::shared_ptr<Texture> tex;
    C3D_Material mat;
    DVLB_s *vshader_dvlb;
	shaderProgram_s program;
    int uLoc_projection, uLoc_modelView;
	C3D_LightLut lut_Light;
    fragment_lit(FILE* args) noexcept;

    ~fragment_lit();

    void setMaterial(C3D_Mtx* modelview, C3D_Mtx* projection) override;

    void resetMaterial() override;
};