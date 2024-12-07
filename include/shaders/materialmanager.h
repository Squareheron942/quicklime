#pragma once

#include "shader.h"
#include <3ds.h>
#include <memory>
#include <stdio.h>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace ql {
	typedef shader *(*shaderInstantiationFunc)(FILE *);
	template <typename T>
	concept validshader =
		requires(FILE *f) { T(f); } && std::is_base_of_v<shader, T>;

	class MaterialManager {
		static LightLock _l;
		MaterialManager() = delete;
		template <typename T>
		static shader *createMaterialInstance(FILE *matFile) {
			return new T(matFile);
		}
		static auto &getMaterialMap() {
			static std::unordered_map<std::string, shaderInstantiationFunc> map;
			return map;
		}

	  public:
		template <typename T>
			requires validshader<T>
		static bool registerMaterial(const char *name) {
			getMaterialMap()[name] = MaterialManager::createMaterialInstance<T>;
			return true;
		}
		static std::unique_ptr<shader> makeMaterial(const char *name,
													FILE *matF);
	};
} // namespace ql

#define MATERIAL_REGISTER(material)                                            \
	bool material##_material =                                                 \
		::ql::MaterialManager::registerMaterial<material>(#material);

