#pragma once

#include "audio/audiomanager.h"
#include "util/gen_lut.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <string>

namespace ql {
	enum AudioRolloff {
		ROLLOFF_NONE, // constant volume
		ROLLOFF_LOG,  // logarithmic falloff
		ROLLOFF_LIN	  // linear falloff
	};

	class GameObject;

	class AudioSource {
		static constexpr auto panVolLeft =
			generateTable<256>([](double a) -> float {
				return std::sqrt(a * 2 / M_PI * std::sin(a));
			});
		static constexpr auto panVolRight =
			generateTable<256>([](double a) -> float {
				return std::sqrt((M_PI / 2 - a) * 2 / M_PI * std::cos(a));
			});
		static float volRollOffLog(const float dist, const float min,
								   const float max);
		static constexpr float volRollOffLin(const float dist, const float min,
											 const float max);
		friend class Scene;		   // allow scene to call update
		friend class AudioManager; // allow audio manager to access/modify info
		void update();
		GameObject *parent;
		ndsp_channel voiceID = -1; // 0 to 24
		channel_prio priority;
		AudioRolloff rolloffMode;
		float distMin, distMax;
		std::string file;
		bool playing, playOnWake;

	  public:
		bool paused, mute;
		float volume, stereoPan;
		AudioSource(GameObject &obj, const void *params);
		void Play();
		void Play(const char *clip);
		void Stop();
	};
} // namespace ql