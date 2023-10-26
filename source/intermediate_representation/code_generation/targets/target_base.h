#pragma once
#include "intermediate_representation/code_buffer.h"
#include "intermediate_representation/nodes/data_type.h"

namespace ir::cg {
	class code_generator_context;

	class target_base {
	public:
		virtual void emit_code(
			code_generator_context& context,
			code_buffer& buffer
		) = 0;

		virtual void allocate_base_registers(
			code_generator_context& context
		) = 0;

		virtual i32 legalize_data_type(const data_type& data_type) = 0;
		virtual i32 classify_register_class(const data_type& data_type) = 0;
	};
}