#include "stats.h"

unsigned int stats::_vertices = 0;
unsigned int stats::_drawcalls = 0;
float stats::_x = 0, stats::_y = 0, stats::_z = 0, stats::_w = 0;

double stats::profiling::go_upd = 0,
    stats::profiling::go_lupd = 0,
    stats::profiling::go_fupd = 0,
    stats::profiling::go_supd = 0;

double stats::profiling::rnd_camrnd = 0,
    stats::profiling::rnd_cull = 0,
    stats::profiling::rnd_meshrnd = 0,
    stats::profiling::rnd_setmtl = 0,
    stats::profiling::rnd_bndprg = 0,
    stats::profiling::rnd_drawarr = 0;

TickCounter stats::profiling::cnt = {},
    stats::profiling::cnt_cull = {},
    stats::profiling::cnt_meshrnd = {},
    stats::profiling::cnt_setmtl = {},
    stats::profiling::cnt_bndprg = {},
    stats::profiling::cnt_drawarr = {},
    stats::profiling::cnt_supd = {},
    stats::profiling::cnt_camrnd = {};