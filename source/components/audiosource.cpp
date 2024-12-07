#include "audiosource.h"
#include "audiofilter.h"
#include "audiolistener.h"
#include "audiomanager.h"
#include "componentmanager.h"
#include "entt.hpp"
#include "gen_lut.h"
#include "scene.h"
#include "ql_assert.h"
#include "transform.h"
#include <3ds.h>
#include <algorithm>
#include <citro3d.h>
#include <cmath>

namespace ql {
	namespace {
		struct AudioSourceParams {
			AudioRolloff rolloffMode;
			bool mute, playOnWake, loop;
			channel_prio priority;
			float distMin, distMax, volume, stereoPan;
			char fname; // only here to signal the start of the file name
		};
	} // namespace

	AudioSource::AudioSource(GameObject &obj, const void *data) {
		parent = &obj;
		AudioSourceParams p;
		if (data)
			p = *(AudioSourceParams *)data;
		mute		= p.mute;
		priority	= 10;
		rolloffMode = p.rolloffMode;
		distMin		= p.distMin;
		distMax		= p.distMax;
		volume		= p.volume;
		stereoPan	= p.stereoPan;
		paused		= false;
		playing		= false;
		file		= &(*(AudioSourceParams *)data).fname;
	}

	void AudioSource::update() {
		if (voiceID < 0 || voiceID > 24)
			return; // invalid channel
		// pause channel if asked
		ndspChnSetPaused(voiceID, paused);

		if (!playing)
			return;

		// set mix based on distance, muting, volume, etc
		float lvol, rvol;
		if (mute) {
			lvol = 0;
			rvol = 0;
		} else {
			GameObject *l = nullptr;
			parent->s.reg.view<AudioListener>().each(
				[&](AudioListener &listener) {
					if (listener.active)
						l = listener.parent;
				});
			ASSERT(l != nullptr, "No listener in scene");

			float dist =
				FVec3_Distance(parent->getComponent<transform>()->position,
							   l->getComponent<transform>()->position);
			float distatten;
			switch (rolloffMode) {
			case ROLLOFF_LIN:
				distatten = volRollOffLin(dist, distMin, distMax);
				break;
			case ROLLOFF_LOG:
				distatten = volRollOffLog(dist, distMin, distMax);
				break;
			default:
			case ROLLOFF_NONE:
				distatten = 1;
				break;
			}
			stereoPan = std::clamp(stereoPan, -1.f, 1.f);
			unsigned char panidx = (uint8_t)((0.5f * stereoPan + 0.5f) * 255);
			lvol				 = volume * panVolLeft[panidx] * distatten;
			rvol				 = volume * panVolRight[panidx] * distatten;
		}

		lvol = std::clamp(lvol, 0.f, 1.f);
		rvol = std::clamp(rvol, 0.f, 1.f);

		// float mix[12] = {lvol, rvol};
		float mix[12] = {1.0, 1.0};
		ndspChnSetMix(voiceID, mix);

		// apply the filter on this object
		if (AudioFilter *filter = parent->getComponent<AudioFilter>())
			filter->apply(voiceID);
	}

	void AudioSource::Play() {
		Stop(); // end anything still playing

		// start audio thread
		voiceID = AudioManager::requestChannel(priority, file.c_str());
		playing = true;
	};

	void AudioSource::Play(const char *clip) {
		Stop(); // end anything still playing

		// start audio thread
		voiceID = AudioManager::requestChannel(priority, clip);
		playing = true;
	};

	void AudioSource::Stop() {
		playing = false;
		// stop audio thread
		AudioManager::freeChannel(voiceID);
		voiceID = 0;
	};

	// log_10 falloff
	float AudioSource::volRollOffLog(const float dist, const float min,
									 const float max) {
		return std::clamp(std::log10(dist / max) / std::log10(min / max), 0.f,
						  1.f);
	}

	// simple linear falloff
	constexpr float AudioSource::volRollOffLin(const float dist,
											   const float min,
											   const float max) {
		return std::clamp(dist / (max - min), 0.f, 1.f);
	}

	COMPONENT_REGISTER(AudioSource)
} // namespace ql