#include "opusdecode.h"
#include <3ds.h>
#include "console.h"

namespace {
    const char *opusStrError(int error)
    {
        switch(error) {
            case OP_FALSE:
                return "OP_FALSE: A request did not succeed.";
            case OP_HOLE:
                return "OP_HOLE: There was a hole in the page sequence numbers.";
            case OP_EREAD:
                return "OP_EREAD: An underlying read, seek or tell operation "
                    "failed.";
            case OP_EFAULT:
                return "OP_EFAULT: A NULL pointer was passed where none was "
                    "expected, or an internal library error was encountered.";
            case OP_EIMPL:
                return "OP_EIMPL: The stream used a feature which is not "
                    "implemented.";
            case OP_EINVAL:
                return "OP_EINVAL: One or more parameters to a function were "
                    "invalid.";
            case OP_ENOTFORMAT:
                return "OP_ENOTFORMAT: This is not a valid Ogg Opus stream.";
            case OP_EBADHEADER:
                return "OP_EBADHEADER: A required header packet was not properly "
                    "formatted.";
            case OP_EVERSION:
                return "OP_EVERSION: The ID header contained an unrecognised "
                    "version number.";
            case OP_EBADPACKET:
                return "OP_EBADPACKET: An audio packet failed to decode properly.";
            case OP_EBADLINK:
                return "OP_EBADLINK: We failed to find data we had seen before or "
                    "the stream was sufficiently corrupt that seeking is "
                    "impossible.";
            case OP_ENOSEEK:
                return "OP_ENOSEEK: An operation that requires seeking was "
                    "requested on an unseekable stream.";
            case OP_EBADTIMESTAMP:
                return "OP_EBADTIMESTAMP: The first or last granule position of a "
                    "link failed basic validity checks.";
            default:
                return "Unknown error.";
        }
    }

    // Main audio decoding logic
    // This function pulls and decodes audio samples from opusFile_ to fill waveBuf_
    bool fillBuffer(OggOpusFile *opusFile_, ndspWaveBuf *waveBuf_, int samplesperbuf, int channelspersample) {
        // Decode samples until our waveBuf is full
        int totalSamples = 0;
        while(totalSamples < samplesperbuf) {
            int16_t *buffer = waveBuf_->data_pcm16 + (totalSamples *
                channelspersample);
            const size_t bufferSize = (samplesperbuf - totalSamples) *
                channelspersample;

            // Decode bufferSize samples from opusFile_ into buffer,
            // storing the number of samples that were decoded (or error)
            const int samples = op_read_stereo(opusFile_, buffer, bufferSize);
            if(samples <= 0) {
                if(samples == 0) break;  // No error here

                Console::error("op_read_stereo: error %d (%s)", samples,
                    opusStrError(samples));
                break;
            }
            
            totalSamples += samples;
        }

        // If no samples were read in the last decode cycle, we're done
        if(totalSamples == 0) {
            Console::log("Playback complete");
            return false;
        }

        // Pass samples to NDSP
        waveBuf_->nsamples = totalSamples;
        ndspChnWaveBufAdd(0, waveBuf_);
        DSP_FlushDataCache(waveBuf_->data_pcm16,
            totalSamples * channelspersample * sizeof(int16_t));

        return true;
    }
    // Audio thread
    // This handles calling the decoder function to fill NDSP buffers as necessary
    void audioThread(void *const args_) {
        opusthreadargs args = *(opusthreadargs *)args_;

        while(!*args.quit) {  // Whilst the quit flag is unset,
                        // search our waveBufs and fill any that aren't currently
                        // queued for playback (i.e, those that are 'done')
            for(size_t i = 0; i < AUDIO_NUM_WAVBUFS; ++i) {
                if(args.waveBufs[i].status != NDSP_WBUF_DONE) {
                    continue;
                }
                
                if(!fillBuffer(args.OpusFile, &args.waveBufs[i], args.samplesperbuf, args.channelspersample)) {   // Playback complete
                    return;
                }
            }


            // Wait for a signal that we're needed again before continuing,
            // so that we can yield to other things that want to run
            // (Note that the 3DS uses cooperative threading)
            LightEvent_Wait(args.event);
        }
        Console::log("Audio thread ended");
    }
}

