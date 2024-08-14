#pragma once

#define NDSP_NUM_CHANNELS 24

#include "threads.h"

using ndsp_channel		= int8_t;
using channel_prio		= uint8_t;
using audiothreadfunc_t = void (*)();

class AudioManager {
	AudioManager() = delete;
	static LightLock l;

  public:
	static ndsp_channel requestChannel(channel_prio priority);
	static void freeChannel(ndsp_channel id);
	static void init();
};