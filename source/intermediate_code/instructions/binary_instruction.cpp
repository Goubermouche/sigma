#include "binary_instruction.h"

namespace ir {
	binary_instruction::binary_instruction(
		value_ptr destination,
		value_ptr left_operand,
		value_ptr right_operand,
		const std::string& name
	) : instruction(destination, name),
	m_left_operand(left_operand),
	m_right_operand(right_operand) {}

	value_ptr binary_instruction::get_left_operand() const {
		return m_left_operand;
	}

	value_ptr binary_instruction::get_right_operand() const {
		return m_right_operand;
	}
}