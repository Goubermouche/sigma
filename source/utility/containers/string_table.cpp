#include "string_table.h"
#include "utility/macros.h"

namespace utility {
	string_table_key::string_table_key() : m_value(0) {}
	string_table_key::string_table_key(const std::string& string)
		: m_value(std::hash<std::string>{}(string)) {}

	auto string_table_key::operator==(string_table_key other) const -> bool {
		return m_value == other.m_value;
	}

	auto string_table_key::get_value() const -> u64 {
		return m_value;
	}

	auto string_table_key::is_valid() const -> bool {
		return m_value != 0;
	}

	bool string_table::contains(string_table_key key) const {
		return m_key_to_string.contains(key);
	}

	auto string_table::insert(const std::string& string) -> string_table_key {
		const string_table_key new_key(string);

		const auto it = m_key_to_string.find(new_key);
		if (it != m_key_to_string.end()) {
			// the key is already contained in the table
			return new_key;
		}

		m_key_to_string[new_key] = string;
		return new_key;
	}

	auto string_table::get(string_table_key key) const -> const std::string& {
		ASSERT(key.get_value() != 0, "invalid symbol key");
		return m_key_to_string.at(key);
	}
} // namespace utility
