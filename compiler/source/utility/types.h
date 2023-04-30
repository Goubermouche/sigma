#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <chrono>
#include <codecvt>
#include <io.h>
#include <fcntl.h>

namespace channel {
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

		static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> s_ws_to_s_converter;
	}

	using namespace types;
}
