#include "use_list.h"

namespace sigma::ir {
	void generate_use_lists(transformation_context& context) {
		context.work.push_all(context.function);

		for (const handle<node> item : context.work.items) {
			// append locals
			if (item == node::type::LOCAL) {
				context.locals.emplace_back(item);
			}

			// mark every node as a user of all of its input nodes
			for (u64 i = 0; i < item->inputs.get_size(); ++i) {
				if (const handle<node> input = item->inputs[i]) {
					item->add_user(input, i, nullptr, &context.function->allocator);
				}
			}
		}

		context.work.clear();
	}
} // namespace sigma::ir
