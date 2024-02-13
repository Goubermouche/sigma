#pragma once
#include <utility/macros.h>

namespace sigma::ir {
	using namespace utility::types;

	struct local {
		u32 size;        // in bytes
		u32 alignment;   // in bytes
	};

	struct memory_access {
		u32 alignment; // in bytes
	};

	struct array {
		i64 stride;
	};

	struct member {
		i64 offset;
	};
}
