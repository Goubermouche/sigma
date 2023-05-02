#include "console.h"

namespace channel {
	console& console::out = *new console();

	void console::init() {
		_setmode(_fileno(stdout), _O_U16TEXT);

		//HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		//CONSOLE_SCREEN_BUFFER_INFOEX cInfo;
		//cInfo.cbSize = sizeof(cInfo);

		//// Get the current console screen buffer information
		//if (!GetConsoleScreenBufferInfoEx(hOutput, &cInfo)) {
		//	return;
		//}

		//COLORREF customPalette[16] = {
		//	 RGB(0, 0, 0),       RGB(34, 139, 34),   RGB(255, 99, 71),   RGB(218, 165, 32),
		//	 RGB(95, 158, 160),  RGB(30, 144, 255),  RGB(255, 105, 180), RGB(192, 192, 192),
		//	 RGB(128, 128, 128), RGB(0, 206, 209),   RGB(154, 205, 50),  RGB(255, 255, 0),
		//	 RGB(255, 160, 122), RGB(199, 21, 133),  RGB(135, 206, 235), RGB(255, 255, 255)
		//};

		//// Set the new color palette
		//for (int i = 0; i < 16; i++) {
		//	cInfo.ColorTable[i] = customPalette[i];
		//}

		//// Update the console screen buffer with the new palette
		//SetConsoleScreenBufferInfoEx(hOutput, &cInfo);
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