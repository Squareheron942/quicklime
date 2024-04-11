#include "vorbisdecode.h"
#include <3ds.h>
#include "console.h"

static const int SAMPLE_RATE = 48000;            // Opus is fixed at 48kHz
static const int SAMPLES_PER_BUF = SAMPLE_RATE * AUDIO_WAVBUF_TIME_MS;  // 120ms buffer
static const int CHANNELS_PER_SAMPLE = 2;        // We ask libopusfile for
                                                 // stereo output; it will down
                                                 // -mix for us as necessary.

static const size_t WAVEBUF_SIZE = SAMPLES_PER_BUF * CHANNELS_PER_SAMPLE
    * sizeof(int16_t);                           // Size of NDSP wavebufs

namespace {
    // Audio thread
    // This handles calling the decoder function to fill NDSP buffers as necessary
    void audioThread(void *const args_) {
        vorbisthreadargs args = *(vorbisthreadargs *)args_;

        while(!*args.quit) {  // Whilst the quit flag is unset,
                        // search our waveBufs and fill any that aren't currently
                        // queued for playback (i.e, those that are 'done')
            for(size_t i = 0; i < AUDIO_NUM_WAVBUFS; ++i) {
                auto& buf = args.waveBufs[i];
                char* bufferOut = reinterpret_cast<char*>(buf.data_pcm16);

                if(buf.status != NDSP_WBUF_DONE) {
                    continue;
                }
                
                uint64_t samplesRead = 0;
	            int samplesToRead = args.bufsize;

                while(samplesToRead > 0)
                {
                    static int current_section;
                    int samplesJustRead =
                        ov_read(args.vorbisFile, bufferOut,
                                samplesToRead > 4096 ? 4096	: samplesToRead,
                                &current_section);

                    if(samplesJustRead < 0)
                        goto exit;
                    else if(samplesJustRead == 0)
                    {
                        /* End of file reached. */
                        goto exit;
                    }

                    samplesRead += samplesJustRead;
                    samplesToRead -= samplesJustRead;
                    bufferOut += samplesJustRead;
                }

                buf.nsamples = samplesRead >> 1; // half since 2 channels

                ndspChnWaveBufAdd(args.channel, &buf);

                DSP_FlushDataCache(buf.data_pcm16, args.bufsize);
            }

            // Wait for a signal that we're needed again before continuing,
            // so that we can yield to other things that want to run
            // (Note that the 3DS uses cooperative threading)
            LightEvent_Wait(args.event);
        }
        exit:
        audio_shared_inf::ndsp_used_channels &= ~BIT(args.channel);
        Console::log("Audio thread ended");
        Console::log("Channels: %p", audio_shared_inf::ndsp_used_channels);
    }
}

VorbisDecode::VorbisDecode(std::string file) : args{NULL, 0, 0} {
    bool error = false;
    if((f = fopen(file.c_str(), "rb")) == NULL)
		error = true;

	if(ov_open(f, &vorbisfile, NULL, 0) < 0)
		error = true;

	if((vi = ov_info(&vorbisfile, -1)) == NULL)
		error = true;

    if (error) {
        Console::error("Error loading vorbis audio");
        return;
    }

    args.vorbisFile = &vorbisfile;
    // args.samplesperbuf = SAMPLES_PER_BUF;
    // args.channelspersample = CHANNELS_PER_SAMPLE;
    args.event = &this->event;
    args.quit = &this->quit;
    args.waveBufs = this->waveBufs;
    args.channel = channel;

    Console::log("Vorbis decoder created");

    // Setup NDSP
    ndspChnReset(channel);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(channel, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(channel, SAMPLE_RATE);
    ndspChnSetFormat(channel, NDSP_FORMAT_STEREO_PCM16);

    // Allocate audio buffer
    const size_t bufferSize = WAVEBUF_SIZE * AUDIO_NUM_WAVBUFS;
    audioBuffer = (int16_t *)linearAlloc(bufferSize);
    if(!audioBuffer) {
        printf("Failed to allocate audio buffer\n");
        return;
    }

    // Setup waveBufs for NDSP
    memset(&waveBufs, 0, sizeof(waveBufs));
    int16_t *buffer = (int16_t*)audioBuffer;

    for(size_t i = 0; i < AUDIO_NUM_WAVBUFS; ++i) {
        waveBufs[i].data_vaddr = buffer;
        waveBufs[i].status     = NDSP_WBUF_DONE;

        buffer += WAVEBUF_SIZE / sizeof(buffer[0]);
    }

    // Set the thread priority to the main thread's priority ...
    int32_t priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    // ... then subtract 1, as lower number => higher actual priority ...
    priority -= 1;
    // ... finally, clamp it between 0x18 and 0x3F to guarantee that it's valid.
    priority = priority < 0x18 ? 0x18 : priority;
    priority = priority > 0x3F ? 0x3F : priority;

    // Start the thread, passing our opusFile as an argument.
    threadId = threadCreate(audioThread, &this->args, THREAD_STACK_SZ, priority, THREAD_AFFINITY, false);
    Console::log("Decoder Playing");
}

VorbisDecode::~VorbisDecode() {
    Stop(); // end audio thread etc
    ov_clear(&vorbisfile);
	fclose(f);
}