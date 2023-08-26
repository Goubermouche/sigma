#include "stack_allocation.h"

namespace ir {
	stack_allocation::stack_allocation(
		value_ptr destination,
		type_ptr type,
		alignment alignment,
		const std::string& name
	) : unary_instruction(destination, type, name), m_alignment(alignment) {}

	alignment stack_allocation::get_alignment() const {
		return m_alignment;
	}

	std::string stack_allocation::to_string() {
		return "@" + m_destination->get_name() + " = allocate " + m_operand->to_string() + " align " + std::to_string(m_alignment.get_value());
	}
}