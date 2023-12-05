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

	C3D_Light light;
};