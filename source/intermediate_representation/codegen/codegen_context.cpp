#include "codegen_context.h"

namespace sigma::ir {
	void codegen_context::append_instruction(handle<instruction> instruction) {
		head->next_instruction = instruction;
		head = instruction;
	}

	void codegen_context::hint_reg(u64 interval_index, reg reg) {
		if (intervals[interval_index].hint.is_valid() == false) {
			intervals[interval_index].hint = reg;
		}
	}

	auto codegen_context::create_symbol_patch() const -> handle<symbol_patch> {
		return static_cast<symbol_patch*>(function->allocator.allocate_zero(sizeof(symbol_patch)));
	}

	auto codegen_context::lookup_virtual_value(handle<node> value) -> handle<virtual_value> {
		const auto it = virtual_values.find(value->global_value_index);
		if(it == virtual_values.end()) {
			return nullptr;
		}

		return &it->second;
	}

	auto codegen_context::allocate_stack(u64 size, u64 alignment) -> i32 {
		stack_usage = utility::align(stack_usage + size, alignment);
		return -static_cast<i32>(stack_usage);
	}

	auto codegen_context::get_stack_slot(handle<node> node) -> i32 {
		// check if a stack slot for the node already exists 
		const auto it = stack_slots.find(node);

		// if it exists, return the stack position
		if (it != stack_slots.end()) {
			return it->second;
		}

		// allocate a new stack slot for the given node
		const local& local_prop = node->get<local>();
		const i32 position = allocate_stack(local_prop.size, local_prop.alignment);
		stack_slots[node] = position;

		return position;
	}
} // namespace sigma::ir
