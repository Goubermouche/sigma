#include "control_flow_graph.h"
#include "intermediate_representation/codegen/codegen_context.h"

namespace sigma::ir {
	auto control_flow_graph::get_immediate_dominator(handle<node> target) -> handle<node> {
		const auto it = blocks.find(target);
		if (it == blocks.end()) {
			return nullptr;
		}

		const handle<basic_block> dom = it->second.dominator;
		return dom ? dom->start : nullptr;
	}

	auto control_flow_graph::get_predecessor(handle<node> target, u64 index) -> handle<node> {
		const auto it = blocks.find(target->inputs[index]);
		if (it != blocks.end()) {
			return target->inputs[index];
		}

		return target->get_predecessor(index);
	}

	auto control_flow_graph::try_get_traversal_index(handle<node> target) -> u64 {
		const auto it = blocks.find(target);
		if (it == blocks.end()) {
			return std::numeric_limits<u64>::max();
		}

		return it->second.id;
	}

	auto control_flow_graph::get_traversal_index(handle<node> target) const -> u64 {
		return blocks.at(target).id;
	}

	auto control_flow_graph::resolve_dominator_depth(handle<node> basic_block) -> i32 {
		const i32 dominator_depth = get_dominator_depth(basic_block);
		if (dominator_depth >= 0) {
			return dominator_depth;
		}

		// depth should be higher that its parent
		const i32 parent = resolve_dominator_depth(get_immediate_dominator(basic_block));
		blocks.at(basic_block).dominator_depth = parent + 1;
		return parent + 1;
	}

	auto control_flow_graph::get_dominator_depth(handle<node> target) const -> i32 {
		return blocks.at(target).dominator_depth;
	}

	auto control_flow_graph::compute_reverse_post_order(const codegen_context& context) -> control_flow_graph {
		ASSERT(
			context.work.items.empty(),
			"invalid work list (expected an empty work list)"
		);

		std::vector<handle<node>> stack;
		control_flow_graph graph;

		const handle<node> entry = context.function->entry_node;
		context.work.visit(entry);
		stack.push_back(entry);

		// depth-first search
		while(!stack.empty()) {
			handle<node> top = stack.back();
			stack.pop_back();

			// we've spotted a basic block entry
			if(top->is_block_begin()) {
				// a branch's projection that refers to a region would rather be
				// coalesced but won't if it's a critical edge
				if (
					top == node::type::PROJECTION &&
					top->inputs[0] == node::type::BRANCH &&
					top->use->next_user == nullptr &&
					top->use->target == node::type::REGION &&
					!top->inputs[0]->is_critical_edge(top)
				) {
					if(!context.work.visit(top->use->target)) {
						// we've already seen this block, skip it
						continue;
					}

					top = top->use->target;
				}

				// walk until we find a terminator
				handle<node> block_entry = top;
				basic_block basic_block;
				basic_block.id = graph.blocks.size();

				while(!top->is_terminator()) {
					handle<node> next = context.work.mark_next_control(top);

					if(next == nullptr) {
						break;
					}

					top = next;
				}

				// the start node always has it's dom depth filled
				basic_block.dominator_depth = basic_block.id == 0 ? 0 : -1;
				basic_block.start           = block_entry;
				basic_block.end             = top;

				context.work.items.push_back(block_entry);
				graph.blocks[block_entry] = basic_block;
			}

			// add successors
			if(top == node::type::BRANCH) {
				const u64 successor_count = top->get<branch>().successors.size();
				stack.resize(stack.size() + successor_count);
				handle<node>* top_nodes = &stack.back();

				for(handle<user> user = top->use; user; user = user->next_user) {
					const handle<node> successor = user->target;

					if(successor->is_control() && context.work.visit(successor)) {
						ASSERT(
							successor == node::type::PROJECTION,
							"successor node of a branch must be a projection"
						);

						const i64 index = successor->get<projection>().index;
						top_nodes[-index] = successor;
					}
				}
			}
			else {
				for(handle<user> user = top->use; user; user = user->next_user) {
					const handle<node> successor = user->target;

					if(successor->is_control() && context.work.visit(successor)) {
						stack.push_back(successor);
					}
				}
			}
		}

		return graph;
	}
} // namespace sigma::ir
