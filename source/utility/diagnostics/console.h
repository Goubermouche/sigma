#pragma once
#include "utility/types.h"

namespace utility {
	struct color_value {
		const char* ansi_code;
	};

	struct left_pad {};

	class color {
	public:
		static constexpr color_value white = { "\033[38;5;231m" };
		static constexpr color_value magenta = { "\033[38;5;134m" };
		static constexpr color_value blue = { "\033[38;5;33m" };
		static constexpr color_value green = { "\033[38;5;41m" };
		static constexpr color_value yellow = { "\033[38;5;228m" };
		static constexpr color_value red = { "\033[38;5;197m" };
	};

	class console {
	public:
		static constexpr left_pad left = {};

		static void init();
		static console& out;
		static console& width(i64 width);
		static console& precision(i64 precision);

		console& operator<<(const color_value& color);
		console& operator<<(const left_pad& left_pad);
		console& operator<<(const console& console);

		console& operator<<(const std::string& value);
		console& operator<<(const filepath& value);

		console& operator<<(const char* value);
		console& operator<<(char value);

		console& operator<<(f32 value);
		console& operator<<(f64 value);

		console& operator<<(u8 value);
		console& operator<<(u16 value);
		console& operator<<(u32 value);
		console& operator<<(u64 value);

		console& operator<<(i8 value);
		console& operator<<(i16 value);
		console& operator<<(i32 value);
		console& operator<<(i64 value);
	private:
		console() = default;
	};
}
