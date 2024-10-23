#pragma once

#include <citro3d.h>
#include <string>
#include <util/console.h>

namespace ql {
	class Texture {
	  public:
		std::string name;
		C3D_Tex tex;
		C3D_TexCube cube;
		Texture(std::string name) : name(name) {}
		~Texture() {}
	};
} // namespace ql