#pragma once
#include "intermediate_representation/node_hierarchy/properties/types.h"

namespace sigma::ir {
	using namespace utility::types;

	struct data_type {
		enum type : u8 {
			INTEGER,     // void is an i0 and bool is an i1
			FLOAT,       // f{32, 64}
			POINTER,     // pointer{0-2047}
			TUPLE,       // tuples, can only be accessed by projections
			CONTROL,     // represents control flow for regions and branches
			MEMORY,      // represents memory and I/O
			CONTINUATION // continuation (usually just the return addresses)
		};

		auto to_string() const -> std::string;
		auto operator==(const data_type& other) const -> bool;

		type ty;
		u8 bit_width;
	};

	struct function_signature {
		std::string identifier;

		std::vector<data_type> parameters;
		std::vector<data_type> returns;

		bool has_var_args = false;
	};
} // namespace sigma::ir

// integral types
#define VOID_TYPE sigma::ir::data_type(sigma::ir::data_type::INTEGER, 0)
#define BOOL_TYPE sigma::ir::data_type(sigma::ir::data_type::INTEGER, 1)
#define PTR_TYPE  sigma::ir::data_type(sigma::ir::data_type::POINTER, 0)
#define I8_TYPE   sigma::ir::data_type(sigma::ir::data_type::INTEGER, 8)
#define I16_TYPE  sigma::ir::data_type(sigma::ir::data_type::INTEGER, 16)
#define I32_TYPE  sigma::ir::data_type(sigma::ir::data_type::INTEGER, 32)
#define I64_TYPE  sigma::ir::data_type(sigma::ir::data_type::INTEGER, 64)

// floating point types
#define F32_TYPE sigma::ir::data_type(sigma::ir::data_type::FLOAT, static_cast<u8>(sigma::ir::float_format::f32))
#define F64_TYPE sigma::ir::data_type(sigma::ir::data_type::FLOAT, static_cast<u8>(sigma::ir::float_format::f64))

// misc types
#define CONTROL_TYPE      sigma::ir::data_type(sigma::ir::data_type::CONTROL)
#define TUPLE_TYPE        sigma::ir::data_type(sigma::ir::data_type::TUPLE)
#define MEMORY_TYPE       sigma::ir::data_type(sigma::ir::data_type::MEMORY)
#define CONTINUATION_TYPE sigma::ir::data_type(sigma::ir::data_type::MEMORY)
