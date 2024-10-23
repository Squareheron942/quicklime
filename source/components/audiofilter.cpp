#include "audiofilter.h"
#include "componentmanager.h"

void applyFilter(FilterType t, ndsp_channel c, bool biquad) {
	if (biquad)
		switch (t) {
		case FILTER_HIGHPASS:
		case FILTER_LOWPASS:
		case FILTER_BANDPASS:
		case FILTER_NOTCH:
			break;
		}
	else {
	}
}

AudioFilter::AudioFilter(GameObject &obj, const void *data) {}

// apply filter
void AudioFilter::apply(ndsp_channel channel) {
	switch (channel) {
	case -1: // all channels
		break;
	default: // specific channel

		break;
	}
}

COMPONENT_REGISTER(AudioFilter)