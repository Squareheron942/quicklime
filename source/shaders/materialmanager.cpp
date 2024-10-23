#include "materialmanager.h"
#include "ql_assert.h"
#include "shader.h"
#include "threads.h"
#include <memory>

namespace ql {
	LightLock MaterialManager::_l = {1};

	std::unique_ptr<shader> MaterialManager::makeMaterial(const char *name,
														  FILE *matF) {
		LightLock_Guard l(_l);
		ASSERT(getMaterialMap().find(name) != getMaterialMap().end(),
			   "Unknown material");
		std::unique_ptr<shader> s(getMaterialMap()[name](matF));
		return s;
	}
} // namespace ql