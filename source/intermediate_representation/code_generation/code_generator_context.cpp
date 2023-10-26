#include "code_generator_context.h"

namespace ir::cg {
	value_description* code_generator_context::lookup_value(handle<node> n) {
		return work_list.get_visited().contains(n) ? &values[n->get_global_value_index()] : nullptr;
	}

	i32 code_generator_context::allocate_stack(u32 size, u32 alignment) {
		stack_usage = utility::align(stack_usage + size, alignment);
		return -static_cast<i32>(stack_usage);
	}

	i32 code_generator_context::can_folded_store(
		handle<node> memory,
		handle<node> address, 
		handle<node> source
	) {
		switch (source->get_type()) {
			default: {
				return -1;
			}

			case node::type::conjunction:
			case node::type::disjunction:
			case node::type::exclusive_disjunction:
			case node::type::addition:
			case node::type::subtraction: {
				if (
					source->get_input(1)->get_type() == node::type::load &&
					source->get_input(1)->get_input(1) == memory &&
					source->get_input(1)->get_input(2) == address &&
					on_last_use(source) &&
					on_last_use(source->get_input(1))
					) {
					constexpr static instruction::type operations[] = {
						instruction::AND,
						instruction::OR,
						instruction::XOR,
						instruction::ADD,
						instruction::SUB,
					};

					return operations[static_cast<i32>(source->get_type()) - static_cast<i32>(node::type::conjunction)];
				}

				return -1;
			}
		}
	}
	bool code_generator_context::on_last_use(handle<node> n) {
		const value_description* value = lookup_value(n);
		return value ? value->get_use_count() == 1 : false;
	}
}
