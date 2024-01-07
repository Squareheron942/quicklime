#pragma once

#include <citro3d.h>
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

class Camera {
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
    std::vector<GameObject*>* objects;
    float aspectRatio;
    unsigned short cullingMask; // sees all 16 layers by default
    Camera(GameObject& parent, const void* args);
    
    void Render();
};