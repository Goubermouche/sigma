#pragma once
#include "utility/types.h"

#define NOMINMAX
#include <Windows.h>

namespace sigma {
	class color {
	public:
		static constexpr WORD white = 7;
		static constexpr WORD pink = 2; // 
		static constexpr WORD magenta = 3;
		static constexpr WORD blue = 1;
		static constexpr WORD light_blue = 9;
		static constexpr WORD green = 2;
		static constexpr WORD light_green = 10;
		static constexpr WORD orange = 14; // 
		static constexpr WORD yellow = 6;
		static constexpr WORD red = 4;
	};

	class console {
	public:
		static void init();
		static console& out;

		console& operator<<(WORD color);
		console& operator<<(const std::string& value);
		console& operator<<(const filepath& value);

		template <typename T>
		console& operator<<(const T& value);
	private:
		console() = default;
		static void set_color(WORD color);
	};

	template<typename T>
	console& console::operator<<(const T& value) {
		std::wcout << value;
		return *this;
	}

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
