#pragma once
#include "intermediate_representation/nodes/properties/types.h"

namespace ir {
	using namespace utility::types;

	class data_type {
	public:
		enum id : u8 {
			integer,        // void is an i0 and bool is an i1
			floating_point, // f{32, 64}
			pointer,        // pointer{0-2047}
			tuple,          // tuples, can only be accessed by projections
			control,        // represents control flow for regions and branches
			memory,         // represents memory and I/O
			continuation    // continuation (usually just the return addresses)
		};

		data_type() = default;
		data_type(id type, u8 bit_width);
		data_type(id type);

		std::string to_string();

		void set_id(id id);
		id get_id() const;

		void set_bit_width(u8 bit_width);
		u8 get_bit_width() const;

		bool operator==(const data_type& other) const;
	private:
		id m_id;
		u8 m_bit_width;
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
