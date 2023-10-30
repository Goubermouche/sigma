#pragma once
#include <utility/containers/long_string.h>
#include <utility/containers/byte_buffer.h>

#include "intermediate_representation/nodes/data_type.h"

namespace ir::cg {
	struct code_generator_context;

	struct code_generation_result {
		utility::long_string assembly_output;
		utility::byte_buffer bytecode;
	};

	class target_base {
	public:
		virtual void emit_code(
			code_generator_context& context,
			s_ptr<cg::code_generation_result> result
		) = 0;

		virtual void allocate_base_registers(
			code_generator_context& context
		) = 0;

		virtual i32 legalize_data_type(const data_type& data_type) = 0;
		virtual i32 classify_register_class(const data_type& data_type) = 0;
	};
}
