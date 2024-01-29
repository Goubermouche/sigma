#pragma once
#include <utility/macros.h>

namespace sigma::ir {
	using namespace utility::types;

	enum class arithmetic_behaviour {
		NONE             = 0,
		NO_SIGNED_WRAP   = 1,
		NO_UNSIGNED_WRAP = 2
	};

	// declare as a flag enum
	FLAG_ENUM(arithmetic_behaviour);

	struct binary_integer_op {
		arithmetic_behaviour behaviour = arithmetic_behaviour::NONE;
	};

	struct compare_op {
		data_type cmp_dt;
	};
}
