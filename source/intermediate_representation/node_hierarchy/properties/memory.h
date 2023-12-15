#pragma once
#include <utility/macros.h>

namespace sigma::ir {
	using namespace utility::types;

	struct local {
		u32 size;        // in bytes
		u32 alignment;   // in bytes
		i32 alias_index;
	};

	struct memory_access {
		u32 alignment; // in bytes
	};
}
