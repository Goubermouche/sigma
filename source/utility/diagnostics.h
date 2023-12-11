#pragma once
#include "utility/types.h"

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
	private:
		template<typename... arguments>
		static auto format_str(
			std::format_string<arguments...> fmt, arguments&&... args
		) -> std::string {
			return std::move(std::format(fmt, std::forward<arguments>(args)...));
		}
	};
} // namespace utility
