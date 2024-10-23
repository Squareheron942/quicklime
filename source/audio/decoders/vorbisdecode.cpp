#include "audiomanager.h"
#include "console.h"
#include "decoders.h"
#include <cstdint>
#include <cstring>
#include <string>
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>

void ql::vorbisdecode(void *data) {
	if (!data)
		return;
	decodeparams p	 = *(decodeparams *)data;
	std::string file = p.filename;

	// initialization
	OggVorbis_File vorbisfile;
	FILE *f;
	vorbis_info *vi;

	bool error = false;
	if ((f = fopen(file.c_str(), "rb")) == NULL)
		error = true;

	if (ov_open(f, &vorbisfile, NULL, 0) < 0)
		error = true;

	if ((vi = ov_info(&vorbisfile, -1)) == NULL)
		error = true;

	if (error) {
		Console::error("Error loading vorbis audio");
		return;
	}

	Console::log("Vorbis decoder created");

	const int SAMPLE_RATE		  = vi->rate;
	const int SAMPLES_PER_BUF	  = SAMPLE_RATE * 200 / 1000; // 200ms buffer
	const int CHANNELS_PER_SAMPLE = 2;
	const size_t WAVEBUF_SIZE =
		SAMPLES_PER_BUF * CHANNELS_PER_SAMPLE * sizeof(int16_t);

	// Setup NDSP
	ndspChnReset(p.chn);
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspChnSetInterp(p.chn, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(p.chn, SAMPLE_RATE);
	ndspChnSetFormat(p.chn, NDSP_FORMAT_STEREO_PCM16);

	// Allocate audio buffer
	const size_t bufferSize = WAVEBUF_SIZE * 3;
	void *audioBuffer		= (int16_t *)linearAlloc(bufferSize);
	if (!audioBuffer) {
		Console::error("Failed to allocate audio buffer");
		return;
	}

	ndspWaveBuf waveBufs[3];
	// Setup waveBufs for NDSP
	memset(&waveBufs, 0, sizeof(waveBufs));
	int16_t *buffer = (int16_t *)audioBuffer;

	for (size_t i = 0; i < 3; ++i) {
		waveBufs[i].data_vaddr = buffer;
		waveBufs[i].status	   = NDSP_WBUF_DONE;

		buffer += WAVEBUF_SIZE / sizeof(buffer[0]);
	}

	while (!AudioManager::quitThread[p.chn]) {
		for (size_t i = 0; i < 3; ++i) {
			auto &buf		= waveBufs[i];
			char *bufferOut = reinterpret_cast<char *>(buf.data_pcm16);

			if (buf.status != NDSP_WBUF_DONE) {
				continue;
			}

			uint64_t samplesRead = 0;
			int samplesToRead	 = 8 * 4096;

			while (samplesToRead > 0) {
				static int current_section;
				int samplesJustRead =
					ov_read(&vorbisfile, bufferOut,
							samplesToRead > 4096 ? 4096 : samplesToRead,
							&current_section);

				if (samplesJustRead <= 0) {
					/* End of file reached. */
					goto exit;
				}

				samplesRead += samplesJustRead;
				samplesToRead -= samplesJustRead;
				bufferOut += samplesJustRead;
			}

			buf.nsamples = samplesRead >> 2; // half since 2 channels

			ndspChnWaveBufAdd(p.chn, &buf);

			DSP_FlushDataCache(buf.data_pcm16, 8 * 4096);
		}

		LightEvent_Wait(&AudioManager::event[p.chn]);
	}
exit:
	ndspChnReset(p.chn);
	linearFree(audioBuffer);
	ov_clear(&vorbisfile);
	fclose(f);
	Console::log("Vorbis thread exited!");
}