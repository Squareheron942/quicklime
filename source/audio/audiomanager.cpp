#include "audiomanager.h"
#include "scenemanager.h"
#include "audiosource.h"

static bool channelInUse[NDSP_NUM_CHANNELS];
static channel_prio channelPrio[NDSP_NUM_CHANNELS];

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