#include "audiolistener.h"
#include "componentmanager.h"
#include "gameobject.h"

namespace ql {
	AudioListener::AudioListener(GameObject &owner, const void *data)
		: parent(&owner), active(true) {
		(void)data;
	}

	COMPONENT_REGISTER(AudioListener);
} // namespace ql

bool test() {
	ql::Console::log("text");
}

bool testvar = test();