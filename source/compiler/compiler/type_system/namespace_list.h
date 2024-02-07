#pragma once
#include <utility/containers/string_table.h>

namespace sigma {
	using namespace utility::types;

	struct namespace_list {
		using base = std::vector<utility::string_table_key>;

		namespace_list() = default;
		namespace_list(const std::vector<utility::string_table_key>& namespaces);

		auto empty() const -> bool;
		auto size() const->u64;

		auto first() const->utility::string_table_key;
		auto last() const->utility::string_table_key;

		auto begin() -> base::iterator;
		auto end() -> base::iterator;

		auto begin() const -> base::const_iterator;
		auto end() const -> base::const_iterator;

		auto operator[](u64 index) const -> base::value_type;

		auto get_stream(const utility::string_table& strings) const->std::stringstream;

		base namespaces;
	};
} // namespace sigma