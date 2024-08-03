#include "audiomanager.h"
#include "scenemanager.h"
#include "audiosource.h"

static bool channelInUse[NDSP_NUM_CHANNELS];
static channel_prio channelPrio[NDSP_NUM_CHANNELS];
static audiothreadfunc_t audioThreads[NDSP_NUM_CHANNEL];

void callbackFunc(void* lock) {
	LightLock_Guard l(*lock);
	for (auto& thread: audioThreads) {
		
	}
};

void AudioManager::init() {
	LightLock_Init(&l);
	ndspSetCallBack(callbackFunc, &l);
}

ndsp_channel AudioManager::requestChannel(channel_prio priority) {
	LightLock_Guard lock(l);
	channel_prio lowestprio = -1;
	ndsp_channel id = 0;
	for (unsigned char i = 0; i < NDSP_NUM_CHANNELS; i++) {
		if (!channelInUse[i]) { // free channel
			channelInUse[i] = true;
			channelPrio[i] = priority;
			return i;
		}
		if (channelPrio[i] <= lowestprio) {
			lowestprio = channelPrio[i];
			id = i;
		}
	}
	SceneManager::currentScene->reg
		.view<AudioSource>()
		.each([&](auto& a) {
			if (a.voiceID == id) a.Stop();
		});
	channelInUse[id] = true;
	channelPrio[id] = priority;
	return id;
}

void AudioManager::freeChannel(ndsp_channel id) {
	if (id < 0 || id > 23) return;
	channelInUse[id] = false;
	channelPrio[id] = 0;
}