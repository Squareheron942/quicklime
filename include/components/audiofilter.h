#pragma once

#include "audio/audiomanager.h"
namespace ql {
	enum FilterType {
		FILTER_HIGHPASS,
		FILTER_LOWPASS,
		FILTER_BANDPASS,
		FILTER_NOTCH
	};

	class GameObject;

	class AudioFilter {
		GameObject *p;
		FilterType t;
		bool onListener;

	  public:
		AudioFilter(GameObject &owner, const void *data);
		void apply(ndsp_channel channel = -1);
		void disable(ndsp_channel channel = -1);
	};
} // namespace ql