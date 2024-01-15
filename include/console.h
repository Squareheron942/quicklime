#pragma once

#include "defines.h"
#include <stdio.h>
#include <3ds.h>
#include <stdarg.h>
#include "stats.h"
#include "controls.h"
#include "sl_time.h"

#define FPS_X 0
#define FPS_Y 2
#define CPU_X 9
#define CPU_Y 2
#define GPU_X 19
#define GPU_Y 2
#define POS_X 0
#define POS_Y 3
#define DRAW_X 0
#define DRAW_Y 4
#define MEM_X 0
#define MEM_Y 5

#define CONSOLE_TOP 2 // 1 line past bottom of static elements
#define CONSOLE_NUM_LINES 32 - CONSOLE_TOP

#define SENS_X 0
#define SENS_Y 2
#define DEAD_X 0
#define DEAD_Y 3

namespace {
    enum DEBUG_MENU {
        MENU_STATS = 0,
        MENU_CONSOLE = 1,
        MENU_SETTINGS = 2
    };
    
    enum CONSOLE_LOG_LEVEL {
        LOG_LEVEL_LOW,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_SUCCESS
    };
}

class Console {
    
    static int menu;
    static bool console_needs_updating;

    static inline void print_console_lines() {
        for (int i = 0; i < CONSOLE_NUM_LINES - 1; i++) 
            printf("\e[s\e[%u;0H\e[2K%s\e[u", CONSOLE_TOP + i, textbuf[i]);
        
    }

    static inline void basic_log(CONSOLE_LOG_LEVEL loglevel, const char* text, va_list args) {
        #if CONSOLE_ENABLED

        char* unformatted = NULL;

        switch (loglevel) {
            case LOG_LEVEL_LOW:
                unformatted = new char[snprintf(NULL, 0, "\e[2K\e[1;37m[DEBUG]\e[22m %s\e[0m", text) + 1];
                sprintf(unformatted, "\e[2K\e[1;37m[DEBUG]\e[22m %s\e[0m", text);
                break;
            case LOG_LEVEL_WARN:
                unformatted = new char[snprintf(NULL, 0, "\e[2K\e[1;33m[WARN]\e[22m %s\e[0m", text) + 1];
                sprintf(unformatted, "\e[2K\e[1;33m[WARN]\e[22m %s\e[0m", text);
                break;
            case LOG_LEVEL_ERROR:
                unformatted = new char[snprintf(NULL, 0, "\e[2K\e[1;31m[ERROR]\e[22m %s\e[0m", text) + 1];
                sprintf(unformatted, "\e[2K\e[1;31m[ERROR]\e[22m %s\e[0m", text);
                break;
            case LOG_LEVEL_SUCCESS:
                unformatted = new char[snprintf(NULL, 0, "\e[2K\e[1;31m[SUCCESS]\e[22m %s\e[0m", text) + 1];
                sprintf(unformatted, "\e[2K\e[1;32m[SUCCESS]\e[22m %s\e[0m", text);
                break;
        }

        if (line >= CONSOLE_NUM_LINES - 2) 
        {
            console_needs_updating = true;
            for (int i = 0; i < CONSOLE_NUM_LINES - 1; i++) 
                textbuf[i] = textbuf[i + 1];
            textbuf[line] = new char[vsnprintf(NULL, 0, unformatted, args) + 1];
            vsprintf(textbuf[line], unformatted, args);
        } else
        {
            textbuf[line] = new char[vsnprintf(NULL, 0, unformatted, args) + 1];
            vsprintf(textbuf[line], unformatted, args);
            if (menu == MENU_CONSOLE) printf("\e[s\e[%u;0H%s\e[u", CONSOLE_TOP + line, textbuf[line]);
            line++;
        }

        delete[] unformatted;

        #endif
    }

    Console() {}
    public:

    static void nextMenu() {
        menu++;
        menu %= 3;
        if (menu == MENU_CONSOLE) console_needs_updating = true;
        printf("\e[2J"); // clear screen
    }

    static int line, scrolloffset;
    static char* textbuf[CONSOLE_NUM_LINES]; // Stores a buffer containing all of the text previously inputted
    
    static inline void log(const char* text, ...) {
        va_list arg;
        va_start(arg, text); 
        basic_log(LOG_LEVEL_LOW, text, arg);
        va_end(arg);
    };