OpusDecode::OpusDecode(audio_params params) {
    Console::log("Opus Decoder Constructor");
    int error = 0;

    opusFile = op_open_file("romfs:/sample.opus", &error);
    if (error) {
        Console::error("Error loading audio");
        Console::error("error %d (%s)", error, opusStrError(error));
        return;
    }
    audio_params p = {
        op_channel_count(opusFile, -1),
        48000
    };
    this->AudioInit(p);
}

OpusDecode::OpusDecode(std::string file) : args{NULL, 0, 0} {
    int error = 0;
    opusFile = op_open_file("romfs:/sample.opus", &error);
    if (error) {
        Console::error("Error loading audio");
        Console::error("error %d (%s)", error, opusStrError(error));
        return;
    }
    audio_params p = {
        op_channel_count(opusFile, -1),
        48000
    };
    this->AudioInit(p);
    Console::log("Done audio initialization");
}

void OpusDecode::Play(audio_params params) {
    // Set the thread priority to the main thread's priority ...
    int32_t priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    // ... then subtract 1, as lower number => higher actual priority ...
    priority -= 1;
    // ... finally, clamp it between 0x18 and 0x3F to guarantee that it's valid.
    priority = priority < 0x18 ? 0x18 : priority;
    priority = priority > 0x3F ? 0x3F : priority;

    args.OpusFile = opusFile;
    args.samplesperbuf = 48000 * 120 / 1000;
    args.channelspersample = 2;
    args.event = &this->event;
    args.quit = &this->quit;
    args.waveBufs = this->waveBufs;

    // Allocate audio buffer
    const size_t bufferSize = args.samplesperbuf * args.channelspersample * AUDIO_NUM_WAVBUFS;
    audioBuffer = (int16_t *)linearAlloc(bufferSize);
    if(!audioBuffer) {
        Console::error("Failed to allocate audio buffer\n");
        return;
    }

    // Setup waveBufs for NDSP
    memset(&waveBufs, 0, sizeof(waveBufs));
    int16_t *buffer = audioBuffer;

    for(size_t i = 0; i < 3; ++i) {
        waveBufs[i].data_vaddr = buffer;
        waveBufs[i].status     = NDSP_WBUF_DONE;

        buffer += args.samplesperbuf * args.channelspersample / sizeof(buffer[0]);
    }

    // Start the thread, passing our opusFile as an argument.
    threadId = threadCreate(audioThread, &this->args, THREAD_STACK_SZ, priority, THREAD_AFFINITY, false);
}

void OpusDecode::Play() {
    // Set the thread priority to the main thread's priority ...
    int32_t priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    // ... then subtract 1, as lower number => higher actual priority ...
    priority -= 1;
    // ... finally, clamp it between 0x18 and 0x3F to guarantee that it's valid.
    priority = priority < 0x18 ? 0x18 : priority;
    priority = priority > 0x3F ? 0x3F : priority;

    args.OpusFile = opusFile;
    args.samplesperbuf = 48000 * 120 / 1000;
    args.channelspersample = 2;
    args.event = &this->event;
    args.quit = &this->quit;
    args.waveBufs = this->waveBufs;

    // Allocate audio buffer
    const size_t bufferSize = args.samplesperbuf * args.channelspersample * AUDIO_NUM_WAVBUFS;
    audioBuffer = (int16_t *)linearAlloc(bufferSize);
    if(!audioBuffer) {
        Console::error("Failed to allocate audio buffer\n");
        return;
    }

    // Setup waveBufs for NDSP
    memset(&waveBufs, 0, sizeof(waveBufs));
    int16_t *buffer = audioBuffer;

    for(size_t i = 0; i < 3; ++i) {
        waveBufs[i].data_vaddr = buffer;
        waveBufs[i].status     = NDSP_WBUF_DONE;

        buffer += args.samplesperbuf * args.channelspersample / sizeof(buffer[0]);
    }

    // Start the thread, passing our opusFile as an argument.
    threadId = threadCreate(audioThread, &this->args, THREAD_STACK_SZ, priority, THREAD_AFFINITY, false);
    Console::log("Decoder Playing");
}

OpusDecode::~OpusDecode() {
    op_free(opusFile);
}