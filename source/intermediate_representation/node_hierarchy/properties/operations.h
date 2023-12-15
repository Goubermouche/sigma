#pragma once
#include <utility/macros.h>

namespace sigma::ir {
	using namespace utility::types;

	enum class arithmetic_behaviour {
		none             = 0,
		no_signed_wrap   = 1,
		no_unsigned_wrap = 2
	};

	// declare as a flag enum
	FLAG_ENUM(arithmetic_behaviour);

	struct binary_integer_op {
		arithmetic_behaviour behaviour = arithmetic_behaviour::none;
	};
}
