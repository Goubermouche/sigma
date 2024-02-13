#pragma once
#include <utility/handle.h>
#include <utility/types.h>

namespace sigma::ir {
	using namespace utility::types;

	enum class linkage : u8 {
		PUBLIC,
		PRIVATE,
		SO_LOCAL, // exports to the rest of the shared object
		SO_EXPORT // exports outside of the shared object
	};

	class module;
	struct symbol {
		enum symbol_type {
			NONE,

			// symbol is dead
			TOMBSTONE,

			EXTERNAL,
			GLOBAL,
			FUNCTION,

			MAX
		};

		symbol() = default;
		symbol(symbol_type type, const std::string& name, handle<module> parent_module, linkage linkage);
		auto is_non_local() const -> bool;

		symbol_type type;
		std::string name;

		handle<module> parent_module; // not used rn

		linkage link;

		u64 ordinal = 0;
		u64 id = 0;
	};
}
