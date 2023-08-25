#include "unary_instruction.h"

namespace ir {
	unary_instruction::unary_instruction(
		value_ptr destination, 
		value_ptr operand,
		const std::string& name
	) : instruction(destination, name),
	m_operand(operand) {}

	value_ptr unary_instruction::get_operand() const {
		return m_operand;
	}
}