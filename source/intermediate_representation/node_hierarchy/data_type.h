#pragma once
#include "intermediate_representation/node_hierarchy/properties/types.h"

namespace ir {
	using namespace utility::types;

	struct data_type {
		enum type : u8 {
			integer,        // void is an i0 and bool is an i1
			floating_point, // f{32, 64}
			pointer,        // pointer{0-2047}
			tuple,          // tuples, can only be accessed by projections
			control,        // represents control flow for regions and branches
			memory,         // represents memory and I/O
			continuation    // continuation (usually just the return addresses)
		};

		auto to_string() const -> std::string;
		auto operator==(const data_type& other) const -> bool;

		type ty;
		u8 bit_width;
	};
}

// integral types
#define VOID_TYPE ir::data_type(ir::data_type::integer, 0)
#define BOOL_TYPE ir::data_type(ir::data_type::integer, 1)
#define PTR_TYPE  ir::data_type(ir::data_type::pointer, 0)
#define I8_TYPE   ir::data_type(ir::data_type::integer, 8)
#define I16_TYPE  ir::data_type(ir::data_type::integer, 16)
#define I32_TYPE  ir::data_type(ir::data_type::integer, 32)
#define I64_TYPE  ir::data_type(ir::data_type::integer, 64)

// floating point types
#define F32_TYPE ir::data_type(ir::data_type::floating_point, static_cast<u8>(float_format::f32))
#define F64_TYPE ir::data_type(ir::data_type::floating_point, static_cast<u8>(float_format::f64))

// misc types
#define CONTROL_TYPE      ir::data_type(ir::data_type::control)
#define TUPLE_TYPE        ir::data_type(ir::data_type::tuple)
#define MEMORY_TYPE       ir::data_type(ir::data_type::memory)
#define CONTINUATION_TYPE ir::data_type(ir::data_type::memory)
