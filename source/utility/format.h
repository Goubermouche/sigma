#pragma once
#include "types.h"

namespace utility::detail {
	template<typename type>
	struct formatter {
		static std::string format(const type& value) {
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}
	};

	template<typename tuple, u64... is>
	std::string format_argument(
		uint64_t index,
		tuple&& arguments,
		std::index_sequence<is...>
	) {
		std::string result;
		const bool found = ((is == index &&
			(result = formatter<std::remove_reference_t<std::tuple_element_t<is, std::remove_reference_t<tuple>>>>::format(std::get<is>(std::forward<tuple>(arguments))), true)) || ...);

		if (!found) {
			throw std::runtime_error("format argument index out of range.");
		}

		return result;
	}

	template<typename... argument_types>
	std::string format(
		const std::string& template_str, argument_types... args
	) {
		std::ostringstream output;
		u64 pos = 0;
		u64 previous_pos = 0;
		std::tuple<argument_types...> arguments(args...);

		uint64_t index = 0;
		while ((pos = template_str.find("{}", pos)) != std::string::npos) {
			output << template_str.substr(previous_pos, pos - previous_pos);
			output << format_argument(index, arguments, std::index_sequence_for<argument_types...>{});
			pos += 2; // skip '{}'
			previous_pos = pos;
			++index;
		}

		output << template_str.substr(previous_pos); // append rest of the string
		return output.str();
	}
}
