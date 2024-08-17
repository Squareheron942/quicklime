#pragma once

#include <array>
#include <cstddef>

template <size_t S	 = 0, std::invocable<double> F,
		  typename R = std::array<std::invoke_result_t<F, double>, S>>
consteval R generateTable(F &&func) {
	static_assert(S > 0, "LUT size must be greater than zero");
	R table{};

	for (size_t i = 0; i < S; i++)
		table[i] = func(static_cast<double>(i) / S);

	return table;
}