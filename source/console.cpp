#include "console.h"


int Console::line = 0;
int Console::scrolloffset = 0;
int Console::menu = 0;
bool Console::console_needs_updating = false;
char* Console::textbuf[CONSOLE_NUM_LINES] = {0};