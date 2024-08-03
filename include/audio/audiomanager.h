#pragma once

#define NDSP_NUM_CHANNELS 24

#include "threads.h"

using ndsp_channel = int8_t;
using channel_prio = uint8_t;
using audiothreadfunc_t = void (*)();

class AudioManager {
	AudioManager() = delete;
	LightLock l;
	public:
	ndsp_channel requestChannel(channel_prio priority);
	void freeChannel(ndsp_channel id);
	void init();
};