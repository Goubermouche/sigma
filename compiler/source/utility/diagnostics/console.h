#pragma once
#include "utility/types.h"

#define NOMINMAX
#include <Windows.h>

namespace sigma {
	struct color_value {
		WORD value;
	};

	class color {
	public:
		static constexpr color_value white = { 7 };
		static constexpr color_value pink = { 2 }; // 
		static constexpr color_value magenta = { 3 };
		static constexpr color_value blue = { 1 };
		static constexpr color_value light_blue = { 9 };
		static constexpr color_value green = { 2 };
		static constexpr color_value light_green = { 10 };
		static constexpr color_value orange = { 14 }; // 
		static constexpr color_value yellow = { 6 };
		static constexpr color_value red = { 4 };
	};

	class console {
	public:
		static void init();
		static console& out;

		console& operator<<(const color_value& color);
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

		template<typename type>
		console& operator<<(std::shared_ptr<type> value);

		// template <typename T>
		// console& operator<<(const T& value);
	private:
		console() = default;
		static void set_color(const color_value& color);
	};

	template<typename type>
	console& console::operator<<(std::shared_ptr<type> value) {
		if (value) // check that the shared_ptr isn't null
			return *this << *value; // call the << operator for error_message
		else
			return *this << "null";
	}

	// template<typename T>
	// console& console::operator<<(const T& value) {
	// 	std::wcout << value;
	// 	return *this;
	// }

	inline COLORREF hex_to_rgb(const std::string& hex) {
		if (hex.size() != 6) {
			throw std::invalid_argument("invalid hex color string");
		}

		i32 red, green, blue;
		sscanf_s(hex.c_str(), "%02x%02x%02x", &red, &green, &blue);
		return RGB(red, green, blue);
	}

	/**
	 * \brief Creates a backslash-escaped version of the \a input string. 
	 * \param input String to escape.
	 * \return Escaped version of the given string. 
	 */
	inline std::string escape_string(const std::string& input) {
		std::string output;
		for (const char ch : input) {
			if (ch == '\\' || ch == '\'' || ch == '\"' || ch == '\a' || ch == '\b' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' || ch == '\x1b') {
				output.push_back('\\');
				switch (ch) {
				case '\\': output.push_back('\\'); break;
				case '\'': output.push_back('\''); break;
				case '\"': output.push_back('\"'); break;
				case '\a': output.push_back('a'); break;
				case '\b': output.push_back('b'); break;
				case '\f': output.push_back('f'); break;
				case '\n': output.push_back('n'); break;
				case '\r': output.push_back('r'); break;
				case '\t': output.push_back('t'); break;
				case '\v': output.push_back('v'); break;
				case '\x1b':
					output.append("x1b");
					break;
				}
			}
			else {
				output.push_back(ch);
			}
		}
		return output;
	}
}
