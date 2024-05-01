#include "camera.h"
#include "c3d/maths.h"
#include "gameobject.h"
#include "entt.hpp"
#include "transform.h"
#include "config.h"
#include "componentmanager.h"
#include "renderer.h"
#include "meshrenderer.h"
#include "lights.h"
#include "light.h"
#include "renderertypes.h"
#include "ui_text.h"
#include "citro2d.h"
#include "stats.h"

Camera* Camera::mainTop = NULL;
Camera* Camera::mainBottom = NULL;

namespace {
    /**
     * @brief The default used is just iod/5 (probably to reduce just how much it splits apart which causes headaches).
     * Having a map function allows the user to basically do whatever they want though.
     * You can have it completely ignore the input and use your own variable, make it exponential, log, alternate between 0 and 100 etc
     * @param iod The inputted slider value
    */
    float defaultIODMap(float iod) { return iod * 0.2f; }

    struct cam_args {
        // general camera properties

        bool wide = true; // whether or not to use the 240x800 mode on supported models // @0
        bool ortho = false; // @1
        unsigned short cull = 0xFFFF; // shows everything // @2

        RenderType type = RENDER_TYPE_TOPSCREEN; // what to render to // @4

        float nearClip = 0.1f, farClip = 1000.f; // @8, @12

        unsigned int bgcolor = 0x3477ebFF; // defaults to dark blue // @16

        // texture render properties
        unsigned short resolution; // min 8x8, max 1024x1024 (must be square) @20

        // ortho camera properties
        float height = 24.f; // @24

        float width = 40.f; // @28

        // perspective camera properties
        bool stereo = true; // whether to use 3D // @32
        float fovY = 55.f; // default for splatoon human form // @36
        /**
         * @brief Dictates how to map the iod input from the slider to the rendering iod
         * @param iod The inputted slider value
         */
        float(*iodMapFunc)(float) = NULL; // @40


    };
}

Camera::~Camera() {
	Console::log("Camera destructor");
}

Camera::Camera(GameObject& parent, const void* args) {
	LightLock_Init(&lock);
	LightLock_Lock(&lock);
    cam_args c;
    if (args)
        c = *(cam_args*)args;

    // set standard args
    nearClip = c.nearClip;
    farClip = c.farClip;
    orthographic = c.ortho;
    type = c.type;
    cullingMask = c.cull;
    // cullingMask = ~0;
    bgcolor = c.bgcolor;

    if (c.ortho) {
        fovY = 0.f;
        height = c.height;
        width = c.width;
    } else { // perspective
        fovY = c.fovY;
    }
    Console::log("nc%1.1f fc%.0f", c.nearClip, c.farClip);
    Console::log("o%u t%u", c.ortho, c.type);
    Console::log("c%p b%p", c.cull, c.bgcolor);
    Console::log("f%.1f h%.0f w%.0f", c.fovY, c.height, c.width);

    switch (c.type) {
        case RENDER_TYPE_TOPSCREEN:
            if (c.iodMapFunc) iodMapFunc = c.iodMapFunc;
            else iodMapFunc = defaultIODMap;
            highRes = c.wide && !config::wideIsUnsupported; // disable if not supported
            if (config::wideIsUnsupported) Console::warn("wide mode not supported");
            stereo = c.stereo && !c.ortho;
            target[0] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8); // only type to be used for display
            if (!target[0]) Console::warn("Could not create render target");
            if (stereo) target[1] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8); // only type to be used for display
            if (highRes) target[2] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8); // only type to be used for display
            aspectRatio = C3D_AspectRatioTop;
            break;
        case RENDER_TYPE_BOTTOMSCREEN:
            aspectRatio = C3D_AspectRatioBot;
            stereo = false;
            highRes = false;
            target[0] = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
            break;
        case RENDER_TYPE_TEXTURE:
            aspectRatio = 1.f;
            stereo = false;
            highRes = false;
            //TODO I still need to figure out render textures
            break;
        default:
            Console::warn("Invalid render type");
    }

    this->parent = &parent;
    // Camera::mainTop = this;
    LightLock_Unlock(&lock);
}

void transformobjs_r(GameObject* root, C3D_FVec topN, C3D_FVec botN, C3D_FVec leftN, C3D_FVec rightN, C3D_FVec nearN, C3D_FVec farN) {

}

