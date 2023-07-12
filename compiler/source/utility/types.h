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

namespace sigma {
	namespace types {
		// signed
		using i8 = int8_t;
		using i16 = int16_t;
		using i32 = int32_t;
		using i64 = int64_t;

		// unsigned 
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
