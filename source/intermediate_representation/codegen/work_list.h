#pragma once
#include "intermediate_representation/codegen/control_flow_graph.h"

namespace sigma::ir {
	struct work_list {
		auto mark_next_control(handle<node> target) -> handle<node>;
		auto visit(handle<node> node) -> bool;

		void compute_dominators(control_flow_graph& cfg) const;
		void push_all(handle<function> function);
		void clear();

		std::unordered_set<handle<node>> visited_items;
		std::vector<handle<node>> items;
	};
}// namespace sigma::ir
