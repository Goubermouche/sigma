#pragma once
#include "intermediate_representation/codegen/live_interval.h"

namespace sigma::ir {
	struct codegen_context;

	struct control_flow_graph {
		auto get_immediate_dominator(handle<node> target) -> handle<node>;
		auto get_predecessor(handle<node> target, u64 i) -> handle<node>;

		auto try_get_traversal_index(handle<node> target) -> u64;
		auto get_traversal_index(handle<node> target) const ->u64;
		auto resolve_dominator_depth(handle<node> basic_block) -> i32;
		auto get_dominator_depth(handle<node> target) const -> i32;

		static auto compute_reverse_post_order(const codegen_context& context) -> control_flow_graph;

		std::unordered_map<handle<node>, basic_block> blocks;
	};
}
