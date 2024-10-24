#pragma once

#include <3ds.h>
#include <citro3d.h>
#include <vector>

namespace ql {
	class GameObject;
	class Renderer;

#define CAM_DISPLAY_TRANSFER_FLAGS                                             \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) |                     \
	 GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |  \
	 GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |                            \
	 GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

	enum RenderType {
		RENDER_TYPE_TOPSCREEN	 = 0,
		RENDER_TYPE_BOTTOMSCREEN = 1,
		RENDER_TYPE_TEXTURE		 = 2
	};

	// typedef struct {GameObject* obj; C3D_Mtx view;} icam_obj;

	class Camera {
		// void transformobjs_r(GameObject* obj, C3D_Mtx& parentmodelmtx,
		// C3D_FVec pos, C3D_FVec topN, C3D_FVec botN, C3D_FVec leftN, C3D_FVec
		// rightN, C3D_FVec nearN, C3D_FVec farN);
		LightLock lock;

	  public:
		GameObject *parent;
		float nearClip, farClip, focalLength = 2.f, fovY, height, width;
		/**
		 * @brief Dictates how to map the iod input from the slider to the
		 * rendering iod
		 * @param iod The inputted slider value
		 */
		float (*iodMapFunc)(float);
		bool stereo, orthographic, highRes, active;
		unsigned int bgcolor;
		RenderType type;
		static C3D_RenderTarget *target[4];
		std::vector<Renderer *> culledList;
		float aspectRatio;
		void setActive(bool active);
		unsigned short cullingMask; // sees all 16 layers by default
		Camera(GameObject &parent, const void *args);
		~Camera();
		void Render();
	};
} // namespace ql