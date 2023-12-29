#pragma once

#include "utility/types.h"

namespace utility {
	struct string_table_key {
		string_table_key();
		string_table_key(const std::string& string);

		auto operator==(string_table_key other) const -> bool;

		auto get_value() const -> u64;
		auto is_valid() const -> bool;
	private:
		u64 m_value; // already hashed
	};
} // namespace utility

template <>
struct std::hash<utility::string_table_key> {
	auto operator()(const utility::string_table_key& k) const noexcept -> utility::u64 {
		// since we've already hashed the string in the constructor we can use the value
		return k.get_value();
	}
};

namespace utility {
	class string_table {
	public:
		string_table() = default;

		bool contains(string_table_key key) const;
		auto insert(const std::string& string) -> string_table_key;
		auto get(string_table_key key) const -> const std::string&;
	private:
		std::unordered_map<string_table_key, std::string> m_key_to_string;
	};
} // namespace utility
