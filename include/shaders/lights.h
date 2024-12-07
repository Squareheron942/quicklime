#pragma once

#include "util/ql_assert.h"
#include <3ds.h>
#include <citro3d.h>

#define HW_NUM_LIGHTS 8

namespace ql::lights {
	static C3D_LightEnv shared_lightenv;
	static C3D_Light lights[HW_NUM_LIGHTS];
	
	namespace {
		static int _lights_in_use = 0;
		static u8 lights_in_use_mask = 0;
		inline bool __attribute__((always_inline)) light_in_use(int i) { return (lights_in_use_mask & (1 << i)); }
		inline void __attribute__((always_inline)) enable_light(int i) { 
			lights_in_use_mask |= 1 << i;
			++_lights_in_use;
			C3D_LightEnable(&lights[i], true); 
		}
		inline void __attribute__((always_inline)) disable_light(int i) { 
			lights_in_use_mask &= ~(1 << i);
			--_lights_in_use;
			C3D_LightEnable(&lights[i], false); 
		}
	}
	
	static const int& lights_in_use = _lights_in_use;
	
	static int allocateLight() {
		for (int i = 0; i < HW_NUM_LIGHTS; i++) {
			if (!light_in_use(i)) {
				enable_light(i);
				return i;
			}
		}
		return -1;
	}
	static void freeLight(int lightIdx) {
		ASSERT(lightIdx >= 0 && lightIdx < HW_NUM_LIGHTS, "Invalid light index");
		if (lightIdx < 0 || lightIdx >= HW_NUM_LIGHTS) return;
		disable_light(lightIdx);
	}
	
	
}