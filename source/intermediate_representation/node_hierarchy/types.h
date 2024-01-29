#pragma once
#include "intermediate_representation/node_hierarchy/properties/types.h"

namespace sigma::ir {
	using namespace utility::types;

	// TODO: rework the IR type system

	struct data_type {
		struct base {
			enum underlying : u8 {
				INTEGER,      // void is an i0 and bool is an i1
				FLOAT,        // f{32, 64}
				POINTER,      // pointer{0-2047}
				TUPLE,        // tuples, can only be accessed by projections
				CONTROL,      // represents control flow for regions and branches
				MEMORY,       // represents memory and I/O
				CONTINUATION, // continuation (usually just the return addresses)
				UNKNOWN
			};

			base();
			base(underlying type);

			operator underlying() const;
			auto get_underlying() const -> underlying;
		private:
			underlying m_type;
		};

		data_type();
		data_type(base base, u8 bit_width = 0);

		auto operator==(const data_type& other) const -> bool;

		auto to_string() const->std::string;

		auto is_floating_point() const -> bool;
		auto is_pointer() const -> bool;

		auto get_base() const -> base;
		auto get_bit_width() const -> u8;
		void set_bit_width(u8 bit_width);
	private:
		base m_base;
		u8 m_bit_width;
	};

	bool operator==(data_type type_a, data_type::base::underlying type_b);

	struct function_signature {
		std::string identifier;

		std::vector<data_type> parameters;
		std::vector<data_type> returns;

		bool has_var_args = false;
	};
} // namespace sigma::ir

// integral types
#define VOID_TYPE sigma::ir::data_type(sigma::ir::data_type::base::INTEGER, 0)
#define BOOL_TYPE sigma::ir::data_type(sigma::ir::data_type::base::INTEGER, 1)
#define PTR_TYPE  sigma::ir::data_type(sigma::ir::data_type::base::POINTER, 0)
#define I8_TYPE   sigma::ir::data_type(sigma::ir::data_type::base::INTEGER, 8)
#define I16_TYPE  sigma::ir::data_type(sigma::ir::data_type::base::INTEGER, 16)
#define I32_TYPE  sigma::ir::data_type(sigma::ir::data_type::base::INTEGER, 32)
#define I64_TYPE  sigma::ir::data_type(sigma::ir::data_type::base::INTEGER, 64)

// floating point types
#define F32_TYPE sigma::ir::data_type(          \
	sigma::ir::data_type::base::FLOAT,            \
	static_cast<u8>(sigma::ir::float_format::f32) \
)
#define F64_TYPE sigma::ir::data_type(          \
	sigma::ir::data_type::base::FLOAT,            \
	static_cast<u8>(sigma::ir::float_format::f64) \
)

// misc types
#define CONTROL_TYPE      sigma::ir::data_type(sigma::ir::data_type::base::CONTROL)
#define TUPLE_TYPE        sigma::ir::data_type(sigma::ir::data_type::base::TUPLE)
#define MEMORY_TYPE       sigma::ir::data_type(sigma::ir::data_type::base::MEMORY)
#define CONTINUATION_TYPE sigma::ir::data_type(sigma::ir::data_type::base::MEMORY)
