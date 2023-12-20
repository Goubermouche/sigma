#pragma once
#include "intermediate_representation/node_hierarchy/function.h"
#include "intermediate_representation/codegen/work_list.h"

namespace sigma::ir {
	struct transformation_context {
		handle<function> function;
		work_list& work;

		// locals 
		std::vector<handle<node>> locals;
	};
}
