#include "namespace_list.h"

namespace sigma {
	namespace_list::namespace_list(const std::vector<utility::string_table_key>& namespaces)
		: namespaces(namespaces) {}

	auto namespace_list::empty() const -> bool {
		return namespaces.empty();
	}

	auto namespace_list::size() const -> u64 {
		return namespaces.size();
	}

	auto namespace_list::first() const -> utility::string_table_key {
		return namespaces.front();
	}

	auto namespace_list::last() const -> utility::string_table_key {
		return namespaces.back();
	}

	auto namespace_list::begin() -> base::iterator {
		return namespaces.begin();
	}

	auto namespace_list::end() -> base::iterator {
		return namespaces.end();
	}

	auto namespace_list::begin() const -> base::const_iterator {
		return namespaces.begin();
	}

	auto namespace_list::end() const -> base::const_iterator {
		return namespaces.end();
	}

	auto namespace_list::operator[](u64 index) const -> base::value_type {
		return namespaces[index];
	}

	auto namespace_list::get_stream(const utility::string_table& strings) const -> std::stringstream {
		std::stringstream namespace_str;

		for (u64 i = 0; i < namespaces.size(); ++i) {
			namespace_str << strings.get(namespaces[i]);
			if (i + 1 < namespaces.size()) {
				namespace_str << "::";
			}
		}

		return namespace_str;
	}
} // namespace sigma
