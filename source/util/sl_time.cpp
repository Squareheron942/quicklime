#include "ql_time.h"

#include <3ds.h>

namespace ql {
	void Time::Update() {
		curtime = osGetTime();
		_delta	= (curtime - oldtime) * 0.001f;
		oldtime = curtime;
	}

	float Time::_delta				 = 0.0167f;
	const float &Time::deltaTime	 = _delta;
	unsigned long long Time::curtime = 0, Time::oldtime = 0;

} // namespace ql