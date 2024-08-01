#include "audiosource.h"
#include <3ds.h>
#include "3ds/ndsp/channel.h"
#include "c3d/maths.h"
#include "entt.hpp"
#include <algorithm>
#include <cmath>
#include "transform.h"
#include "scene.h"
#include "audiolistener.h"
#include "audiofilter.h"
#include "sl_assert.h"

struct AudioSourceParams {
	bool mute, playOnWake, loop;
	char priority;
	float volume, pitch, stereoPan;
	// file name (not included since string so variable size)
};

AudioSource::AudioSource(GameObject& obj, const void* data) {
	
}

void AudioSource::update() {
	// pause channel if asked
	ndspChnSetPaused(voiceID, paused);
	
	// set mix based on distance, muting, volume, etc
	float lvol, rvol;
	if (mute) {
		lvol = 0;
		rvol = 0;
	} else {
		GameObject* l = nullptr;
		parent->s
			.reg
			.view<AudioListener>()
			.each([&](AudioListener& listener){ 
				if (listener.active)
					l = listener.parent;
			});
		ASSERT(l != nullptr, "No listener in scene");
		
		float dist = FVec3_Distance(
			parent->getComponent<transform>()->position, 
			l->getComponent<transform>()->position
		);
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
		std::clamp(stereoPan, -1.f, 1.f);
		unsigned char panidx = (unsigned char)((0.5f * stereoPan + 0.5f) * 255);
		lvol = volume * panVolLeft[panidx] * distatten;
		rvol = volume * panVolRight[panidx] * distatten;
	}
	std::clamp(lvol, 0.f, 1.f);
	std::clamp(rvol, 0.f, 1.f);
	
	float mix[12] = {lvol, rvol};
	ndspChnSetMix(voiceID, mix);
	
	// apply the filter on this object
	if (AudioFilter* filter = parent->getComponent<AudioFilter>())
		filter->apply(voiceID);
}

void AudioSource::Play() {};
void AudioSource::Stop() {};


// log_10 falloff
float AudioSource::volRollOffLog(const float dist, const float min, const float max) {
	return std::clamp(std::log10(dist/max)/std::log10(min/max), 0.f, 1.f);
}

// simple linear falloff
float AudioSource::volRollOffLin(const float dist, const float min, const float max) {
	return std::clamp(dist / (max - min), 0.f, 1.f);
}

COMPONENT_REGISTER(AudioSource)


