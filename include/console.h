#pragma once

#include <stdio.h>
#include <3ds.h>
#include <stdarg.h>
#include <malloc.h>

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
#define NV_X 0
#define NV_Y 9

#define CONSOLE_TOP 2 // 1 line past bottom of static elements
#define CONSOLE_NUM_LINES 32 - CONSOLE_TOP

#define SENS_X 0
#define SENS_Y 2
#define DEAD_X 0
#define DEAD_Y 3

class Console {
	Console() = delete;
	enum DEBUG_MENU {
        MENU_STATS = 0,
        MENU_CONSOLE = 1,
        MENU_SETTINGS = 2,
        MENU_PROFILING = 3
    };

    enum CONSOLE_LOG_LEVEL {
        LOG_LEVEL_LOW,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_SUCCESS
    };

    static void print_console_lines();

    static int menu, line, scrolloffset;
    static bool console_needs_updating;
    static LightLock lock;

    static void basic_log(CONSOLE_LOG_LEVEL loglevel, const char* text, va_list args);
    public:
    static char* textbuf[CONSOLE_NUM_LINES]; // Stores a buffer containing all of the text previously inputted

    static void nextMenu();
    static void update();
    static void init();

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
};