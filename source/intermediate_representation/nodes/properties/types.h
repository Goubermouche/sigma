#pragma once
#include <utility/types.h>

namespace ir {
	using namespace utility::types;

	enum class float_format {
		f32,
		f64
	};

	struct integer_property {
		u64 value = 0;
		u8 bit_width = 0;
	};

	struct fp32_property {
		f32 value = 0.0f;
	};

	struct fp64_property {
		f64 value = 0.0;
	};
}
