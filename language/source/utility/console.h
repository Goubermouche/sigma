#pragma once
#include "macros.h"

namespace channel {
    enum class color {
        red = 31,
        green = 32,
        yellow = 33,
        blue = 34,
        magenta = 35,
        cyan = 36,
        white = 37,
        light_red = 91,
        light_green = 92,
        light_yellow = 93,
        light_blue = 94,
        light_magenta = 95,
        light_cyan = 96,
        light_white = 97,
        bg_red = 41,
        bg_green = 42,
        bg_yellow = 43,
        bg_blue = 44,
        bg_magenta = 45,
        bg_cyan = 46,
        bg_white = 47,
        bg_light_red = 101,
        bg_light_green = 102,
        bg_light_yellow = 103,
        bg_light_blue = 104,
        bg_light_magenta = 105,
        bg_light_cyan = 106,
        bg_light_white = 107,
        reset = 0
	};

    class console {
    public:
        static std::string color(color color, const std::string& message) {
            return "\033[" + std::to_string(static_cast<i32>(color)) + "m" + message + "\033[0m";
        }

        static void log(const std::string& message) {
            std::cout << message << '\n';
        }
    };
}