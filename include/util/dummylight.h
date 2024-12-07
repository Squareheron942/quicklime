#pragma once

#include <citro3d.h>

namespace ql {
	class DummyLight {
		friend class Light;
		public:
		inline DummyLight() {};
		inline DummyLight(void* data) {
			(void)data;
		}
		inline DummyLight(DummyLight&&) {}
		inline DummyLight& operator=(DummyLight&&) { return *this; }
		inline void set(C3D_Mtx& modelView) {(void)modelView;}
	};
}