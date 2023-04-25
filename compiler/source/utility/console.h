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

    inline std::string escape_string(const std::string& input) {
        std::string output;
        for (const char ch : input) {
            if (ch == '\\' || ch == '\'' || ch == '\"' || ch == '\a' || ch == '\b' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v') {
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
                }
            }
            else {
                output.push_back(ch);
            }
        }
        return output;
    }
}