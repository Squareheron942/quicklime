#include "c3d/types.h"
#include "camera.h"
#include "componentmanager.h"
#include "config.h"
#include "gameobject.h"
#include "ql_assert.h"
#include "renderer.h"
#include "scene.h"
#include "transform.h"
#include "lights.h"
#include "light.h"
#include <3ds.h>
#include <citro3d.h>
#include <utility>

namespace ql {
	namespace {
		static const int CAM_DISPLAY_TRANSFER_FLAGS =
			GX_TRANSFER_FLIP_VERT(0) |
			GX_TRANSFER_OUT_TILED(0) |
			GX_TRANSFER_RAW_COPY(0) |
			GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
			GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
			GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO);

		// framebuffers
		static C3D_RenderTarget *target[4] = {
			C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8), // top left
			C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8), // top right
			C3D_RenderTargetCreate(240, 800, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8), // top wide
			C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8)	 // bottom
		};

		static const void target_init() {
			static bool initialized;
			if (initialized)
				return;
			for (auto t : target)
				ASSERT(t != nullptr, "Was not able to allocate render target");
			C3D_RenderTargetSetOutput(target[0], GFX_TOP, GFX_LEFT, CAM_DISPLAY_TRANSFER_FLAGS);
			C3D_RenderTargetSetOutput(target[1], GFX_TOP, GFX_RIGHT, CAM_DISPLAY_TRANSFER_FLAGS);
			C3D_RenderTargetSetOutput(target[2], GFX_TOP, GFX_LEFT, CAM_DISPLAY_TRANSFER_FLAGS);
			C3D_RenderTargetSetOutput(target[3], GFX_BOTTOM, GFX_LEFT, CAM_DISPLAY_TRANSFER_FLAGS);
			initialized = true;
		}
		static const void target_clear(DisplayTarget targetDisplay, bool useWide, bool use3D, uint32_t backgroundColour) {
			switch (targetDisplay) {
			case DISPLAY_TOP:
				if (useWide) {
					C3D_RenderTargetClear(target[2], C3D_CLEAR_ALL, backgroundColour, 0);
				} else {
					C3D_RenderTargetClear(target[0], C3D_CLEAR_ALL, backgroundColour, 0);
					if (use3D) C3D_RenderTargetClear(target[1], C3D_CLEAR_ALL, backgroundColour, 0);
				}
				break;
			case DISPLAY_BOTTOM:
				C3D_RenderTargetClear(target[3], C3D_CLEAR_ALL, 0, 0);
				break;
			default:
				break;
			}
		}

		struct cam_args {
			u32 layermask		  = ~0;
			DisplayTarget display = DISPLAY_TOP;
			float nearClip = 0.1f, farClip = 1000.f;
			uint32_t backgroundColour = 0xFF349beb;
			// ortho camera properties
			float height = 24.f;
			float width	 = 40.f;
			// perspective camera properties
			float fovY	 = 55.f; // default for splatoon human form
			bool stereo	 = true; // whether to use 3D
			bool active	 = true;
			bool wide	 = true;
			bool ortho	 = false;
		};

		template <typename T, typename comparatorfunc_t>
		void insertionSort(std::vector<T> &array, comparatorfunc_t comparator) {
			for (size_t j = 1; j < array.size(); ++j) {
				for (size_t k = 0; k < j; k++) {
					if (comparator(array[k], array[j])) {
						T temp = array[j];
						for (size_t l = j; l > k; --l)
							array[l] = array[l - 1];
						array[k] = temp;
					}
				}
			}
		}

		void sortObjects(auto culledBuckets, C3D_FVec& position) {
			auto backToFront = [&](std::pair<Renderer *, transform *> &a, std::pair<Renderer *, transform *> b) {
				const C3D_FVec delta_a = FVec3_Subtract(a.second->position, position);
				const C3D_FVec delta_b = FVec3_Subtract(b.second->position, position);
				const float dista2	   = delta_a.x * delta_a.x + delta_a.y * delta_a.y + delta_a.z * delta_a.z;
				const float distb2	   = delta_b.x * delta_b.x + delta_b.y * delta_b.y + delta_b.z * delta_b.z;

				return dista2 < distb2;
			};
			auto frontToBack = [&](std::pair<Renderer *, transform *> &a, std::pair<Renderer *, transform *> b) {
				const C3D_FVec delta_a = FVec3_Subtract(a.second->position, position);
				const C3D_FVec delta_b = FVec3_Subtract(b.second->position, position);
				const float dista2	   = delta_a.x * delta_a.x + delta_a.y * delta_a.y + delta_a.z * delta_a.z;
				const float distb2	   = delta_b.x * delta_b.x + delta_b.y * delta_b.y + delta_b.z * delta_b.z;

				return dista2 > distb2;
			};
			insertionSort(culledBuckets[QUEUE_OPAQUE], frontToBack);
			insertionSort(culledBuckets[QUEUE_TRANSPARENT], backToFront);
			insertionSort(culledBuckets[QUEUE_OVERLAY], backToFront);
		}

		float defaultIodMapFunc(float iod) { return iod * 0.2f; }
	} // namespace

	bool Camera::cameraObjectListDirty = true;
	
	Camera::Camera(GameObject &owner, const void *args) {
		ASSERT(args != nullptr, "Invalid camera constructor arg");
		target_init();
		cam_args c;
		if (args)
			c = *(cam_args *)args;
		parent = &owner;
		active		  = c.active;
		stereoEnabled = c.stereo;
		highRes		  = c.wide & !stereoEnabled & !config::wideIsUnsupported;
		backgroundColour = c.backgroundColour;
		display = c.display;
		cullingMask = c.layermask;
		nearClip = c.nearClip;
		farClip = c.farClip;
		height = c.height;
		width = c.width;
		orthographic = c.ortho;
		fovY = c.fovY;
		iodMapFunc = defaultIodMapFunc;
	}

	Camera::~Camera() {}

	void Camera::Render() {
		if (!active)
			return;

		float iod  = iodMapFunc(osGet3DSliderState());
		bool use3D = stereoEnabled && (iod != 0);
		gfxSet3D(use3D);
		gfxSetWide(highRes);

		target_clear(display, highRes, use3D, backgroundColour);
		updateMatrix(iod);

		// split all objects into buckets
		if (cameraObjectListDirty) {
			cameraObjectListDirty = false;
			for (auto& bucket : culledBuckets) bucket.clear();
			parent->s
				.reg
				.view<Renderer, transform>()
				.each(
					[&](auto &renderer, auto &transform) {
						// do culling here
						culledBuckets[renderer.queue()].emplace_back(&renderer, &transform);
					});
		}
		// sort
		transform *t	  = parent->getComponent<transform>();
		C3D_FVec position = t->position;
		C3D_Mtx view	  = *t;
		Mtx_Inverse(&view);
		C3D_Mtx modelView[2];
		Mtx_Multiply(&modelView[0], &cameraMatrix[0], &view);
		Mtx_Multiply(&modelView[1], &cameraMatrix[1], &view);

		sortObjects(culledBuckets, position);
		
		// set lights
		C3D_LightEnvBind(&lights::shared_lightenv);

		parent->s.reg.view<Light>().each([&](auto &light) { light.setSelf(view); });

		// draw
		for (auto &bucket : culledBuckets) {
			for (auto &object : bucket) {
				object.first->render(view, modelView[0], cullingMask);
			}
		}
		if (use3D) { // second eye
			C3D_FrameDrawOn(target[1]);
			for (auto &bucket : culledBuckets) {
				for (auto &object : bucket) {
					object.first->render(view, modelView[1], cullingMask);
				}
			}
		}
	}

	void Camera::setActive(bool active) { this->active = active; }
	void Camera::setIodMapFunc(iod_func func) {
		if (!func)
			return;
		this->iodMapFunc = func;
	}

	void Camera::updateMatrix(float iod) {
		if (orthographic) // no perspective
			Mtx_OrthoTilt(&cameraMatrix[0], -width / 2, width / 2, -height / 2, height / 2, nearClip, farClip, false);
		else if (stereoEnabled) { // perspective and 3D
			Mtx_PerspStereoTilt(&cameraMatrix[0], C3D_AngleFromDegrees(fovY), aspectRatio, nearClip, farClip, +iod, focalLength, false);
			Mtx_PerspStereoTilt(&cameraMatrix[1], C3D_AngleFromDegrees(fovY), aspectRatio, nearClip, farClip, -iod, focalLength, false);
		} else // perspective but no 3D
			Mtx_PerspTilt(&cameraMatrix[0], C3D_AngleFromDegrees(fovY), aspectRatio, nearClip, farClip, false);
	}
	void Camera::enableStereo(bool enabled) {
		stereoEnabled = enabled & !config::stereoIsUnsupported & (display == DISPLAY_TOP);
		highRes &= !stereoEnabled;
	}
	void Camera::enableWide(bool enabled) {
		highRes = enabled & !config::wideIsUnsupported & (display == DISPLAY_TOP);
		stereoEnabled &= !highRes;
	}

	COMPONENT_REGISTER(Camera)
} // namespace ql