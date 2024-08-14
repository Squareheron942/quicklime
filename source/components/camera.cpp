#include "camera.h"

#include <3ds.h>
#include <citro3d.h>

#include "citro2d.h"
#include "componentmanager.h"
#include "config.h"
#include "entt.hpp"
#include "gameobject.h"
#include "light.h"
#include "lights.h"
#include "renderer.h"
#include "scene.h"
#include "stats.h"
#include "threads.h"
#include "transform.h"

C3D_RenderTarget *Camera::target[4] = {
	C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8),
	C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8),
	C3D_RenderTargetCreate(240, 800, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8),
	C3D_RenderTargetCreate(
		240, 320, GPU_RB_RGBA8,
		GPU_RB_DEPTH24_STENCIL8)}; // 2 targets in case of stereoscopic view,
								   // plus one for if wide view is also enabled
								   // (since it's a different resolution)

namespace {
	/**
	 * @brief The default used is just iod/5 (probably to reduce just how much
	 * it splits apart which causes headaches). Having a map function allows the
	 * user to basically do whatever they want though. You can have it
	 * completely ignore the input and use your own variable, make it
	 * exponential, log, alternate between 0 and 100 etc
	 * @param iod The inputted slider value
	 */
	float defaultIODMap(float iod) { return iod * 0.2f; }

	struct cam_args {
		// general camera properties
		bool active = true;
		bool wide =
			true; // whether or not to use the 240x800 mode on supported models
		bool ortho			= false;
		unsigned short cull = 0xFFFF; // shows everything

		RenderType type		= RENDER_TYPE_TOPSCREEN; // what to render to

		float nearClip = 0.1f, farClip = 1000.f;

		unsigned int bgcolor = 0x3477ebFF; // defaults to dark blue

		// texture render properties
		unsigned short resolution; // min 8x8, max 1024x1024 (must be square)

		// ortho camera properties
		float height			   = 24.f;

		float width				   = 40.f;

		// perspective camera properties
		bool stereo				   = true; // whether to use 3D
		float fovY				   = 55.f; // default for splatoon human form
		/**
		 * @brief Dictates how to map the iod input from the slider to the
		 * rendering iod
		 * @param iod The inputted slider value
		 */
		float (*iodMapFunc)(float) = NULL;
	};
} // namespace

Camera::~Camera() {}

Camera::Camera(GameObject &parent, const void *args) {
	cam_args c;
	if (args)
		c = *(cam_args *)args;

	// set standard args
	nearClip	 = c.nearClip;
	farClip		 = c.farClip;
	orthographic = c.ortho;
	type		 = c.type;
	cullingMask	 = c.cull;
	// cullingMask = ~0;
	bgcolor		 = c.bgcolor;
	active		 = c.active;

	if (c.ortho) {
		fovY   = 0.f;
		height = c.height;
		width  = c.width;
	} else { // perspective
		fovY = c.fovY;
	}
	Console::log("nc%1.1f fc%.0f", c.nearClip, c.farClip);
	Console::log("o%u t%u", c.ortho, c.type);
	Console::log("c%p b%p", c.cull, c.bgcolor);
	Console::log("f%.1f h%.0f w%.0f", c.fovY, c.height, c.width);

	switch (c.type) {
	case RENDER_TYPE_TOPSCREEN:
		if (c.iodMapFunc)
			iodMapFunc = c.iodMapFunc;
		else
			iodMapFunc = defaultIODMap;
		highRes =
			c.wide && !config::wideIsUnsupported; // disable if not supported
		if (config::wideIsUnsupported)
			Console::warn("wide mode not supported");
		stereo = c.stereo && !c.ortho;
		if (!target[0])
			Console::warn("Could not create render targets");
		aspectRatio = C3D_AspectRatioTop;
		break;
	case RENDER_TYPE_BOTTOMSCREEN:
		aspectRatio = C3D_AspectRatioBot;
		stereo		= false;
		highRes		= false;
		break;
	case RENDER_TYPE_TEXTURE:
		aspectRatio = 1.f;
		stereo		= false;
		highRes		= false;
		// TODO I still need to figure out render textures
		break;
	default:
		Console::warn("Invalid render type");
	}

	this->parent = &parent;
}

