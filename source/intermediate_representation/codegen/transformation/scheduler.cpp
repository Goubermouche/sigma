#include "scheduler.h"

namespace sigma::ir {
	void schedule_early(
		codegen_context& context, const handle<node>& target
	) {
		if (!context.work->visit(target)) {
			return;
		}

		// push node, late scheduling will process this list
		context.work->items.push_back(target);

		// schedule inputs first
		for(const handle<node>& input : target->inputs) {
			if(input) {
				schedule_early(context, input);
			}
		}

		// skip pinned nodes
		if(target->is_pinned() && target->inputs.get_size() > 0) {
			return;
		}

		// start at the entry point
		handle best = context.schedule.at(context.work->items[0]);
		i32 best_depth = 0;

		// choose deepest block
		for (u64 i = 0; i < target->inputs.get_size(); ++i) {
			if (const handle<node> input = target->inputs[i]) {
				auto it = context.schedule.find(input);

				if (it == context.schedule.end()) {
					continue;
				}

				const handle<basic_block> basic_block = it->second;

				if (best_depth < basic_block->dominator_depth) {
					best_depth = basic_block->dominator_depth;
					best = basic_block;
				}
			}
		}

		best->items.insert(target);
		context.schedule[target] = best;
	}

	void schedule_late(
		codegen_context& context, const handle<node>& target
	) {
		// skip pinned nodes
		if(target->is_pinned()) {
			return;
		}

		handle<basic_block> least_common_ancestor = nullptr;

		// find the least common ancestor
		for (auto use = target->use; use; use = use->next_user) {
			handle<node> user_node = use->n;

			auto it = context.schedule.find(user_node);
			if (it == context.schedule.end()) {
				continue; // node is dead
			}

			handle<basic_block> use_block = it->second;

			if (user_node->ty == node::PHI) {
				const handle<node> use_node = user_node->inputs[0];

				ASSERT(
					use_node->ty == node::REGION,
					"user block expects a region node"
				);

				ASSERT(
					user_node->inputs.get_size() == use_node->inputs.get_size() + 1,
					"phi has parent with mismatched predecessors"
				);

				u64 j = 1;
				for (; j < user_node->inputs.get_size(); ++j) {
					if (user_node->inputs[j] == target) {
						break;
					}
				}

				it = context.schedule.find(use_node->inputs[j - 1]);
				if (it != context.schedule.end()) {
					use_block = it->second;
				}
			}

			least_common_ancestor = find_least_common_ancestor(least_common_ancestor, use_block);
		}

		if (least_common_ancestor) {
			const auto it = context.schedule.find(target);

			if (it != context.schedule.end()) {
				// replace the old ancestor
				const handle<basic_block> old = it->second;
				context.schedule[target] = least_common_ancestor;
				old->items.erase(target);
			}
			else {
				context.schedule[target] = least_common_ancestor;
			}
		}
	}

	void schedule_node_hierarchy(codegen_context& context) {
		// generate graph dominators 
		context.work->compute_dominators(context.graph);
		context.work->visited_items.clear();

		for (u64 i = 0; i < context.graph.blocks.size(); ++i) {
			context.graph.blocks.at(context.work->items[i]).items.reserve(32);
		}

		for (u64 i = context.graph.blocks.size(); i-- > 0;) {
			handle<node> basic_block_node = context.work->items[i];
			const handle basic_block = &context.graph.blocks[basic_block_node];
			handle<node> basic_block_end = basic_block->end;

			if (i == 0) {
				// schedule the entry node
				handle<node> start = context.func->entry_node;
				basic_block->items.insert(start);
				context.schedule[start] = basic_block;
			}

			while(true) {
				basic_block->items.insert(basic_block_end);
				context.schedule[basic_block_end] = basic_block;

				// add projections to the same block
				for (handle<user> use = basic_block_end->use; use; use = use->next_user) {
					handle<node> projection = use->n;

					if (
						use->slot == 0 &&
						(projection->ty == node::PROJECTION || projection->ty == node::PHI)
					) {
						if (!context.schedule.contains(projection)) {
							basic_block->items.insert(projection);
							context.schedule[projection] = basic_block;
						}
					}
				}

				if (basic_block_end == basic_block_node) {
					break;
				}

				basic_block_end = basic_block_end->inputs[0];
			}
		}

		for (u64 i = context.graph.blocks.size(); i-- > 0;) {
			schedule_early(context, context.graph.blocks.at(context.work->items[i]).end);
		}

		for (u64 i = context.graph.blocks.size(); i < context.work->items.size(); ++i) {
			schedule_late(context, context.work->items[i]);
		}

		context.work->items.resize(context.graph.blocks.size());
		context.work->visited_items.clear();
		context.labels.resize(context.graph.blocks.size());
	}
}