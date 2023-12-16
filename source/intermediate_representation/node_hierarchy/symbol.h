#pragma once
#include <utility/types.h>
#include <utility/containers/handle.h>

namespace sigma::ir {
	using namespace utility::types;

	class module;
	struct symbol {
		enum symbol_tag {
			NONE,

			// symbol is dead
			TOMBSTONE,

			EXTERNAL,
			GLOBAL,
			FUNCTION,

			MAX
		};

		auto is_non_local() const -> bool;

		handle<module> parent_module;
		std::string name;
		symbol_tag tag;
		u64 symbol_id;
		u64 ordinal = 0;
	};
}
