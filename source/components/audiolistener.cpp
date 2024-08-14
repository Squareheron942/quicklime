#include "audiolistener.h"
#include "componentmanager.h"
#include "gameobject.h"

AudioListener::AudioListener(GameObject &owner, const void *data)
	: parent(&owner), active(true) {
	(void)data;
}

COMPONENT_REGISTER(AudioListener)