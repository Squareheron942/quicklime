#include "mp3decode.h"
#include "console.h"
#include "stdio.h"
#include "mp3file.h"

namespace {

    static ssize_t replace_read(void * file, void * buffer, size_t length) {
        return fread(buffer, 1, length, file);
    }

    static off_t replace_lseek(void * file, off_t to, int whence) {
        if (fseek(file, to, whence) < 0)
            return -1;

        return ftell(file);
    }

    // Main audio decoding logic
    // This function pulls and decodes audio samples from mpg123 handle to fill waveBuf_
    bool fillBuffer(mpg123_handle* mh, ndspWaveBuf *waveBuf_, int samplesperbuf, int channelspersample, int channel) {
        // Decode samples until our waveBuf is full
        int totalSamples = 0;
        while(totalSamples < samplesperbuf) {
            int16_t *buffer = waveBuf_->data_pcm16 + (totalSamples *
                channelspersample);
            const size_t bufferSize = (samplesperbuf - totalSamples) *
                channelspersample;

            // Decode bufferSize samples from handle into buffer,
            // storing the number of samples that were decoded (or error)
            size_t samples = 0;
            mpg123_read(mh, reinterpret_cast<unsigned char*>(buffer), bufferSize, &samples);
            if(samples == 0) break;  // No error here
            
            totalSamples += samples;
        }

        // If no samples were read in the last decode cycle, we're done
        if(totalSamples == 0) {
            Console::log("Playback complete");
            return false;
        }

        // Pass samples to NDSP
        waveBuf_->nsamples = totalSamples;
        ndspChnWaveBufAdd(channel, waveBuf_);
        DSP_FlushDataCache(waveBuf_->data_pcm16,
            totalSamples * channelspersample * sizeof(int16_t));

        return true;
    }
    // Audio thread
    // This handles calling the decoder function to fill NDSP buffers as necessary
    void audioThread(void *const args_) {
        mp3threadargs args = *(mp3threadargs *)args_;

        while(!*args.quit) {  // Whilst the quit flag is unset,
                        // search our waveBufs and fill any that aren't currently
                        // queued for playback (i.e, those that are 'done')
            for(size_t i = 0; i < AUDIO_NUM_WAVBUFS; ++i) {
                if(args.waveBufs[i].status != NDSP_WBUF_DONE) {
                    continue;
                }
                
                if(!fillBuffer(args.mh, &args.waveBufs[i], args.samplesperbuf, args.channelspersample, args.channel)) {   // Playback complete
                    goto exit;
                }
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



MP3Decode::MP3Decode(std::string file) {
    if ((file = fopen(file.c_str(), "r")) == nullptr) {
        Console::error("File not found");
        return;
    }

    int err = mpg123_init();
	int encoding = 0;

	if(err != MPG123_OK)
		return;
    
    mh = mpg123_new(NULL, &err);

	if(!mh)
	{
		Console::error("MP3 decode error:");
        Console::error(mpg123_plain_strerror(err));
		return;
	}

    mpg123_replace_reader_handle(mh, replace_read, replace_lseek, nullptr);

    if(mpg123_open_handle(mh, file) != MPG123_OK || mpg123_getformat(mh, &samplerate, &channels, &encoding) != MPG123_OK)
	{
		Console::error("mpg123 error:");
        Console::error(mpg123_strerror(mh));
		return;
	} 
	mpg123_format_none(mh);
	mpg123_format(mh, samplerate, MPG123_STEREO, MPG123_ENC_SIGNED_16);
	bufsize = mpg123_outblock(mh) * 16;
}

MP3Decode::~MP3Decode() {
    mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
}