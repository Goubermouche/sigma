#include "use_list.h"

namespace ir {
	void generate_use_lists(transformation_context& context) {
		context.work_list->push_all(context.function);

		for (const handle<node>& item : context.work_list->items) {
			// append locals
			if (item->ty == node::local) {
				context.locals.emplace_back(item);
			}

			// mark every node as a user of all of its input nodes
			for (u64 i = 0; i < item->inputs.get_size(); ++i) {
				if (const handle<node> input = item->inputs[i]) {
					item->add_user(input, i, nullptr, &context.function->allocator);
				}
			}
		}

		context.work_list->clear();
	}
}
