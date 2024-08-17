#pragma once

#include "util/defines.h"
#include <3ds.h>
#include <string>

using ndsp_channel		= int8_t;
using channel_prio		= uint8_t;
using audiothreadfunc_t = void (*)(void *);

struct decodeparams {
	ndsp_channel chn;
	const char *filename;
};

class AudioManager {
	AudioManager() = delete;
	static LightLock l;

  public:
	static bool quitThread[NDSP_NUM_CHANNELS];
	static LightEvent event[NDSP_NUM_CHANNELS];
	/*
	Requests a channel to play audio file on.
	@param priority Priority from 0-255 with which to make request,
	will block lower-priority channel if none available
	@param filename Full path of the audio file to be played
	@return NDSP channel ID the audio is played on, or -1 if no channel
	available
	*/
	static ndsp_channel requestChannel(channel_prio priority,
									   const char *filename);
	/*
	Stops given channel from playing and ends any thread which is currently
	using it. Thread may keep playing until the end of the buffer is reached
	(~120ms max)
	*/
	static void freeChannel(ndsp_channel id);
	static void init();
};