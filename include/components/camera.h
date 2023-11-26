#pragma once

#include "componentmanager.h"
#include <citro3d.h>
#include <forward_list>
#include "config.h"

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

namespace {
    /** 
     * The default used is just iod/5 (probably to reduce just how much it splits apart which causes headaches) 
     * Having a map function allows the user to basically do whatever they want though
     * You can have it completely ignore the input and use your own variable, make it exponential, log, alternate between 0 and 100 etc
    */
    float defaultIODMap(float iod) { return iod * 0.2f; }

    struct cam_args {
        // general camera properties
        bool wide = false; // whether or not to use the 240x800 mode on supported models
        RenderType type = RENDER_TYPE_TOPSCREEN; // what to render to
        float nearClip = 0.1f, farClip = 1000.f;
        bool ortho = false;
        unsigned int bgcolor = 0x800000; // defaults to half blue


        // ortho camera properties
        float height = 24.f;
        float width = 40.f;

        // perspective camera properties
        bool stereo = false; // whether to use 3D
        float fovY = 55.f; // default for splatoon human form
        float(*iodMapFunc)(float) = NULL; // mapping function to convert slider value to iod

        // texture render properties
        unsigned short resolution; // min 8x8, max 1024x1024 (must be square)
    };
}

class Camera {
    GameObject* parent;
    float nearClip = 0.f, farClip = 1000.f, focalLength = 2.f, fovY = 80.f, height = 24.0f, width = 40.0f;
    float(*iodMapFunc)(float); // dictates how to map the iod input from the slider to the rendering iod
    bool stereo = false, orthographic = false, highRes = false;
    unsigned int bgcolor;
    RenderType type;
    C3D_Mtx projection;
    C3D_RenderTarget* target[3] = {NULL, NULL, NULL}; // 2 targets in case of stereoscopic view, plus one for if wide view is also enabled (since it's a different resolution)
    static Camera *mainTop, *mainBottom; // main cameras for top and bottom screen respectively
    static std::forward_list<GameObject&> topCameraObjects, bottomCameraObjects;
    float aspectRatio;

    Camera(GameObject& parent, void* args) {
        if (args) {
            cam_args c = *(cam_args*)args;
            // set standard args
            nearClip = c.nearClip;
            farClip = c.farClip;
            orthographic = c.ortho;
            type = c.type;

            if (c.ortho) {
                fovY = 0.f;
                height = c.height;
                width = c.width;
            } else { // perspective
                fovY = c.fovY;
                
            }
            switch (c.type) {
                case RENDER_TYPE_TOPSCREEN:
                    if (c.iodMapFunc) iodMapFunc = c.iodMapFunc;
                    else iodMapFunc = defaultIODMap;
                    highRes = c.wide && !config::wideIsUnsupported; // disable if not supported
                    stereo = c.stereo;
                    if (stereo) {
                        target[0] = C3D_RenderTargetCreate(400, 240, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8); // only type to be used for display
                        target[1] = C3D_RenderTargetCreate(400, 240, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8); // only type to be used for display
                        if (highRes) target[2] = C3D_RenderTargetCreate(800, 240, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8); // only type to be used for display
                    }
                case RENDER_TYPE_BOTTOMSCREEN:
                    aspectRatio = C3D_AspectRatioBot;
                    stereo = false;
                    highRes = false;
                    break;
                case RENDER_TYPE_TEXTURE:
                    aspectRatio = 1.f;
                    stereo = false;
                    highRes = false;
                    break;
            }
        }

        this->parent = &parent;
    }

    void Render() {
        
        float iod = 0;
        if (stereo && !orthographic) iodMapFunc(osGet3DSliderState()); // only calculate if actually necessary
        bool useWide = highRes && !(iod > 0.f);
        if (type == RENDER_TYPE_BOTTOMSCREEN) C3D_RenderTargetSetOutput(target[0], GFX_BOTTOM, GFX_LEFT, CAM_DISPLAY_TRANSFER_FLAGS);
        else if (type == RENDER_TYPE_TOPSCREEN) {
            if (useWide) C3D_RenderTargetSetOutput(target[2], GFX_BOTTOM, GFX_LEFT, CAM_DISPLAY_TRANSFER_FLAGS);
            else C3D_RenderTargetSetOutput(target[0], GFX_TOP, GFX_LEFT, CAM_DISPLAY_TRANSFER_FLAGS);
            if (stereo && !useWide) C3D_RenderTargetSetOutput(target[1], GFX_BOTTOM, GFX_RIGHT, CAM_DISPLAY_TRANSFER_FLAGS);
        }

        if (orthographic) Mtx_OrthoTilt(&projection, -width / 2, width / 2, -height / 2, height / 2, nearClip, farClip, false); // no perspective
        else if (stereo) // both perspective and 3D
            Mtx_PerspStereoTilt(
                &projection, 
                C3D_AngleFromDegrees(fovY), 
                aspectRatio, 
                nearClip, farClip, 
                -iod, focalLength, 
                false
            );
        else // perspective but no 3D
            Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(fovY), aspectRatio, nearClip, farClip, false);

        gfxSetWide(useWide); // Enable wide mode if wanted and if not rendering on both screens

        /**
         * Clear left (default) render target fully.
         * Could maybe add the option to select the 3 options (only color, only depth buffer) 
         * but I think it's fine since those options are barely ever used
         */
        C3D_RenderTargetClear(useWide ? target[2] : target[0], C3D_CLEAR_ALL, bgcolor, 0);
        C3D_FrameDrawOn(useWide ? target[2] : target[0]);

        // actually render stuff for left eye

        if (!orthographic && stereo && iod > 0.0f) {
            Mtx_PerspStereoTilt(
                &projection, 
                C3D_AngleFromDegrees(fovY), 
                C3D_AspectRatioTop, 
                nearClip, farClip, 
                iod, focalLength, 
                false
            );
            C3D_RenderTargetClear(target[1], C3D_CLEAR_ALL, bgcolor, 0);
            C3D_FrameDrawOn(target[1]);

            // actually render stuff here for 2nd eye

        }
    }
};