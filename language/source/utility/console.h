#pragma once
#include "macros.h"

#define NOMINMAX
#include <Windows.h>

namespace channel {
    enum class color {
        red = FOREGROUND_RED,
        green = FOREGROUND_GREEN,
        yellow = FOREGROUND_RED | FOREGROUND_GREEN,
        blue = FOREGROUND_BLUE,
        magenta = FOREGROUND_RED | FOREGROUND_BLUE,
        cyan = FOREGROUND_GREEN | FOREGROUND_BLUE,
        white = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        light_red = FOREGROUND_RED | FOREGROUND_INTENSITY,
        light_green = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        light_yellow = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        light_blue = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        light_magenta = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        light_cyan = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        light_white = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
	};

    class console {
    public:
        static void log(color color, const std::string& message) {
            set_color(color);
            std::cout << message;
            set_color(color::white);
        }
    private:
        static void set_color(color color) {
            const HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(console_handle, static_cast<WORD>(color));
        }
    };
}