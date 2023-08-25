#include "stack_allocation.h"

namespace ir {
	stack_allocation::stack_allocation(
		value_ptr destination,
		type_ptr type, 
		const std::string& name
	) : unary_instruction(destination, type, name) {}

	std::string stack_allocation::to_string() {
		return "@" + m_destination->get_name() + " = allocate " + m_operand->to_string();
	}
}