void Camera::Render() {

    culledList.clear(); // remove all the old objects without deallocating the space since 99% of the time it won't need to change the space

    stats::_drawcalls = 0;

    // camera setup

    C3D_Mtx projection;
    Mtx_Identity(&projection);
    float iod = 0;
    if (stereo && !orthographic) iod = iodMapFunc(osGet3DSliderState()); // only calculate if actually necessary
    bool useWide = highRes && !(iod > 0.f);

    switch (type) {
        case RENDER_TYPE_TOPSCREEN:
            if (useWide) {
                C3D_RenderTargetSetOutput(target[2], GFX_TOP, GFX_LEFT, CAM_DISPLAY_TRANSFER_FLAGS);
                C3D_RenderTargetClear(target[2], C3D_CLEAR_ALL, bgcolor, 0);
                C3D_FrameDrawOn(target[2]);
            } else {
                C3D_RenderTargetSetOutput(target[0], GFX_TOP, GFX_LEFT, CAM_DISPLAY_TRANSFER_FLAGS);
                C3D_RenderTargetClear(target[0], C3D_CLEAR_ALL, bgcolor, 0);
                C3D_FrameDrawOn(target[0]);
            }
            break;
        case RENDER_TYPE_BOTTOMSCREEN:
            C3D_RenderTargetSetOutput(target[0], GFX_BOTTOM, GFX_LEFT, CAM_DISPLAY_TRANSFER_FLAGS);
            C3D_RenderTargetClear(target[0], C3D_CLEAR_ALL, bgcolor, 0);
            C3D_FrameDrawOn(target[0]);
            break;
        case RENDER_TYPE_TEXTURE:
            // TODO add handling for this
            break;
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


    // gfxSetWide(useWide); // Enable wide mode if wanted and if not rendering in stereo

    // scene setup

    transform* trans = parent->getComponent<transform>();

    if (!trans) return; // if no transform, the scene can't be rendered. there should always be a transform but the check is here just in case

    C3D_Mtx view = *trans;
    Mtx_Inverse(&view);

    // lighting update

    for (int i = 0; i < HW_MAX_LIGHTS; ++i)
        if (lights::active[i]) lights::active[i]->update(&view);


    osTickCounterStart(&stats::profiling::cnt_cull);

    C3D_Mtx vp;

    Mtx_Multiply(&vp, &projection, &view); // create view projection matrix, much faster since it saves a ton of matrix multiplications and is also useful for frustum culling

    // calculate frustum normals

    // C3D_FVec pos = trans->position;
    // C3D_FVec topN, botN, leftN, rightN, nearN, farN;

    // farN.x = -vp.r[0].c[2] + vp.r[0].c[3];
    // farN.y = -vp.r[1].c[2] + vp.r[1].c[3];
    // farN.z = -vp.r[2].c[2] + vp.r[2].c[3];


    // botN.x = vp.r[0].c[1] + vp.r[0].c[3];
    // botN.y = vp.r[1].c[1] + vp.r[1].c[3];
    // botN.z = vp.r[2].c[1] + vp.r[2].c[3];

    // topN.x = -vp.r[0].c[1] + vp.r[0].c[3];
    // topN.y = -vp.r[1].c[1] + vp.r[1].c[3];
    // topN.z = -vp.r[2].c[1] + vp.r[2].c[3];


    // leftN.x = vp.r[0].c[0] + vp.r[0].c[3];
    // leftN.y = vp.r[1].c[0] + vp.r[1].c[3];
    // leftN.z = vp.r[2].c[0] + vp.r[2].c[3];

    // rightN.x = -vp.r[0].c[0] + vp.r[0].c[3];
    // rightN.y = -vp.r[1].c[0] + vp.r[1].c[3];
    // rightN.z = -vp.r[2].c[0] + vp.r[2].c[3];

    // // normalize frustum normals
    // float invnearM = 1/sqrtf(nearN.x*nearN.x + nearN.y*nearN.y + nearN.z*nearN.z);
    // float invfarM = 1/sqrtf(farN.x*farN.x + farN.y*farN.y + farN.z*farN.z);
    // float invtopM = 1/sqrtf(topN.x*topN.x + topN.y*topN.y + topN.z*topN.z);
    // float invbotM = 1/sqrtf(botN.x*botN.x + botN.y*botN.y + botN.z*botN.z);
    // float invleftM = 1/sqrtf(leftN.x*leftN.x + leftN.y*leftN.y + leftN.z*leftN.z);
    // float invrightM = 1/sqrtf(rightN.x*rightN.x + rightN.y*rightN.y + rightN.z*rightN.z);

    // nearN.x *= invnearM;
    // nearN.y *= invnearM;
    // nearN.z *= invnearM;

    // farN.x *= invfarM;
    // farN.y *= invfarM;
    // farN.z *= invfarM;

    // topN.x *= invtopM;
    // topN.y *= invtopM;
    // topN.z *= invtopM;

    // botN.x *= invbotM;
    // botN.y *= invbotM;
    // botN.z *= invbotM;

    // leftN.x *= invleftM;
    // leftN.y *= invleftM;
    // leftN.z *= invleftM;

    // rightN.x *= invrightM;
    // rightN.y *= invrightM;
    // rightN.z *= invrightM;


    // culling prepass
    for (GameObject* obj : sceneRoot->children) {
        // mesh* m = NULL;
        if (!obj) continue; // skip null objects, there shouldn't be any so I think it can be removed, and I can't remove it from the list at this step so it might end up being kinda slow
        if (!(obj->layer & cullingMask)) continue; // skip culled objects
        if (!obj->renderer) continue; // skip objects with no set renderer
        // if (!(m = obj->getComponent<mesh>())) continue; // skip objects with no mesh
        // if (!obj->getComponent<MeshRenderer>()) continue; // skip objects with no renderer

        // transform* t = obj->getComponent<transform>();
        // C3D_FVec p = FVec3_Subtract(pos, t->position);

        // bool left = -m->radius < FVec3_Dot(leftN, p);
        // if (!left) continue;
        // bool right = -m->radius < FVec3_Dot(rightN, p);
        // if (!right) continue;
        // bool top = -m->radius < FVec3_Dot(topN, p);
        // if (!top) continue;
        // bool bot = -m->radius < FVec3_Dot(botN, p);
        // if (!bot) continue;
        // bool back = -m->radius < FVec3_Dot(farN, p);
        // if (!back) continue;
        // bool front = -m->radius < FVec3_Dot(nearN, p);
        // if (!front) continue;

        // if (!(left + right + back + front + top + bot)) continue; // if not in any then skip it (this means it is outside the frustum)
        culledList.push_back(obj);
    }
    osTickCounterUpdate(&stats::profiling::cnt_cull);
    stats::profiling::rnd_cull = osTickCounterRead(&stats::profiling::cnt_cull);

    // actually render stuff for left eye
    osTickCounterStart(&stats::profiling::cnt_meshrnd);
    // render objects
    for (GameObject* obj : culledList) {
        if (obj->renderer & RENDERER_MESH) obj->getComponent<MeshRenderer>()->render(view, projection);
        if (obj->renderer & RENDERER_TEXT) {
	        C2D_SceneTarget(target[0]); //TODO defer text/ui rendering
	        obj->getComponent<Text>()->Render();
        }
    }
    osTickCounterUpdate(&stats::profiling::cnt_meshrnd);
    stats::profiling::rnd_meshrnd = osTickCounterRead(&stats::profiling::cnt_meshrnd);


    if (!(stereo && iod > 0.0f)) return; // stop after first eye is drawn unless 3d is enabled

    // render right eye

    gfxSet3D(true);
    C3D_RenderTargetSetOutput(target[1], GFX_TOP, GFX_RIGHT, CAM_DISPLAY_TRANSFER_FLAGS);
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

    osTickCounterStart(&stats::profiling::cnt_meshrnd);
    // render objects
    // C2D_SceneTarget(target[1]);
    for (GameObject* obj : culledList) {
        if (obj->renderer & RENDERER_MESH) obj->getComponent<MeshRenderer>()->render(view, projection);
        if (obj->renderer & RENDERER_TEXT) {
        	C2D_SceneTarget(target[1]);
        	obj->getComponent<Text>()->Render();
        }
    }
    osTickCounterUpdate(&stats::profiling::cnt_meshrnd);
    stats::profiling::rnd_meshrnd += osTickCounterRead(&stats::profiling::cnt_meshrnd);

}

COMPONENT_REGISTER(Camera)