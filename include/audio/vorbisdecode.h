#pragma once

#include <3ds.h>
#include "audiodecode.h"

#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "console.h"


struct vorbisthreadargs {
    OggVorbis_File* vorbisFile;
    LightEvent* event;
    volatile bool *quit;
    unsigned char channel;
    int bufsize;
    ndspWaveBuf *waveBufs;
};

class VorbisDecode : public AudioDecode {
    OggVorbis_File vorbisfile;
    vorbis_info    *vi;
    FILE           *f;
    const size_t   buffSize = 8 * 4096;
    vorbisthreadargs args;

    public:
    VorbisDecode(std::string file);
    virtual ~VorbisDecode() override;
};