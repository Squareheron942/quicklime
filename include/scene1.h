#pragma once

#include "scene.h"
#include "vertex.h"
#include <3ds.h>
#include <citro3d.h>
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
	// all this is from the hardcoded rendering I have rn, will need to be removed
	GameObject script1object, moveobject;
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

	C3D_LightEnv lightEnv;
	C3D_Light light;
	C3D_LightLut lut_Light;
};