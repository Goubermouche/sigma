#include "add_instruction.h"

namespace ir {
	add_instruction::add_instruction(
		value_ptr destination,
		value_ptr left_operand,
		value_ptr right_operand,
		const std::string& name
	) : binary_instruction(destination, left_operand, right_operand, name) {}

	std::string add_instruction::to_string() {
		return get_value_string() + " = add " + m_left_operand->get_value_string() + ", " + m_right_operand->get_value_string();
	}
}