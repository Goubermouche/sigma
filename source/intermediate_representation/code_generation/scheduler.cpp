#include "scheduler.h"

namespace ir::cg {
	void schedule_nodes(code_generator_context& context) {
		context.work_list.clear();
		context.work_list = work_list::post_order(context.function->get_entry_node());
		// context.work_list.compute_dominators(context.function->get_entry_node(), context.block_count);


		// early schedule
		context.work_list.clear_visited();
		schedule_early(context, context.function->get_exit_node());

		// late schedule
		context.work_list.clear_visited();
		for(const handle<node> item : context.work_list.get_items()) {
			schedule_late(context, item);
		}
		
		schedule_late(context, context.function->get_entry_node());
	}

	void schedule_early(code_generator_context& context, handle<node> n) {
		// don't revisit already visited nodes
		if(!context.work_list.visit(n)) {
			return;
		}

		// mark leaf nodes for further processing
		if(n->get_input_count() <= 2) {
			context.work_list.add_item(n);
		}

		// schedule inputs first, ensures that operands are scheduled before the
		// current node
		for(const handle<node> input : n->get_inputs()) {
			if(input) {
				schedule_early(context, input);
			}
		}

		// pinned nodes can't be rescheduled, skip them
		if(n->is_pinned()) {
			return;
		}

		handle<node> best = context.function->get_entry_node();
		i32 best_depth = 0;

		// choose the block with the deepest dominator depth
		for (const handle<node> input : n->get_inputs()) {
			if (input && input->get_input(0)) {
				const handle<node> parent = input->get_parent_region();
				const i32 parent_depth = parent->get<region_property>()->dominator_depth;

				if (best_depth < parent_depth) {
					best = parent;
					best_depth = parent_depth;
				}
			}
		}

		// consider the first parameter of our function as the best option
		// instead of the entry itself
		if (context.function->get_entry_node() == best) {
			best = context.function->get_parameter(0);
		}

		// rewrite the node's inputs to the newly selected best block
		n->replace_input_node(best, 0);
	}

	void schedule_late(code_generator_context& context, handle<node> n) {
		// don't revisit already visited nodes
		if(!context.work_list.visit(n)) {
			return;
		}

		// schedule all users first, so that they're scheduled before the current node
		for(const handle<user> user : n->get_users()) {
			schedule_late(context, user->get_node());
		}

		// pinned nodes can't be rescheduled, skip them
		if(n->is_pinned()) {
			return;
		}

		handle<node> least_common_ancestor = nullptr;

		// find the least common ancestor of the node
		for(const handle<user> user : n->get_users()) {
			const handle<node> user_node = user->get_node();

			// skip empty nodes
			if (user_node->get_input(0) == nullptr) {
				continue;
			}

			handle<node> user_block = user_node->get_input(0)->get_parent_region();

			// check for phi nodes, since they represent data flow merges
			if (user_node->get_type() == node::phi) {
				ASSERT(
					user_node->get_input_count() == user_block->get_input_count() + 1,
					"phi node has an incorrect number of inputs"
				);

				ptr_diff j = -1;
				for (; j < static_cast<ptr_diff>(user_node->get_input_count()); ++j) {
					if (user_node->get_input(j) == n) {
						break;
					}
				}

				ASSERT(j >= 0, "unable to find the current node in phi inputs");
				user_block = user_block->get_input(j - 1)->get_block_begin();
			}

			least_common_ancestor = find_least_common_ancestor(least_common_ancestor, user_block);
		}

		// consider the first parameter of our function as the best option
		// instead of the entry itself
		if(context.function->get_entry_node() == least_common_ancestor) {
			least_common_ancestor = context.function->get_parameter(0);
		}

		// rewrite the node's inputs to the newly selected block
		n->replace_input_node(least_common_ancestor, 0);
	}
}
