#include "work_list.h"
#include "intermediate_representation/codegen/codegen_context.h"

namespace sigma::ir {
	auto work_list::mark_next_control(handle<node> target) -> handle<node> {
		// unless it's a branch (aka a terminator), it'll have one successor
		handle<node> next = nullptr;

		for (handle<user> user = target->use; user; user = user->next_user) {
			handle<node> successor = user->n;

			// we can't treat regions in the chain
			if (successor->ty == node::REGION) {
				break;
			}

			// we've found the next step in control flow
			if (successor->is_control() && visit(successor)) {
				return successor;
			}
		}

		return nullptr;
	}

	void work_list::compute_dominators(control_flow_graph& cfg) const {
		const auto entry = &cfg.blocks.at(items[0]);
		bool changed = true;

		entry->dominator = entry;

		while (changed) {
			changed = false;

			// for all nodes, b, in reverse post order (except start node)
			for (u64 i = 1; i < cfg.blocks.size(); ++i) {
				handle<node> block = items[i];
				handle<node> new_immediate_dominator = nullptr;

				// pick first "processed" predecessor
				const u64 predecessor_count = block->inputs.get_size();
				u64 j = 0;

				for (; j < predecessor_count; j++) {
					const handle<node> predecessor = block->get_predecessor(j);

					if (cfg.get_immediate_dominator(predecessor)) {
						new_immediate_dominator = predecessor;
						break;
					}
				}

				// for all other predecessors, p, of b
				for (; j < predecessor_count; j++) {
					const handle<node> predecessor = block->get_predecessor(j);

					if (handle<node> immediate_dominator = cfg.get_immediate_dominator(predecessor)) {
						ASSERT(predecessor->inputs.get_size() > 0, "panic");
						u64 a = cfg.try_get_traversal_index(predecessor);

						if (a != std::numeric_limits<u64>::max()) {
							u64 b = cfg.get_traversal_index(new_immediate_dominator);

							while (a != b) {
								while (a > b) {
									const handle<node> dominator = cfg.get_immediate_dominator(items[a]);
									a = dominator ? cfg.get_traversal_index(dominator) : 0;
								}

								while (b > a) {
									const handle<node> dominator = cfg.get_immediate_dominator(items[b]);
									b = dominator ? cfg.get_traversal_index(dominator) : 0;
								}
							}

							new_immediate_dominator = items[a];
						}
					}
				}

				ASSERT(new_immediate_dominator != nullptr, "panic");
				const auto basic_block = &cfg.blocks.at(block);

				if (
					basic_block->dominator == nullptr || 
					basic_block->dominator->start != new_immediate_dominator
				) {
					basic_block->dominator = &cfg.blocks.at(new_immediate_dominator);
					changed = true;
				}
			}
		}

		// generate depth values for each block
		for (u64 i = cfg.blocks.size(); i-- > 1;) {
			cfg.resolve_dominator_depth(items[i]);
		}
	}

	void work_list::push_all(handle<function> function) {
		std::stack<handle<node>> stack;

		for (const handle<node>& end : function->terminators) {
			// place the endpoint, we'll construct the rest from there
			if (!visit(end)) {
				continue;
			}

			stack.push(end);

			while (!stack.empty()) {
				handle<node> n = stack.top();
				stack.pop();

				// place self first
				items.push_back(n);

				for (const handle<node>& input : n->inputs) {
					if (input && visit(input)) {
						stack.push(input);
					}
				}
			}
		}
	}

	auto work_list::visit(handle<node> node) -> bool {
		return visited_items.insert(node).second;
	}

	void work_list::clear() {
		visited_items.clear();
		items.clear();
	}
}
