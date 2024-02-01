#pragma once

#include <citro3d.h>
#include <3ds.h>

namespace stats {
    extern unsigned int _drawcalls;
    extern float _x, _y, _z, _w;
    extern unsigned int _vertices;

    namespace profiling {
        extern TickCounter cnt, cnt_camrnd, cnt_meshrnd, cnt_setmtl, cnt_bndprg, cnt_drawarr, cnt_cull, cnt_supd;
        extern double go_upd, go_lupd, go_fupd, go_supd;
        extern double rnd_camrnd, rnd_cull, rnd_meshrnd, rnd_setmtl, rnd_bndprg, rnd_drawarr;
    }
}