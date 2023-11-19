#pragma once
#include "intermediate_representation/node_hierarchy/function.h"
#include "intermediate_representation/codegen/work_list.h"

namespace ir {
	struct transformation_context {
		handle<function> function;
		handle<work_list> work_list;

		// locals 
		std::vector<handle<node>> locals;
	};
}
