#include "console.h"
#include "defines.h"
#include "stats.h"
#include "controls.h"
#include "sl_time.h"
#include "threads.h"


int Console::line = 0;
int Console::scrolloffset = 0;
int Console::menu = 1;
bool Console::console_needs_updating = false;
char Console::textbuf[CONSOLE_NUM_LINES][56] = {{}};
LightLock Console::_l;

#define LOG(loglevel, text) \
	va_list arg;\
	va_start(arg, text);\
	basic_log(loglevel, text, arg);\
	va_end(arg);

unsigned int getnumlines(char* text) { 
	unsigned int i = 0; 
	strtok(text, "\n");
	do { ++i; } while(strtok(NULL, "\n"));
	return i;
}	

void Console::print_console_lines() {
    for (int i = 0; i < CONSOLE_NUM_LINES - 1; i++)
        printf("\e[s\e[%u;0H\e[2K%s\e[u", CONSOLE_TOP + i, textbuf[i]);
}

void Console::init() {
	LightLock_Init(&_l);
}

inline void setFrameTime() {
    #if CONSOLE
    printf("\e[s\e[%u;%uHFPS: %.0f       \e[u", FPS_Y, FPS_X, 1.f/Time::deltaTime);
    printf("\e[s\e[%u;%uHCPU: %.2f       \e[u", CPU_Y, CPU_X, C3D_GetProcessingTime());
    printf("\e[s\e[%u;%uHGPU: %.2f       \e[u", GPU_Y, GPU_X, C3D_GetDrawingTime());
    #endif
}

inline void setPosition(int x = stats::_x, int y = stats::_y, int z = stats::_z, int w = stats::_w) {
    #if CONSOLE
    printf("\e[s\e[%u;%uHPosition: %d %d %d %d        \e[u", POS_Y, POS_X, x, y, z, w);
    #endif
}

inline void setDrawCalls(int dc = stats::_drawcalls) {
    #if CONSOLE
    printf("\e[s\e[%u;%uHDraw Calls: %u         \e[u", DRAW_Y, DRAW_X, dc);
    #endif
}

inline void updateMemUsage() {
    #if CONSOLE
    printf("\e[s\e[%u;%uHHeap Usage: %u KiB         \e[u", MEM_Y, MEM_X, mallinfo().uordblks/1024);
    printf("\e[s\e[%u;%uHLinear RAM Free: %lu KiB         \e[u", MEM_Y + 1, MEM_X, linearSpaceFree()/1024);
    printf("\e[s\e[%u;%uHVRAM Free: %lu KiB         \e[u", MEM_Y + 2, MEM_X, vramSpaceFree()/1024);
    #endif
}

inline void dispSensitivity() {
    #if CONSOLE
    printf("\e[s\e[%u;%uHGyro Sensitivity: %f\e[u", SENS_Y, SENS_X, controls::gyroSensitivity());
    printf("\e[s\e[%u;%uHGyro Minimum Rate: %f deg/s\e[u", DEAD_Y, DEAD_X, controls::gyroDeadZone());
    #endif
}

inline void dispProfiling() {
    #if CONSOLE
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
    // printf("\e[s\e[%u;%uHUpdContext(): %f\e[u", 13, 6, drw_ctx);
    // printf("\e[s\e[%u;%uHGPUCMD_Add(): %f\e[u", 14, 6, drw_gpucmd);

    stats::profiling::rnd_setmtl = 0;
    stats::profiling::rnd_drawarr = 0;
    stats::profiling::rnd_bndprg = 0;
    // drw_ctx = 0;
    // drw_gpucmd = 0;
    #endif
}

inline void showNumVertices() {
    #if CONSOLE
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
    #if CONSOLE
    LightLock_Guard l(_l);
    const char* endcap = "\e[0m";
    static char base[40 + 15 + 1] = {"\e[2K\e[1;3_m"}; // same between all so might as well leave it that way

    base[9] = loglevel; // silly hack that works bc the loglevel enum
    
    vsnprintf(base + 11, 40, text, args); // add text after formatting
    unsigned int i = 0;
    while (base[++i] && i <= 40); // advance to null byte, stop at theoretical max
    strcpy(base + i, endcap); // safe because it is constant

    if (line >= CONSOLE_NUM_LINES - 2)
    {
        for (int i = 0; i < CONSOLE_NUM_LINES - 1; i++)
        	strncpy(textbuf[i], textbuf[i + 1], 55);
            // textbuf[i] = textbuf[i + 1]; // shift up all buffers
        strncpy(textbuf[line], base, 55);
    } else
    {
    	strncpy(textbuf[line], base, 55);
        if (menu == MENU_CONSOLE) printf("\e[s\e[%u;0H%s\e[u", CONSOLE_TOP + line, textbuf[line]);
        line++;
    }
    console_needs_updating = true;
    #endif
}

void Console::log(const char* text, ...) { LOG(LOG_LEVEL_LOW, text); };
void Console::warn(const char* text, ...) { LOG(LOG_LEVEL_WARN, text); };
void Console::error(const char* text, ...) {  LOG(LOG_LEVEL_ERROR, text); };
void Console::success(const char* text, ...) { LOG(LOG_LEVEL_SUCCESS, text); };

void Console::update() {
	#if CONSOLE
	LightLock_Guard l(_l);
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
    #endif
}