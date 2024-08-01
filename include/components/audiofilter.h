#pragma once

#include "componentmanager.h"
#include "audiomanager.h"

enum FilterType {
	HighPass,
	LowPass,
	BandPass,
	Notch
};

class GameObject;
class AudioFilter {
	FilterType t;
	bool onListener;
	public:
	AudioFilter(GameObject& obj, const void* params);
	void apply(ndsp_channel channel = -1); // applies to specific channel
};