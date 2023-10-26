#include "use_list.h"

namespace ir::cg {
	void generate_use_lists(code_generator_context& context) {
		context.work_list.clear();
		context.work_list.push_all(context.function->get_exit_node());

		for(handle<node> item : context.work_list.get_items()) {
			// append locals
			if(item->get_type() == node::local) {
				context.locals.emplace_back(item);
			}

			// mark every node as a user of all of its input nodes
			for (u64 i = 0; i < item->get_input_count(); ++i) {
				if(const handle<node> input = item->get_input(i)) {
					item->add_user(input, static_cast<i64>(i), nullptr);
				}
			}
		}
	}
}
