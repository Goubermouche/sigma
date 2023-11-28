#pragma once
#include <utility/types.h>
#include <utility/containers/handle.h>

namespace ir {
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

		handle<module> module;
		std::string name;
		symbol_tag tag;
		u64 symbol_id;
		u64 ordinal;
	};
}
