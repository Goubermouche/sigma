#pragma once
#include "types.h"

#define NOMINMAX
#include <Windows.h>

namespace channel {
    class color {
    public:
        static constexpr WORD red = FOREGROUND_RED;
        static constexpr WORD green = FOREGROUND_GREEN;
        static constexpr WORD yellow = FOREGROUND_RED | FOREGROUND_GREEN;
        static constexpr WORD blue = FOREGROUND_BLUE;
        static constexpr WORD magenta = FOREGROUND_RED | FOREGROUND_BLUE;
        static constexpr WORD cyan = FOREGROUND_GREEN | FOREGROUND_BLUE;
        static constexpr WORD white = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        static constexpr WORD light_red = FOREGROUND_RED | FOREGROUND_INTENSITY;
        static constexpr WORD light_green = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        static constexpr WORD light_yellow = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        static constexpr WORD light_blue = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        static constexpr WORD light_magenta = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        static constexpr WORD light_cyan = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        static constexpr WORD light_white = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    };

    class console {
    public:
        static void init();
        static console& out;

        console& operator<<(WORD color);
        console& operator<<(const std::string& value);

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