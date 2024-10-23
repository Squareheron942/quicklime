#include "audiomanager.h"
#include "3ds/types.h"
#include "audiosource.h"
#include "console.h"
#include "decoders.h"
#include "defines.h"
#include "scenemanager.h"
#include "ql_assert.h"
#include "threads.h"
#include <3ds.h>
#include <cstdint>
#include <cstdio>

namespace ql {
	bool AudioManager::quitThread[NDSP_NUM_CHANNELS]  = {};
	LightEvent AudioManager::event[NDSP_NUM_CHANNELS] = {};
	LightLock AudioManager::l						  = {1};
	namespace {
		void callbackFunc(void *lock) {
			(void)lock;
			for (auto &ev : AudioManager::event)
				LightEvent_Signal(&ev);
		};
		static bool channelInUse[NDSP_NUM_CHANNELS] = {0};
		static channel_prio channelPrio[NDSP_NUM_CHANNELS];
		static Thread audioThreads[NDSP_NUM_CHANNELS];
	} // namespace

	void AudioManager::init() {
		LightLock_Init(&l);
		ndspSetCallback(callbackFunc, nullptr);
		for (auto &ev : event)
			LightEvent_Init(&ev, RESET_ONESHOT);
	}

	ndsp_channel AudioManager::requestChannel(channel_prio priority,
											  const char *file) {
		LightLock_Guard lock(l);
		channel_prio lowestprio = 0;
		ndsp_channel id			= -1;
		for (ndsp_channel i = 0; i < NDSP_NUM_CHANNELS; i++) {
			if (!channelInUse[i]) { // free channel
				id = i;
				Console::log("Free channel found");
				break;
			}
			if (channelPrio[i] <= lowestprio) {
				lowestprio = channelPrio[i];
				id		   = i;
			}
		}

		// channel allocation failed
		if (priority < lowestprio)
			return -1;
		if (id < 0 || id > 23)
			return -1;
		// force disable the running channel
		SceneManager::currentScene->reg.view<AudioSource>().each([&](auto &a) {
			if (a.voiceID == id)
				a.Stop();
		});

		quitThread[id]		 = false;
		channelInUse[id]	 = true;
		channelPrio[id]		 = priority;

		decodeparams p		 = {id, file};
		audiothreadfunc_t fn = nullptr;
		std::string extension =
			std::string{file}.substr(std::string{file}.find_last_of(".") + 1);
		if (extension == "opus") {
			fn = ql::opusdecode;
		} else if (extension == "ogg") {
			fn = ql::vorbisdecode;
		}

		ASSERT(fn != nullptr, "Invalid audio file type");

		int32_t threadpriority = 0x30;
		svcGetThreadPriority(&threadpriority, CUR_THREAD_HANDLE);
		threadpriority -= 1;
		threadpriority	 = threadpriority < 0x18 ? 0x18 : threadpriority;
		threadpriority	 = threadpriority > 0x3F ? 0x3F : threadpriority;
		audioThreads[id] = threadCreate(fn, &p, AUDIO_THREAD_STACK_SZ,
										threadpriority, -1, false);
		Console::log("Playing using channel %d", id);
		return id;
	}

	void AudioManager::freeChannel(ndsp_channel id) {
		if (id < 0 || id > 23)
			return;
		if (channelInUse[id]) {
			quitThread[id] = true;
			LightEvent_Signal(&event[id]);
			threadJoin(audioThreads[id], UINT64_MAX);
			threadFree(audioThreads[id]);
		}
		channelInUse[id] = false;
		channelPrio[id]	 = 0;
	}
} // namespace ql