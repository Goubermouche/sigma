#include "console.h"

namespace channel {
	console& console::out = *new console();

	void console::init() {
		_setmode(_fileno(stdout), _O_U16TEXT);
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