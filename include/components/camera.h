#pragma once

#include <citro3d.h>
#include <citro2d.h>
#include <vector>

class GameObject;

#define CAM_DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

enum RenderType {
    RENDER_TYPE_TOPSCREEN = 0,
    RENDER_TYPE_BOTTOMSCREEN = 1,
    RENDER_TYPE_TEXTURE = 2
};

// typedef struct {GameObject* obj; C3D_Mtx view;} icam_obj;

class Camera {
    // void transformobjs_r(GameObject* obj, C3D_Mtx& parentmodelmtx, C3D_FVec pos, C3D_FVec topN, C3D_FVec botN, C3D_FVec leftN, C3D_FVec rightN, C3D_FVec nearN, C3D_FVec farN);

public:
    GameObject* parent;
    float nearClip, farClip, focalLength = 2.f, fovY, height, width;
    /**
     * @brief Dictates how to map the iod input from the slider to the rendering iod
     * @param iod The inputted slider value
     */
    float(*iodMapFunc)(float);
    bool stereo, orthographic, highRes;
    unsigned int bgcolor;
    RenderType type;
    C3D_RenderTarget* target[3] = {NULL, NULL, NULL}; // 2 targets in case of stereoscopic view, plus one for if wide view is also enabled (since it's a different resolution)
    static Camera *mainTop, *mainBottom; // main cameras for top and bottom screen respectively
    GameObject* sceneRoot;
    std::vector<GameObject*> culledList;
    float aspectRatio;
    unsigned short cullingMask; // sees all 16 layers by default
    Camera(GameObject& parent, const void* args);
    
    void Render();
};