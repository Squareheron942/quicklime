#pragma once

#include "defines.h"
#include <stdio.h>

#define FPS_X 0
#define FPS_Y 0
#define FRAME_X 15
#define FRAME_Y 0
#define POS_X 0
#define POS_Y 1
#define DRAW_X 0
#define DRAW_Y 2

class Console {
    Console() {}
    public:
    static void log(char* text);
    static void setFPS(int fps) {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHFPS: %u\e[u", FPS_Y, FPS_X, fps);
        #endif
    }
    static void setFrameTime(float frametime) {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHFrame Time: %f ms\e[u", FRAME_Y, FRAME_X, frametime);
        #endif
    }

    static void setPosition(int x, int y, int z) {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHPosition: %d %d %d\e[u", POS_Y, POS_X, x, y, z);
        #endif
    }

    static void setDrawCalls(int dc) {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHDraw Calls: %u\e[u", DRAW_Y, DRAW_X, dc);
        #endif
    }
};