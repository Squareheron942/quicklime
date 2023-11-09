#include "console.h"


int Console::line = 0;
int Console::scrolloffset = 0;
char* Console::textbuf[CONSOLE_NUM_LINES] = {0};