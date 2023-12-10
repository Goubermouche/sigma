#pragma once

namespace sigma::ir {
	using namespace utility::types;

	struct node;
	struct function;

	struct region {
		handle<node> memory_in;
		handle<node> memory_out;
	};

	struct branch {
		std::vector<handle<node>> successors;
		std::vector<i64> keys;
	};

	struct projection {
		u64 index = 0;
	};

	struct function_call {
		function_signature signature; // function prototype
		std::vector<handle<node>> projections;
	};
}
