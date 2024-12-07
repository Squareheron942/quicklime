#pragma once

#include <3ds.h>
#include <citro3d.h>
#include <utility>
#include <vector>

namespace ql {
	class GameObject;
	class Renderer;
	class transform;

	enum DisplayTarget {
		DISPLAY_TOP	   = 0,
		DISPLAY_BOTTOM = 1
	};

	using iod_func = float (*)(float);

	class Camera {
		friend class GameObject;
	  private:
		static bool cameraObjectListDirty;
		LightLock lock;
		GameObject *parent;
		iod_func iodMapFunc = nullptr;
		DisplayTarget display;
		std::vector<std::pair<Renderer *, transform *>> culledBuckets[3];
		C3D_Mtx cameraMatrix[2];
		void updateMatrix(float iod);

	  public:
		float nearClip, farClip, focalLength = 2.f, fovY, height, width, aspectRatio;
		bool stereoEnabled, orthographic, highRes, active;
		uint32_t backgroundColour, cullingMask;
		Camera(GameObject &parent, const void *args);
		~Camera();
		void Render();
		void setActive(bool active);
		/**
		 * @brief Set function which dictates how to map the iod input from the slider to the
		 * rendering iod
		 */
		void setIodMapFunc(iod_func func);
		void enableStereo(bool enabled);
		void enableWide(bool enabled);
	};
} // namespace ql