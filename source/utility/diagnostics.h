#pragma once
#include "utility/types.h"
#include "utility/macros.h"

#include <format>
#include <list>

namespace utility {
	class console {
	public:
		template<typename... arguments>
		static void print(std::format_string<arguments...> fmt, arguments&&... args) {
			std::cout << format_str(std::move(fmt), std::forward<arguments>(args)...);
		}

		template<typename... arguments>
		static void println(std::format_string<arguments...> fmt, arguments&&... args) {
			std::cout << format_str(std::move(fmt), std::forward<arguments>(args)...) << '\n';
		}

		static void flush() {
			std::cout << std::flush;
		}
	private:
		template<typename... arguments>
		static auto format_str(
			std::format_string<arguments...> fmt, arguments&&... args
		) -> std::string {
			return std::format(fmt, std::forward<arguments>(args)...);
		}
	};
} // namespace utility