const float AudioSource::panVolLeft[] {1.000000,0.998036,0.996049,0.994039,0.992007,0.989953,0.987877,0.985778,0.983658,0.981516,0.979351,0.977165,0.974958,0.972728,0.970478,0.968205,0.965912,0.963597,0.961261,0.958904,0.956526,0.954127,0.951707,0.949267,0.946806,0.944324,0.941822,0.939299,0.936757,0.934193,0.931610,0.929007,0.926384,0.923741,0.921078,0.918395,0.915693,0.912971,0.910230,0.907470,0.904690,0.901891,0.899073,0.896236,0.893380,0.890505,0.887611,0.884699,0.881768,0.878818,0.875850,0.872864,0.869859,0.866837,0.863796,0.860737,0.857660,0.854566,0.851454,0.848324,0.845176,0.842011,0.838829,0.835629,0.832412,0.829178,0.825927,0.822658,0.819373,0.816072,0.812753,0.809418,0.806066,0.802698,0.799313,0.795912,0.792495,0.789062,0.785612,0.782147,0.778666,0.775169,0.771657,0.768128,0.764585,0.761025,0.757451,0.753861,0.750256,0.746636,0.743001,0.739351,0.735686,0.732007,0.728312,0.724604,0.720880,0.717143,0.713391,0.709624,0.705844,0.702049,0.698241,0.694419,0.690583,0.686733,0.682869,0.678992,0.675102,0.671198,0.667281,0.663351,0.659408,0.655451,0.651482,0.647500,0.643505,0.639497,0.635477,0.631445,0.627400,0.623342,0.619273,0.615191,0.611097,0.606992,0.602874,0.598745,0.594604,0.590451,0.586287,0.582111,0.577924,0.573726,0.569516,0.565296,0.561064,0.556822,0.552569,0.548305,0.544030,0.539745,0.535450,0.531144,0.526827,0.522501,0.518164,0.513818,0.509461,0.505095,0.500719,0.496333,0.491938,0.487533,0.483119,0.478695,0.474263,0.469821,0.465370,0.460910,0.456441,0.451964,0.447478,0.442983,0.438480,0.433968,0.429449,0.424920,0.420384,0.415840,0.411288,0.406727,0.402159,0.397584,0.393001,0.388410,0.383812,0.379206,0.374594,0.369974,0.365347,0.360713,0.356072,0.351425,0.346771,0.342110,0.337443,0.332769,0.328089,0.323402,0.318710,0.314012,0.309307,0.304597,0.299881,0.295159,0.290431,0.285699,0.280960,0.276216,0.271468,0.266714,0.261954,0.257190,0.252421,0.247648,0.242869,0.238086,0.233299,0.228507,0.223711,0.218910,0.214106,0.209297,0.204484,0.199668,0.194848,0.190024,0.185196,0.180365,0.175531,0.170693,0.165852,0.161008,0.156161,0.151311,0.146458,0.141602,0.136744,0.131883,0.127020,0.122154,0.117286,0.112416,0.107543,0.102669,0.097792,0.092914,0.088034,0.083152,0.078269,0.073385,0.068498,0.063611,0.058723,0.053833,0.048942,0.044051,0.039158,0.034265,0.029371,0.024477,0.019582,0.014687,0.009791,0.004896};
const float AudioSource::panVolRight[] {0.000000,0.004896,0.009791,0.014687,0.019582,0.024477,0.029371,0.034265,0.039158,0.044051,0.048942,0.053833,0.058723,0.063611,0.068498,0.073385,0.078269,0.083152,0.088034,0.092914,0.097792,0.102669,0.107543,0.112416,0.117286,0.122154,0.127020,0.131883,0.136744,0.141602,0.146458,0.151311,0.156161,0.161008,0.165852,0.170693,0.175531,0.180365,0.185196,0.190024,0.194848,0.199668,0.204484,0.209297,0.214106,0.218910,0.223711,0.228507,0.233299,0.238086,0.242869,0.247648,0.252421,0.257190,0.261954,0.266714,0.271468,0.276217,0.280960,0.285699,0.290431,0.295159,0.299881,0.304597,0.309307,0.314012,0.318710,0.323402,0.328089,0.332769,0.337443,0.342110,0.346771,0.351425,0.356072,0.360713,0.365347,0.369974,0.374594,0.379206,0.383812,0.388410,0.393001,0.397584,0.402159,0.406727,0.411288,0.415840,0.420384,0.424920,0.429449,0.433968,0.438480,0.442983,0.447478,0.451964,0.456441,0.460910,0.465370,0.469821,0.474263,0.478695,0.483119,0.487533,0.491938,0.496333,0.500719,0.505095,0.509461,0.513818,0.518164,0.522501,0.526827,0.531144,0.535450,0.539745,0.544030,0.548305,0.552569,0.556822,0.561064,0.565296,0.569516,0.573726,0.577924,0.582111,0.586287,0.590451,0.594604,0.598745,0.602874,0.606992,0.611097,0.615191,0.619273,0.623342,0.627400,0.631445,0.635477,0.639497,0.643505,0.647500,0.651482,0.655451,0.659408,0.663351,0.667281,0.671198,0.675102,0.678992,0.682869,0.686733,0.690583,0.694419,0.698241,0.702050,0.705844,0.709624,0.713391,0.717143,0.720880,0.724604,0.728312,0.732007,0.735686,0.739351,0.743001,0.746636,0.750256,0.753861,0.757451,0.761025,0.764585,0.768128,0.771657,0.775169,0.778666,0.782147,0.785612,0.789062,0.792495,0.795912,0.799313,0.802698,0.806066,0.809418,0.812753,0.816072,0.819373,0.822658,0.825927,0.829178,0.832412,0.835629,0.838829,0.842011,0.845176,0.848324,0.851454,0.854566,0.857660,0.860737,0.863796,0.866837,0.869859,0.872864,0.875850,0.878818,0.881768,0.884699,0.887611,0.890505,0.893380,0.896236,0.899073,0.901891,0.904690,0.907470,0.910230,0.912971,0.915693,0.918395,0.921078,0.923741,0.926384,0.929007,0.931610,0.934194,0.936757,0.939299,0.941822,0.944324,0.946806,0.949267,0.951707,0.954127,0.956526,0.958904,0.961261,0.963597,0.965912,0.968205,0.970478,0.972728,0.974958,0.977165,0.979351,0.981516,0.983658,0.985779,0.987877,0.989953,0.992007,0.994039,0.996049,0.998036};