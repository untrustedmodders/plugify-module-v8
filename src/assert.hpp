#pragma once

#include <source_location>

#define ASSERT(cond)                                                                                                                                                            \
	if (!(cond)) [[unlikely]] {                                                                                                                                                 \
		constexpr std::source_location location = std::source_location::current();                                                                                              \
		std::puts(std::format("Assertion failed on {} ({}:{}) `{}`: " #cond "\n", location.file_name(), location.line(), location.column(), location.function_name()).c_str()); \
		std::terminate();                                                                                                                                                       \
	}

#define UNUSED(func) [[maybe_unused]] auto _ = func
