#include "function.h"
#include "intermediate_code/intermediate_code.h"

namespace ir {
	function::function(
		type_ptr return_type,
		const std::vector<type_ptr>& argument_types,
		const std::string& name,
		builder& builder
	) : value(name),
		m_return_type(return_type) {
		for (const auto& type : argument_types) {
			m_arguments.push_back(std::make_shared<function_argument>(
				type,
				builder.create_debug_name("")
			));
		}
	}

	void function::add_block(block_ptr block) {
		m_blocks.emplace_back(block);
	}

	std::string function::to_string() {
		std::stringstream out;

		out << m_return_type->to_string() << " " << get_value_string() << "(";

		// Get the argument list 
		for (u64 i = 0; i < m_arguments.size(); i++) {
			out << m_arguments[i]->to_string();

			if (i < m_arguments.size() - 1) {
				out << std::string(", ");
			}
		}

		out << std::string(")");

		return out.str();
	}

	const std::vector<function_argument_ptr>& function::get_arguments() const {
		return m_arguments;
	}

	const std::vector<block_ptr>& function::get_blocks() const {
		return m_blocks;
	}
}