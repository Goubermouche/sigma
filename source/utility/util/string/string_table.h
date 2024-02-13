#pragma once
#include "../macros.h"

namespace utility {
	struct string_table_key {
		string_table_key() : m_value(0) {}
		string_table_key(const std::string& string) : m_value(std::hash<std::string>{}(string)) {}

		auto operator==(string_table_key other) const -> bool {
			return m_value == other.m_value;
		}

		auto operator<(string_table_key other) const -> bool {
			return m_value < other.m_value;
		}

		auto get_value() const -> u64 {
			return m_value;
		}

		auto is_valid() const -> bool {
			return m_value != 0;
		}
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

		bool contains(string_table_key key) const {
			return m_key_to_string.contains(key);
		}

		auto insert(const std::string& string) -> string_table_key {
			const string_table_key new_key(string);

			const auto it = m_key_to_string.find(new_key);
			if (it != m_key_to_string.end()) {
				// the key is already contained in the table
				return new_key;
			}

			m_key_to_string[new_key] = string;
			return new_key;
		}

		auto get(string_table_key key) const -> const std::string& {
			ASSERT(key.get_value() != 0, "invalid symbol key");
			return m_key_to_string.at(key);
		}
	private:
		std::unordered_map<string_table_key, std::string> m_key_to_string;
	};
} // namespace utility
