#include "string_table.h"
#include "utility/macros.h"

namespace utility {
	auto string_table_key::create_key() -> string_table_key {
		static std::random_device s_random_device;
		static std::mt19937 s_random_engine(s_random_device());
		static std::uniform_int_distribution<uint32_t> s_uniform_distribution;

		string_table_key key;
		key.m_value = s_uniform_distribution(s_random_engine);
		return key;
	}

	auto string_table_key::operator==(string_table_key other) const -> bool {
		return m_value == other.m_value;
	}

	auto string_table_key::get_value() const -> u32 {
		return m_value;
	}

	auto string_table_key::is_valid() const -> bool {
		return m_value != 0;
	}

	bool string_table::contains(string_table_key key) const {
		return m_key_to_string.contains(key);
	}

	auto string_table::insert(const std::string& symbol) -> string_table_key {
		const auto str_to_key = m_string_to_key.find(symbol);

		if (str_to_key != m_string_to_key.end()) {
			return str_to_key->second;
		}

		const auto key = string_table_key::create_key();
		m_key_to_string[key] = symbol;
		m_string_to_key[symbol] = key;
		return key;
	}

	auto string_table::get(string_table_key key) const -> const std::string& {
		ASSERT(key.get_value() != 0, "invalid symbol key");
		return m_key_to_string.at(key);
	}
} // namespace utility