void Camera::setActive(bool active) { this->active = active; }

void Camera::Render() {
	if (!active)
		return;

	culledList
		.clear(); // remove all the old objects without deallocating the space
				  // since 99% of the time it won't need to change the space

	// camera setup

	C3D_Mtx projection;
	Mtx_Identity(&projection);
	float iod = 0;
	if (stereo && !orthographic)
		iod = iodMapFunc(
			osGet3DSliderState()); // only calculate if actually necessary
	bool useWide = highRes && !(iod > 0.f);

	switch (type) {
	case RENDER_TYPE_TOPSCREEN:
		if (useWide) {
			C3D_RenderTargetSetOutput(target[2], GFX_TOP, GFX_LEFT,
									  CAM_DISPLAY_TRANSFER_FLAGS);
			C3D_RenderTargetClear(target[2], C3D_CLEAR_ALL, bgcolor, 0);
			C3D_FrameDrawOn(target[2]);

		} else {
			C3D_RenderTargetSetOutput(target[0], GFX_TOP, GFX_LEFT,
									  CAM_DISPLAY_TRANSFER_FLAGS);
			C3D_RenderTargetClear(target[0], C3D_CLEAR_ALL, bgcolor, 0);
			C3D_FrameDrawOn(target[0]);
		}
		break;
	case RENDER_TYPE_BOTTOMSCREEN:
		C3D_RenderTargetSetOutput(target[3], GFX_BOTTOM, GFX_LEFT,
								  CAM_DISPLAY_TRANSFER_FLAGS);
		C3D_RenderTargetClear(target[3], C3D_CLEAR_ALL, bgcolor, 0);
		C3D_FrameDrawOn(target[3]);
		break;
	case RENDER_TYPE_TEXTURE:
		// TODO add handling for this
		break;
	}

	if (orthographic)
		Mtx_OrthoTilt(&projection, -width / 2, width / 2, -height / 2,
					  height / 2, nearClip, farClip, false); // no perspective
	else if (stereo) // both perspective and 3D
		Mtx_PerspStereoTilt(&projection, C3D_AngleFromDegrees(fovY),
							aspectRatio, nearClip, farClip, -iod, focalLength,
							false);
	else // perspective but no 3D
		Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(fovY), aspectRatio,
					  nearClip, farClip, false);

	gfxSetWide(
		useWide); // Enable wide mode if wanted and if not rendering in stereo

	// scene setup

	transform *trans = parent->getComponent<transform>();

	// if no transform, the scene can't be rendered. there should
	// always be a transform but the check is here just in case
	if (!trans)
		return;
	C3D_Mtx view = *trans;
	Mtx_Inverse(&view);

	// lighting update

	for (int i = 0; i < HW_MAX_LIGHTS; ++i)
		if (lights::active[i])
			lights::active[i]->update(&view);

	osTickCounterStart(&stats::profiling::cnt_cull);

	// C3D_Mtx vp;

	// Mtx_Multiply(&vp, &projection, &view); // create view projection matrix,
	// much faster since it saves a ton of matrix multiplications and is also
	// useful for frustum culling

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
	// float invnearM = 1/sqrtf(nearN.x*nearN.x + nearN.y*nearN.y +
	// nearN.z*nearN.z); float invfarM = 1/sqrtf(farN.x*farN.x + farN.y*farN.y +
	// farN.z*farN.z); float invtopM = 1/sqrtf(topN.x*topN.x + topN.y*topN.y +
	// topN.z*topN.z); float invbotM = 1/sqrtf(botN.x*botN.x + botN.y*botN.y +
	// botN.z*botN.z); float invleftM = 1/sqrtf(leftN.x*leftN.x +
	// leftN.y*leftN.y + leftN.z*leftN.z); float invrightM =
	// 1/sqrtf(rightN.x*rightN.x + rightN.y*rightN.y + rightN.z*rightN.z);

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
	// for (GameObject& obj : parent->s.objects) {
	//     // mesh* m = NULL;
	//     // if (!obj) continue; // skip null objects, there shouldn't be any
	//     so I think it can be removed, and I can't remove it from the list at
	//     this step so it might end up being kinda slow Renderer* r; if (!(r =
	//     obj.getComponent<Renderer>())) continue; // skip objects with no set
	//     renderer

	//     // transform* t = obj->getComponent<transform>();
	//     // C3D_FVec p = FVec3_Subtract(pos, t->position);

	//     // bool left = -m->radius < FVec3_Dot(leftN, p);
	//     // if (!left) continue;
	//     // bool right = -m->radius < FVec3_Dot(rightN, p);
	//     // if (!right) continue;
	//     // bool top = -m->radius < FVec3_Dot(topN, p);
	//     // if (!top) continue;
	//     // bool bot = -m->radius < FVec3_Dot(botN, p);
	//     // if (!bot) continue;
	//     // bool back = -m->radius < FVec3_Dot(farN, p);
	//     // if (!back) continue;
	//     // bool front = -m->radius < FVec3_Dot(nearN, p);
	//     // if (!front) continue;

	//     // if (!(left + right + back + front + top + bot)) continue; // if
	//     not in any then skip it (this means it is outside the frustum)
	//     // culledList.push_back(r);
	// }
	osTickCounterUpdate(&stats::profiling::cnt_cull);
	stats::profiling::rnd_cull = osTickCounterRead(&stats::profiling::cnt_cull);
	// actually render stuff for left eye
	osTickCounterStart(&stats::profiling::cnt_meshrnd);
	// render objects
	parent->s.reg.view<Renderer, transform>().each([&](auto &rnd, auto &trans) {
		// C3D_FVec p = FVec3_Subtract(pos, trans->position);

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

		// if (!(left + right + back + front + top + bot)) continue; // if not
		// in any then skip it (this means it is outside the frustum)
		rnd.render(view, projection, cullingMask);
	});
	// for (Renderer* rend : culledList) rend->render(view, projection);
	osTickCounterUpdate(&stats::profiling::cnt_meshrnd);
	stats::profiling::rnd_meshrnd =
		osTickCounterRead(&stats::profiling::cnt_meshrnd);

	// render text
	// C2D_SceneTarget(target[(type == RENDER_TYPE_TOPSCREEN) ? (stereo ? 0 : 2)
	// : 3]); //TODO defer text/ui rendering

	if (!(stereo && iod > 0.0f) || type == RENDER_TYPE_BOTTOMSCREEN)
		return; // stop after first eye is drawn unless 3d is enabled and on
				// top screen

	// render right eye

	// gfxSet3D(true);
	C3D_RenderTargetSetOutput(target[1], GFX_TOP, GFX_RIGHT,
							  CAM_DISPLAY_TRANSFER_FLAGS);
	Mtx_PerspStereoTilt(&projection, C3D_AngleFromDegrees(fovY),
						C3D_AspectRatioTop, nearClip, farClip, iod, focalLength,
						false);

	C3D_RenderTargetClear(target[1], C3D_CLEAR_ALL, bgcolor, 0);
	C3D_FrameDrawOn(target[1]);

	osTickCounterStart(&stats::profiling::cnt_meshrnd);
	// render objects
	// for (Renderer* rend : culledList) rend->render(view, projection);
	osTickCounterUpdate(&stats::profiling::cnt_meshrnd);
	stats::profiling::rnd_meshrnd +=
		osTickCounterRead(&stats::profiling::cnt_meshrnd);

	// render text
	// C2D_SceneTarget(target[(type == RENDER_TYPE_TOPSCREEN) ? (stereo ? 1 : 2)
	// : 3]); //TODO defer text/ui rendering
}

COMPONENT_REGISTER(Camera)