#pragma once

#include <array>
#include <cstdlib>
#include "componentmanager.h"
#include "audiomanager.h"

enum AudioRolloff {
	ROLLOFF_NONE, // constant volume
	ROLLOFF_LOG, // logarithmic falloff
	ROLLOFF_LIN // linear falloff
};

class GameObject;

class AudioSource {
	friend class Scene; // allow scene to call update
	friend class AudioManager; // allow audio manager to access/modify info
	GameObject* parent;
	ndsp_channel voiceID = -1; // 0 to 24
	AudioRolloff rolloffMode;
	float distMin, distMax;
	static const float panVolLeft[256], panVolRight[256];
	static float volRollOffLog(const float dist, const float min, const float max);
	static float volRollOffLin(const float dist, const float min, const float max);
	void update();
	public:
	bool paused, mute;
	float volume, stereoPan;
	AudioSource(GameObject& obj, const void* params);
	void Play();
	void Stop();
};