    static inline void warn(const char* text, ...) {
        va_list arg;
        va_start(arg, text);
        basic_log(LOG_LEVEL_WARN, text, arg);
        va_end(arg);
    };

    static inline void error(const char* text, ...) {
        va_list arg;
        va_start(arg, text);
        basic_log(LOG_LEVEL_ERROR, text, arg);
        va_end(arg);
    };

    static inline void success(const char* text, ...) {
        va_list arg;
        va_start(arg, text);
        basic_log(LOG_LEVEL_SUCCESS, text, arg);
        va_end(arg);
    };
    static inline void setFrameTime() {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHFPS: %.0f       \e[u", FPS_Y, FPS_X, 1.f/Time::deltaTime);
        printf("\e[s\e[%u;%uHCPU: %.2f       \e[u", CPU_Y, CPU_X, C3D_GetProcessingTime());
        printf("\e[s\e[%u;%uHGPU: %.2f       \e[u", GPU_Y, GPU_X, C3D_GetDrawingTime());
        #endif
    }

    static inline void setPosition(int x = stats::_x, int y = stats::_y, int z = stats::_z, int w = stats::_w) {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHPosition: %d %d %d %d        \e[u", POS_Y, POS_X, x, y, z, w);
        #endif
    }

    static inline void setDrawCalls(int dc = stats::_drawcalls) {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHDraw Calls: %u         \e[u", DRAW_Y, DRAW_X, dc);
        #endif
    }

    static inline void updateMemUsage() {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHAll RAM Usage: %lu/%lu MiB         \e[u", MEM_Y, MEM_X, osGetMemRegionUsed(MEMREGION_ALL)/1048576, (osGetMemRegionSize(MEMREGION_ALL))/1048576); // 124 on old, 228 on new;
        printf("\e[s\e[%u;%uHApplication RAM Usage: %lu/%lu MiB         \e[u", MEM_Y + 1, MEM_X, osGetMemRegionUsed(MEMREGION_APPLICATION)/1048576, (osGetMemRegionSize(MEMREGION_APPLICATION))/1048576); // mode 0 64mb, mode 2 96mb, mode 3 80mb, mode 4 72mb, mode 5 32mb, NEW: mode 6/8 124mb, mode 7 178mb
        printf("\e[s\e[%u;%uHSystem RAM Usage: %lu/%lu MiB         \e[u", MEM_Y + 2, MEM_X, osGetMemRegionUsed(MEMREGION_SYSTEM)/1048576, (osGetMemRegionSize(MEMREGION_SYSTEM))/1048576); // mode 0 44mb, mode 2 12mb, mode 3 28mb, mode 4 36mb, mode 5 76mb, mode 6/8 100mb, mode 7 46mb
        printf("\e[s\e[%u;%uHBase RAM Usage: %lu/%lu MiB         \e[u", MEM_Y + 3, MEM_X, osGetMemRegionUsed(MEMREGION_BASE)/1048576, (osGetMemRegionSize(MEMREGION_BASE))/1048576); // 20mb on old, 32mb on new
        #endif
    }

    static inline void dispSensitivity() {
        #if CONSOLE_ENABLED
        printf("\e[s\e[%u;%uHGyro Sensitivity: %f\e[u", SENS_Y, SENS_X, controls::gyroSensitivity());
        printf("\e[s\e[%u;%uHGyro Minimum Rate: %f deg/s\e[u", DEAD_Y, DEAD_X, controls::gyroDeadZone());
        #endif
    }

    static inline void update() {
        if (controls::getDown(controls::key::KEY_SELECT)) nextMenu();
        switch (menu) {
            case MENU_STATS:
                setFrameTime();
                setDrawCalls();
                setPosition();
                updateMemUsage();
                #if CONSOLE_ENABLED
                printf("\e[s\e[1;0H<                Stats                 >\e[u"); // 40 chars wide
                #endif
                break;
            case MENU_CONSOLE:
                if (console_needs_updating) print_console_lines();
                console_needs_updating = false;
                #if CONSOLE_ENABLED
                printf("\e[s\e[1;0H<               Console                >\e[u"); // 40 chars wide
                #endif
                break;
            case MENU_SETTINGS:
                dispSensitivity();
                #if CONSOLE_ENABLED
                printf("\e[s\e[1;0H<               Settings               >\e[u"); // 40 chars wide
                #endif
                break;
        }
    }
};