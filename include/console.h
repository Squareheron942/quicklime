#pragma once

#include "defines.h"
#include <stdio.h>
#include <3ds.h>
#include <stdarg.h>

#define FPS_X 0
#define FPS_Y 0
#define FRAME_X 15
#define FRAME_Y 0
#define POS_X 0
#define POS_Y 1
#define DRAW_X 0
#define DRAW_Y 2
#define MEM_X 0
#define MEM_Y 3

#define CONSOLE_TOP MEM_Y + 3 // 1 line past bottom of static elements
#define CONSOLE_NUM_LINES 24 - CONSOLE_TOP

class Console {
    Console() {}
    public:

    static int line, scrolloffset;
    static char* textbuf[CONSOLE_NUM_LINES]; // Stores a buffer containing all of the text previously inputted
    
    static inline void log(const char* text, ...) {
        #if CONSOLE_ENABLED
        va_list args;
        va_start (args, text);

        if (line >= CONSOLE_NUM_LINES - 1) 
        { // loop text position
            line = 1;
        } else
        { // just add text to next line
            line++;
        }

        char* unformatted = new char[snprintf(NULL, 0, "\e[2K\e[1;37m[DEBUG]\e[22m %s", text) + 1];
        sprintf(unformatted, "\e[2K\e[1;37m[DEBUG]\e[22m %s\e[0m", text);
        textbuf[line] = new char[vsnprintf(NULL, 0, unformatted, args) + 1];
        vsprintf(textbuf[line], unformatted, args);
        printf("\e[s\e[%u;0H%s\e[u", CONSOLE_TOP + line, textbuf[line]);
        va_end(args);
        delete[] unformatted;

        #endif
    };

    static inline void warn(const char* text, ...) {
        va_list args;
        va_start (args, text);
        #if CONSOLE_ENABLED
        if (line >= CONSOLE_NUM_LINES - 1) { // scroll text
            line = 1;
            // scrolloffset++;
            // for (int i = 0; i < CONSOLE_NUM_LINES - 1; i++) {
            //     textbuf[i] = textbuf[i + 1]; // scroll each line down
            //     printf("\e[s\e[%u;0H%s\e[0m\e[u", i + CONSOLE_TOP, textbuf[i]);
            // }
        } else { // just add text to next line
            line++;
        }
        char* unformatted = new char[snprintf(NULL, 0, "\e[2K\e[1;33m[WARN]\e[22m %s", text) + 1];
        sprintf(unformatted, "\e[2K\e[1;33m[WARN]\e[22m %s\e[0m", text);
        textbuf[line] = new char[vsnprintf(NULL, 0, unformatted, args) + 1];
        vsprintf(textbuf[line], unformatted, args);
        printf("\e[s\e[%u;0H%s\e[u", CONSOLE_TOP + line, textbuf[line]);
        va_end(args);
        delete[] unformatted;
        #endif
    };

    static inline void error(const char* text, ...) {
        #if CONSOLE_ENABLED
        va_list args;
        va_start (args, text);
        if (line >= CONSOLE_NUM_LINES - 1) { // scroll text
            line = 1;
            // scrolloffset++;
            // for (int i = 0; i < CONSOLE_NUM_LINES - 1; i++) {
            //     textbuf[i] = textbuf[i + 1]; // scroll each line down
            //     printf("\e[s\e[%u;0H%s\e[0m\e[u", i + CONSOLE_TOP, textbuf[i]);
            // }
        } else { // just add text to next line
            line++;
        }
        char* unformatted = new char[snprintf(NULL, 0, "\e[2K\e[1;31m[ERROR]\e[22m %s", text) + 1];
        sprintf(unformatted, "\e[2K\e[1;31m[ERROR]\e[22m %s\e[0m", text);
        textbuf[line] = new char[vsnprintf(NULL, 0, unformatted, args) + 1];
        vsprintf(textbuf[line], unformatted, args);
        printf("\e[s\e[%u;0H%s\e[u", CONSOLE_TOP + line, textbuf[line]);
        va_end(args);
        delete[] unformatted;
        #endif
    };

    static inline void setFPS(int fps) {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHFPS: %u\e[u", FPS_Y, FPS_X, fps);
        #endif
    }
    static inline void setFrameTime(float frametime) {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHFrame Time: %f ms\e[u", FRAME_Y, FRAME_X, frametime);
        #endif
    }

    static inline void setPosition(int x, int y, int z) {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHPosition: %d %d %d\e[u", POS_Y, POS_X, x, y, z);
        #endif
    }

    static inline void setDrawCalls(int dc) {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHDraw Calls: %u\e[u", DRAW_Y, DRAW_X, dc);
        #endif
    }

    static inline void updateMemUsage() {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uH\e[2KAll RAM Usage: %2.2f%%\e[u", MEM_Y, MEM_X, (osGetMemRegionUsed(MEMREGION_ALL) * 100) / (float)(osGetMemRegionSize(MEMREGION_ALL)));
        printf("\e[s\e[%u;%uH\e[2KApplication RAM Usage: %2.2f%%\e[u", MEM_Y + 1, MEM_X, (osGetMemRegionUsed(MEMREGION_APPLICATION) * 100) / (float)(osGetMemRegionSize(MEMREGION_APPLICATION)));
        printf("\e[s\e[%u;%uH\e[2KSystem RAM Usage: %2.2f%%\e[u", MEM_Y + 2, MEM_X, (osGetMemRegionUsed(MEMREGION_SYSTEM) * 100) / (float)(osGetMemRegionSize(MEMREGION_SYSTEM)));
        printf("\e[s\e[%u;%uH\e[2KBase RAM Usage: %2.2f%%\e[u", MEM_Y + 3, MEM_X, (osGetMemRegionUsed(MEMREGION_BASE) * 100) / (float)(osGetMemRegionSize(MEMREGION_BASE)));
        #endif
    }
};