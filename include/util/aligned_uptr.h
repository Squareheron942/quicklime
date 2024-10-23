#pragma once

namespace ql {
	template <class T>
	using unique_ptr_aligned = std::unique_ptr<T, decltype(&free)>;
	template <class T>
	unique_ptr_aligned<T> aligned_uptr(size_t align, size_t size) {
				return unique_ptr_aligned<T>(static_cast<T *>(aligned_alloc(align, size)),
											 &free);
	}
}