#pragma once
#include <utility/macros.h>

namespace ir {
	using namespace utility::types;

	struct local_property {
		u32 size;        // in bytes
		u32 alignment;   // in bytes
		i32 alias_index;
	};

	struct memory_access_property {
		u32 alignment; // in bytes
	};
}