#pragma once

#include <array>
#include <bitset>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>

// platform specific 
#ifdef _WIN32
#define NOMINMAX
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#endif

namespace utility {
	namespace types {
		// signed integers
		using i8 = int8_t;
		using i16 = int16_t;
		using i32 = int32_t;
		using i64 = int64_t;

		// unsigned integers
		using u8 = uint8_t;
		using u16 = uint16_t;
		using u32 = uint32_t;
		using u64 = uint64_t;

		// floating point
		using f32 = float;
		using f64 = double;

		using ptr_diff = ptrdiff_t;

		template<typename type>
		using s_ptr = std::shared_ptr<type>;

		template<typename type>
		using u_ptr = std::unique_ptr<type>;
	} // namespace utility::types

	using namespace types;
} // namespace utility
