#pragma once

#include <array>
#include <cstddef>
#include <codecvt>
#include <deque>
#include <expected>
#include <filesystem>
#include <fcntl.h>
#include <format>
#include <fstream>
#include <future>
#include <functional>
#include <iomanip>
#include <io.h>
#include <iostream>
#include <iterator>
#include <string>
#include <semaphore>
#include <unordered_map>
#include <utility>
#include <vector>
#include <regex>

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
		using f32 = float_t;
		using f64 = double_t;

		using filepath = std::filesystem::path;
	}

	using namespace types;
}
