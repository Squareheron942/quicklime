#include "console.h"
#include "3ds/allocator/linear.h"
#include "defines.h"
#include "stats.h"
#include "controls.h"
#include "sl_time.h"


int Console::line = 0;
int Console::scrolloffset = 0;
int Console::menu = 1;
bool Console::console_needs_updating = false;
char* Console::textbuf[CONSOLE_NUM_LINES] = {0};
LightLock Console::lock;

void Console::print_console_lines() {
    for (int i = 0; i < CONSOLE_NUM_LINES - 1; i++)
        if (textbuf[i])
        	printf("\e[s\e[%u;0H\e[2K%s\e[u", CONSOLE_TOP + i, textbuf[i]);
}

void Console::init() {
	LightLock_Init(&lock);
}

inline void setFrameTime() {
    #if CONSOLE_ENABLED
    printf("\e[s\e[%u;%uHFPS: %.0f       \e[u", FPS_Y, FPS_X, 1.f/Time::deltaTime);
    printf("\e[s\e[%u;%uHCPU: %.2f       \e[u", CPU_Y, CPU_X, C3D_GetProcessingTime());
    printf("\e[s\e[%u;%uHGPU: %.2f       \e[u", GPU_Y, GPU_X, C3D_GetDrawingTime());
    #endif
}

inline void setPosition(int x = stats::_x, int y = stats::_y, int z = stats::_z, int w = stats::_w) {
    #if CONSOLE_ENABLED
    printf("\e[s\e[%u;%uHPosition: %d %d %d %d        \e[u", POS_Y, POS_X, x, y, z, w);
    #endif
}

inline void setDrawCalls(int dc = stats::_drawcalls) {
    #if CONSOLE_ENABLED
    printf("\e[s\e[%u;%uHDraw Calls: %u         \e[u", DRAW_Y, DRAW_X, dc);
    #endif
}

inline void updateMemUsage() {
    #if CONSOLE_ENABLED
    printf("\e[s\e[%u;%uHHeap Usage: %u KiB         \e[u", MEM_Y, MEM_X, mallinfo().uordblks/1024);
    printf("\e[s\e[%u;%uHLinear RAM Free: %lu KiB         \e[u", MEM_Y + 1, MEM_X, linearSpaceFree()/1024);
    printf("\e[s\e[%u;%uHVRAM Free: %lu KiB         \e[u", MEM_Y + 2, MEM_X, vramSpaceFree()/1024);
    #endif
}

inline void dispSensitivity() {
    #if CONSOLE_ENABLED
    printf("\e[s\e[%u;%uHGyro Sensitivity: %f\e[u", SENS_Y, SENS_X, controls::gyroSensitivity());
    printf("\e[s\e[%u;%uHGyro Minimum Rate: %f deg/s\e[u", DEAD_Y, DEAD_X, controls::gyroDeadZone());
    #endif
}

inline void dispProfiling() {
    #if CONSOLE_ENABLED
    printf("\e[s\e[%u;%uHScene Update: %f\e[u", 2, 0, stats::profiling::go_supd);
    printf("\e[s\e[%u;%uHUpdate: %f\e[u", 3, 2, stats::profiling::go_upd);
    printf("\e[s\e[%u;%uHLateUpdate: %f\e[u", 4, 2, stats::profiling::go_lupd);
    printf("\e[s\e[%u;%uHFixedUpdate: %f\e[u", 5, 2, stats::profiling::go_fupd);

    printf("\e[s\e[%u;%uHCamera::render(): %f\e[u", 7, 0, stats::profiling::rnd_camrnd);
    printf("\e[s\e[%u;%uHCulling: %f\e[u", 8, 2, stats::profiling::rnd_cull);
    printf("\e[s\e[%u;%uHMeshRenderer::render(): %f\e[u", 9, 2, stats::profiling::rnd_meshrnd);
    printf("\e[s\e[%u;%uHSetMaterial(): %f\e[u", 10, 4, stats::profiling::rnd_setmtl);
    printf("\e[s\e[%u;%uHBindProgram(): %f\e[u", 11, 6, stats::profiling::rnd_bndprg);
    printf("\e[s\e[%u;%uHDrawArrays(): %f\e[u", 12, 4, stats::profiling::rnd_drawarr);
    printf("\e[s\e[%u;%uHUpdContext(): %f\e[u", 13, 6, drw_ctx);
    printf("\e[s\e[%u;%uHGPUCMD_Add(): %f\e[u", 14, 6, drw_gpucmd);

    stats::profiling::rnd_setmtl = 0;
    stats::profiling::rnd_drawarr = 0;
    stats::profiling::rnd_bndprg = 0;
    drw_ctx = 0;
    drw_gpucmd = 0;
    #endif
}

inline void showNumVertices() {
    #if CONSOLE_ENABLED
    printf("\e[s\e[%u;%uHVertices: %u\e[u", NV_Y, NV_X, stats::_vertices);
    #endif
}

void Console::nextMenu() {
    menu++;
    menu %= 4;
    if (menu == MENU_CONSOLE) console_needs_updating = true;
    printf("\e[2J"); // clear screen
}

void Console::basic_log(CONSOLE_LOG_LEVEL loglevel, const char* text, va_list args) {
    #if CONSOLE_ENABLED
    LightLock_Lock(&lock);

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
            textbuf[i] = textbuf[i + 1]; // shift up all buffers
        textbuf[line] = new char[vsnprintf(NULL, 0, unformatted, args) + 1];
        vsprintf(textbuf[line], unformatted, args);
    } else
    {
        textbuf[line] = new char[vsnprintf(NULL, 0, unformatted, args) + 1];
        vsprintf(textbuf[line], unformatted, args);
        if (menu == MENU_CONSOLE) printf("\e[s\e[%u;0H%s\e[u", CONSOLE_TOP + line, textbuf[line]);
        strtok(textbuf[line], "\n");
        while(strtok(NULL, "\n")) line++;
        line++;
    }

    delete[] unformatted;

    LightLock_Unlock(&lock);
    #endif
}

void Console::update() {
	#if CONSOLE_ENABLED
	LightLock_Lock(&lock); // don't allow writing to the text buffer and outputting text at the same time
    if (controls::getDown(controls::key::KEY_SELECT)) nextMenu();
    switch (menu) {
        case MENU_STATS:
            setFrameTime();
            setDrawCalls();
            setPosition();
            updateMemUsage();
            showNumVertices();
            printf("\e[s\e[1;0H<                Stats                 >\e[u"); // 40 chars wide
            break;
        case MENU_CONSOLE:
            if (console_needs_updating) print_console_lines();
            console_needs_updating = false;
            printf("\e[s\e[1;0H<               Console                >\e[u"); // 40 chars wide
            break;
        case MENU_SETTINGS:
            dispSensitivity();
            printf("\e[s\e[1;0H<               Settings               >\e[u"); // 40 chars wide
            break;
        case MENU_PROFILING:
            dispProfiling();
            printf("\e[s\e[1;0H<               Profiling              >\e[u"); // 40 chars wide
            break;
    }
    LightLock_Unlock(&lock);
    #endif
}