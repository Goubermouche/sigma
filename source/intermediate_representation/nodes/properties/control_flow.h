#pragma once

namespace ir {
	using namespace utility::types;

	struct empty_property {};

	class node;
	struct region_property {
		handle<node> end;
		handle<node> dominator;
		handle<node> memory_in;
		handle<node> memory_out;

		i32 dominator_depth = 0;
		i32 post_order_id = 0;
		std::string tag;
	};

	struct branch_property {
		std::vector<handle<node>> successors;
		std::vector<i64> keys;
	};

	struct projection_property {
		i64 index = 0;
	};
}
