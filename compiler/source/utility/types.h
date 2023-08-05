#pragma once

#include <array>
#include <cstddef>
#include <codecvt>
#include <deque>
#include <expected>
#include <filesystem>
#include <fstream>
#include <future>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <regex>
#include <locale>
#include <numeric>

// platform specific 
#ifdef _WIN32
#define NOMINMAX
    #include <io.h>
	#include <fcntl.h>

#else
    #include <unistd.h>
#endif

namespace sigma {
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

		using filepath = std::filesystem::path;
	}

	using namespace types;
}
