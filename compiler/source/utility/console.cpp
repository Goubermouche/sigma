#include "console.h"

namespace channel {
	console& console::out = *new console();

	void console::init() {
		_setmode(_fileno(stdout), _O_U16TEXT);
		const HANDLE h_output = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFOEX csbiInfo;
		csbiInfo.cbSize = sizeof(csbiInfo);
		GetConsoleScreenBufferInfoEx(h_output, &csbiInfo);

		const std::string color_palette[11] = {
			"0c0c0c", // background
			"d0d0d0", // text
			"ed94c0", // numeric, boolean
			"c191ff", // name
			"6c95eb", // keyword
			"66c3cc", // member
			"39cc8f", // function
			"85c46c", // comment
			"c9a26d", // string literal
			"c9ba6d", // yellow
			"d43434", // red
		};

		// "0c0c0c" // background
		// "d0d0d0"	// text
		// "ed94c0"	// numeric, boolean
		// "c191ff"	// name
		// "6c95eb"	// keyword
		// "66c3cc"	// member
		// "39cc8f"	// function
		// "85c46c"	// comment
		// "c9a26d"	// string literal
		// "d43434"	// red

		for (int i = 0; i < 11; ++i) {
			csbiInfo.ColorTable[i] = hex_to_rgb(color_palette[i]);
		}

		SetConsoleScreenBufferInfoEx(h_output, &csbiInfo);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color::white);
	}

	console& console::operator<<(WORD color) {
		set_color(color);
		return *this;
	}

	console& console::operator<<(const std::string& value) {
		const std::wstring wide_string(value.begin(), value.end());
		std::wcout << wide_string;
		return *this;
	}

	void console::set_color(WORD color) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	}
}