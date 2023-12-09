#pragma once

#include "utility/types.h"

namespace utility {
	struct symbol_table_key {
		static auto create_key() -> symbol_table_key;

		auto operator==(symbol_table_key other) const -> bool;

		auto get_value() const->u32;
		auto is_valid() const -> bool;
	private:
		u32 m_value = 0;
	};
} // namespace utility

template <>
struct std::hash<utility::symbol_table_key> {
	auto operator()(const utility::symbol_table_key& k) const -> utility::u64 {
		return std::hash<utility::types::u32>()(k.get_value());
	}
};

namespace utility {
	// TODO: implement a more memory efficient symbol table (don't forget to maintain key stability)
	// what we need:
	// -   fast lookup (main priority)
	// -   reasonably fast insertions
	// -   memory efficiency
	// -   stable keys (that is, keys need to be stable so that when we concatenate multiple symbol
	//     tables into one big symbol table (after parsing is done), the addresses remain valid and
	//     point to the correct strings)
	// -   thread safe behaviour
	// potential ideas:
	// -   some sort of in place allocator where the index offsets of strings would be the addresses

	class symbol_table {
	public:
		symbol_table() = default;

		auto insert(const std::string& symbol) -> symbol_table_key;
		auto get(symbol_table_key key) const -> const std::string&;
	private:
		std::unordered_map<std::string, symbol_table_key> m_string_to_key;
		std::unordered_map<symbol_table_key, std::string> m_key_to_string;
	};
} // namespace utility
