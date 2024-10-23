#include "stats.h"

namespace ql::stats {
	unsigned int _vertices	= 0;
	unsigned int _drawcalls = 0;
	float _x = 0, _y = 0, _z = 0, _w = 0;

	double profiling::go_upd = 0, profiling::go_lupd = 0,
		   profiling::go_fupd = 0, profiling::go_supd = 0;

	double profiling::rnd_camrnd = 0, profiling::rnd_cull = 0,
		   profiling::rnd_meshrnd = 0, profiling::rnd_setmtl = 0,
		   profiling::rnd_bndprg = 0, profiling::rnd_drawarr = 0;

	TickCounter profiling::cnt = {}, profiling::cnt_cull = {},
				profiling::cnt_meshrnd = {},
				profiling::cnt_setmtl  = {},
				profiling::cnt_bndprg  = {},
				profiling::cnt_drawarr = {},
				profiling::cnt_supd	  = {},
				profiling::cnt_camrnd  = {};
} // namespace ql::stats