#pragma once
#include <utility/containers/handle.h>

namespace sigma::ir {
	using namespace utility::types;

	enum class float_format {
		F32,
		F64
	};

	struct integer {
		u64 value = 0;
		u8 bit_width = 0;
	};

	struct floating_point_32 {
		f32 value = 0.0f;
	};

	struct floating_point_64 {
		f64 value = 0.0;
	};
}
