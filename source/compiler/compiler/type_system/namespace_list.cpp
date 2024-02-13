#include "namespace_list.h"

namespace sigma {
	namespace_list::namespace_list(const utility::memory_view<utility::string_table_key>& namespaces)
		: namespaces(namespaces) {}

	auto namespace_list::empty() const -> bool {
		return namespaces.is_empty();
	}

	auto namespace_list::size() const -> u64 {
		return namespaces.get_size();
	}

	auto namespace_list::first() const -> utility::string_table_key {
		return namespaces.first();
	}

	auto namespace_list::last() const -> utility::string_table_key {
		return namespaces.last();
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

	auto namespace_list::operator[](u64 index) const -> utility::string_table_key {
		return namespaces[index];
	}

  auto namespace_list::operator==(const namespace_list& other) const -> bool {
		return namespaces == other.namespaces;
  }

	auto namespace_list::get_stream(const utility::string_table& strings) const -> std::stringstream {
		std::stringstream namespace_str;

		for (u64 i = 0; i < namespaces.get_size(); ++i) {
			namespace_str << strings.get(namespaces[i]);
			if (i + 1 < namespaces.get_size()) {
				namespace_str << "::";
			}
		}

		return namespace_str;
	}
} // namespace sigma
