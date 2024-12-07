#pragma once 

#include <fstream>
#include <string>
#include "aligned_uptr.h"

namespace ql {
	auto readFileAligned(const std::string &filename) {
		std::ifstream in(filename, std::ios::in | std::ios::binary);
		if (!in) {
			auto r = aligned_uptr<char>(alignof(char), 0);
			r.reset(nullptr);
			return r;
		}
		// only works with c++11 or higher, lower versions don't guarantee contiguous string data
		unsigned long size;
		in.seekg(0, std::ios::end);
		size   = in.tellg();
		auto c = aligned_uptr<char>(0x1000, size);
		in.seekg(0, std::ios::beg);
		in.read(c.get(), size);
		in.close();
		return c;
	}
}