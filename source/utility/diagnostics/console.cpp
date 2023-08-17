#include "console.h"
#include "macros.h"

namespace sigma {
	console& console::out = *new console();

	void console::init() {
#ifdef _WIN32
		_setmode(_fileno(stdout), _O_U16TEXT);
#else
		std::setlocale(LC_ALL, "");
		std::ios::sync_with_stdio(false);
		std::wcout.imbue(std::locale());
#endif
	}

	console& console::width(i64 width) {
		std::wcout << std::setw(width);
		return out;
	}

	console& console::precision(i64 precision) {
		std::wcout << std::fixed << std::setprecision(precision);
		return out;
	}

	console& console::operator<<(const color_value& color) {
		std::wcout << color.ansi_code;
		return *this;
	}

	console& console::operator<<(const left_pad& left_pad) {
		SUPPRESS_C4100(left_pad);
		std::wcout << std::left;
		return *this;
	}

	console& console::operator<<(const console& console) {
		SUPPRESS_C4100(console);
		return *this;
	}

	console& console::operator<<(const std::string& value) {
		const std::wstring wide_string(value.begin(), value.end());
		std::wcout << wide_string;
		return *this;
	}

	console& console::operator<<(const filepath& value) {
		std::wcout << value.wstring();
		return *this;
	}

	console& console::operator<<(const char* value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(char value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(f32 value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(f64 value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(u8 value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(u16 value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(u32 value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(u64 value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(i8 value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(i16 value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(i32 value) {
		std::wcout << value;
		return *this;
	}

	console& console::operator<<(i64 value) {
		std::wcout << value;
		return *this;
	}
}
