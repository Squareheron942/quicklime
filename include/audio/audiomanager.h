#pragma once

#define NDSP_NUM_CHANNELS 24

#include "threads.h"

using ndsp_channel = unsigned char;
using channel_prio = unsigned char;

class AudioManager {
	AudioManager() = delete;
	LightLock l = {1};
	public:
	ndsp_channel requestChannel(channel_prio priority);
};