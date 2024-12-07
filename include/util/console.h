#pragma once

#include <3ds.h>
#include <stdarg.h>

#define FPS_X 0
#define FPS_Y 2
#define CPU_X 9
#define CPU_Y 2
#define GPU_X 19
#define GPU_Y 2
#define SCN_X 0
#define SCN_Y 3
#define DRAW_X 0
#define DRAW_Y 4
#define MEM_X 0
#define MEM_Y 5
#define NV_X 0
#define NV_Y 9

#define CONSOLE_TOP 2 // 1 line past bottom of static elements
#define CONSOLE_NUM_LINES 30 - CONSOLE_TOP
#define CONSOLE_NUM_CHARS 40

#define SENS_X 0
#define SENS_Y 2
#define DEAD_X 0
#define DEAD_Y 3

namespace ql {
	class Console {
		Console() = delete;
		enum DEBUG_MENU {
			MENU_STATS,
			MENU_CONSOLE,
			MENU_SETTINGS,
			MENU_PROFILING
		};

		enum CONSOLE_LOG_LEVEL {
			LOG_LEVEL_LOW	  = '7',
			LOG_LEVEL_WARN	  = '3',
			LOG_LEVEL_ERROR	  = '1',
			LOG_LEVEL_SUCCESS = '2'
		};

		static void print_console_lines();

		static int menu, line, scrolloffset;
		static bool console_needs_updating;
		static LightLock _l;

		static void basic_log(CONSOLE_LOG_LEVEL loglevel, const char *text,
							  va_list args);

	  public:
		static char textbuf[CONSOLE_NUM_LINES]
						   [40 + 15 + 1]; // Stores a buffer containing all of
										  // the text previously inputted

		static void nextMenu();
		static void update();
		static void init();

		static void log(const char *text, ...);
		static void warn(const char *text, ...);
		static void error(const char *text, ...);
		static void success(const char *text, ...);
	};
} // namespace ql
