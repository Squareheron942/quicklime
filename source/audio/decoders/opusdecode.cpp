#include "audiomanager.h"
#include "console.h"
#include "decoders.h"
#include <cstdint>
#include <opusfile.h>
#include <string>

namespace {
	static const int SAMPLE_RATE	 = 48000; // Opus is fixed at 48kHz
	static const int SAMPLES_PER_BUF = SAMPLE_RATE * 120 / 1000; // 120ms buffer
	static const int CHANNELS_PER_SAMPLE = 2;
	static const size_t WAVEBUF_SIZE =
		SAMPLES_PER_BUF * CHANNELS_PER_SAMPLE * sizeof(int16_t);

	// Main audio decoding logic
	// This function pulls and decodes audio samples from opusFile_ to fill
	// waveBuf_
	bool fillBuffer(OggOpusFile *opusFile_, ndspWaveBuf *waveBuf_,
					ndsp_channel chn) {
		// Decode samples until our waveBuf is full
		int totalSamples = 0;
		while (totalSamples < SAMPLES_PER_BUF) {
			int16_t *buffer =
				waveBuf_->data_pcm16 + (totalSamples * CHANNELS_PER_SAMPLE);
			const size_t bufferSize =
				(SAMPLES_PER_BUF - totalSamples) * CHANNELS_PER_SAMPLE;

			// Decode bufferSize samples from opusFile_ into buffer,
			// storing the number of samples that were decoded (or error)
			const int samples = op_read_stereo(opusFile_, buffer, bufferSize);
			if (samples <= 0)
				break;

			totalSamples += samples;
		}

		// If no samples were read in the last decode cycle, we're done
		if (totalSamples == 0)
			return false;

		// Pass samples to NDSP
		waveBuf_->nsamples = totalSamples;
		ndspChnWaveBufAdd(chn, waveBuf_);
		DSP_FlushDataCache(waveBuf_->data_pcm16, totalSamples *
													 CHANNELS_PER_SAMPLE *
													 sizeof(int16_t));

		return true;
	}
} // namespace

void ql::opusdecode(void *data) {
	if (!data)
		return;
	decodeparams p	 = *(decodeparams *)data;
	std::string file = p.filename;

	// initialization

	ndspChnReset(p.chn);
	ndspChnSetInterp(p.chn, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(p.chn, SAMPLE_RATE); // opus is fixed at 48kHz
	ndspChnSetFormat(p.chn, NDSP_FORMAT_STEREO_PCM16);

	int error			  = 0;
	OggOpusFile *opusFile = op_open_file(file.c_str(), &error);

	if (error) {
		Console::error("Error %d while loading opus file", error);
		return;
	}
	ndspWaveBuf s_waveBufs[3] = {};
	int16_t *audioBuffer	  = (int16_t *)linearAlloc(0);
	if (!audioBuffer) {
		Console::error("Failed to allocate audio buffer");
		return;
	}

	// Setup waveBufs for NDSP
	int16_t *buffer = audioBuffer;

	for (size_t i = 0; i < 3; ++i) {
		s_waveBufs[i].data_vaddr = buffer;
		s_waveBufs[i].status	 = NDSP_WBUF_DONE;

		buffer += WAVEBUF_SIZE / sizeof(audioBuffer[0]);
	}

	while (!AudioManager::quitThread[p.chn]) {
		// Whilst the quit flag is unset,
		// search our waveBufs and fill any that aren't currently
		// queued for playback (i.e, those that are 'done')
		for (size_t i = 0; i < 3; ++i) {
			if (s_waveBufs[i].status != NDSP_WBUF_DONE)
				continue;
			if (!fillBuffer(opusFile, &s_waveBufs[i],
							p.chn)) // Playback complete
				goto exit;
		}

		LightEvent_Wait(&AudioManager::event[p.chn]);
	}
exit:
	ndspChnReset(p.chn);
	linearFree(audioBuffer);
	op_free(opusFile);
	Console::log("Opus thread exited!");
}
