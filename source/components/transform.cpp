#include "transform.h"
#include "componentmanager.h"

transform::~transform() {
	Console::log("transform destructor");
}

COMPONENT_REGISTER(transform)