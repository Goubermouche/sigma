#pragma once
#include "intermediate_representation/code_generation/code_generator_context.h"

namespace ir::cg {
	class allocator_base {
	public:
		virtual void allocate(code_generator_context& context) = 0;
	};
}

