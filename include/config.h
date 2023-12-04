#pragma once

#include <3ds.h>

#define CITRA_TYPE 0x20000
#define CITRA_VERSION 11

namespace config {
    extern CFG_Language lang;
    extern CFG_SystemModel model;
    extern CFG_Region region;
    extern bool wideIsUnsupported;
    extern bool isOnCitra;

    Result getInfo();
}