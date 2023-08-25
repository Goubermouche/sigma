#include "intermediate_code.h"

namespace ir {
	stack_allocation_instruction_ptr builder::create_stack_allocation(
		type_ptr type, 
		const std::string& name
	) {
		value_ptr destination = std::make_shared<value>(create_debug_name(name));
		const stack_allocation_instruction_ptr allocation = std::make_shared<stack_allocation>(
			destination,
			type,
			""
		);

		return append(allocation);
	}

	store_instruction_ptr builder::create_store(
		stack_allocation_instruction_ptr allocation,
		constant_ptr value_to_store, 
		const std::string& name
	) {
		return append(std::make_shared<store_instruction>(
			allocation->get_destination(),
			value_to_store,
			create_debug_name(name)
		));
	}

	add_instruction_ptr builder::create_add(
		value_ptr left,
		value_ptr right, 
		const std::string& name
	) {
		value_ptr destination = std::make_shared<value>(create_debug_name(name));
		const add_instruction_ptr add = std::make_shared<add_instruction>(
			destination,
			left,
			right,
			""
		);

		return append(add);
	}

	function_ptr builder::create_function(
		type_ptr return_type, 
		const std::vector<type_ptr>& argument_types, 
		const std::string& name
	) {
		function_ptr func = std::make_shared<function>(
			return_type,
			argument_types,
			create_debug_name(name),
			*this
		);

		return m_functions.emplace_back(func);
	}

	ret_instruction_ptr builder::create_ret(
		value_ptr return_value
	) {
		const ret_instruction_ptr ret = std::make_shared<ret_instruction>(
			return_value
		);

		return append(ret);
	}

	block_ptr builder::create_block(
		function_ptr parent_function,
		const std::string& name
	) {
		auto b = std::make_shared<block>(create_debug_name(name));
		ASSERT(!m_functions.empty(), "no function capable of receiving a block exists (m_functions is empty)");

		m_functions.back()->add_block(b);
		return b;
	}

	ret_instruction_ptr builder::create_ret_void() {
		const ret_instruction_ptr ret = std::make_shared<ret_instruction>();
		return append(ret);
	}

	void builder::set_insert_point(
		block_ptr block
	) {
		m_insert_point = block;
	}

	void builder::print() const {
		// for all functions
		for (const auto& function : m_functions) {
			utility::console::out << function->to_string() << " {";
			const auto& function_blocks = function->get_blocks();

			if (function_blocks.empty()) {
				utility::console::out << '}';
			}

			utility::console::out << '\n';

			// for all blocks 
			for (const auto& block : function_blocks) {
				utility::console::out << "  " << block->to_string() << '\n';
				const auto& block_instructions = block->get_instructions();

				// for all instructions 
				for(const auto& instruction : block_instructions) {
					utility::console::out << "    " << instruction->to_string() << '\n';
				}
			}

			utility::console::out << (function_blocks.empty() ? "\n" : "}\n\n");
		}
	}


	std::string builder::create_debug_name(
		const std::string& name
	) const {
		static u64 debug_name_index;
		return name.empty() ? std::to_string(debug_name_index++) : name;
	}
}