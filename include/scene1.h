#pragma once

#include "scene.h"
#include "vertex.h"
#include <3ds.h>
#include "object3d.h"
#include "entt.hpp"
#include "fast_obj.h"
#include "gameobject.h"

class Scene1 : public Scene {
    public: 
    Scene1();
    ~Scene1() override;
    void update() override;
    void drawTop(float iod) override;
    void drawBottom() override;
	entt::registry objects;
	GameObject root;

	// all this is from the hardcoded rendering I have rn, will need to be removed
	GameObject script1object;
    C3D_Mtx material =
    {
        {
        { { 0.0f, 0.2f, 0.2f, 0.2f } }, // Ambient
        { { 0.0f, 0.4f, 0.4f, 0.4f } }, // Diffuse
        { { 0.0f, 0.0f, 0.0f, 0.0f } }, // Specular
        { { 1.0f, 0.0f, 0.0f, 0.0f } }, // Emission
        }
    };

	const C3D_Material lmat =
	{
		{ 0.1f, 0.1f, 0.1f }, //ambient
		{ 0.4f, 0.4f, 0.4f }, //diffuse
		{ 0.8f, 0.8f, 0.8f }, //specular0
		{ 0.0f, 0.0f, 0.0f }, //specular1
		{ 0.0f, 0.0f, 0.0f }, //emission
	};

    int uLoc_projection, uLoc_modelView;
    int uLoc_lightVec, uLoc_lightHalfVec, uLoc_lightClr, uLoc_material, texcoord_offsets;
    C3D_Mtx projection;
    fastObjMesh* mesh;
    DVLB_s* vshader_dvlb;
    shaderProgram_s program;
	
    C3D_TexEnv* env;
    void* vbo_data;
    void** vbo_data2;
    C3D_Tex bottom_tex, top_tex;
    C3D_BufInfo bufCube, bufCube2, *bufPlaza;
    float offsetX = 0.0, offsetY = 0.0, angleX = 0.0, distance = 0.0, offset2 = 0.0;
    unsigned char angleY = 0;
    C3D_FVec camPos;
	vertex** meshes = nullptr;
	C3D_LightEnv lightEnv;
	C3D_Light light;
	C3D_LightLut lut_Light;
};