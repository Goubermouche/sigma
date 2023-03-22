#pragma once

namespace language {
	enum class token {
		end_of_file = -1,

		function = -2,

		// types
		// signed
		var_i8 = -3,
		var_i16 = -4,
		var_i32 = -5,
		var_i64 = -6,

		// unsigned
		var_u8 = -7,
		var_u16 = -8,
		var_u32 = -9,
		var_u64 = -10,

		// floating point
		var_f32 = -11,
		var_f64 = -12,

		// char
		var_char = -13,  // signed char
		var_uchar = -14, // unsigned char

		// expressions
		// etc
	};
}
