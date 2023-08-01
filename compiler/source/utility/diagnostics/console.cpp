#include "console.h"

namespace sigma {
	console& console::out = *new console();

	void console::init() {
		_setmode(_fileno(stdout), _O_U16TEXT);
		const HANDLE h_output = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFOEX csbiInfo;
		csbiInfo.cbSize = sizeof(csbiInfo);
		GetConsoleScreenBufferInfoEx(h_output, &csbiInfo);

		//const std::string color_palette[11] = {
		//	"0c0c0c", // background
		//	"d0d0d0", // text
		//	"ed94c0", // numeric, boolean
		//	"c191ff", // name
		//	"6c95eb", // keyword
		//	"66c3cc", // member
		//	"39cc8f", // function
		//	"85c46c", // comment
		//	"c9a26d", // string literal
		//	"c9ba6d", // yellow
		//	"d43434", // red
		//};

		//for (u64 i = 0; i < 11; ++i) {
		//	csbiInfo.ColorTable[i] = hex_to_rgb(color_palette[i]);
		//}

		//SetConsoleScreenBufferInfoEx(h_output, &csbiInfo);
		//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color::white);
	}

	console& console::operator<<(const color_value& color) {
		set_color(color);
		return *this;
	}

	console& console::operator<<(const precision& precision) {
		std::wcout << std::fixed << std::setprecision(precision.get_precision());
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
		// int size_needed = MultiByteToWideChar(CP_UTF8, 0, value, -1, NULL, 0);
		// wchar_t* wstr = new wchar_t[size_needed];
		// MultiByteToWideChar(CP_UTF8, 0, value, -1, wstr, size_needed);
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

	void console::set_color(const color_value& color) {
		// SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color.value);
	}

	precision::precision(u64 precision)
		: m_precision(precision) {}

	u64 precision::get_precision() const {
		return m_precision;
	}
}
