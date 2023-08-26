#include "load_instruction.h"

namespace ir {
	load_instruction::load_instruction(
		value_ptr destination, 
		value_ptr value_to_load,
		type_ptr value_type,
		alignment alignment, 
		const std::string& name
	) : unary_instruction(destination, value_to_load, name),
	m_value_type(value_type), 
	m_alignment(alignment) {}

	alignment load_instruction::get_alignment() const {
		return m_alignment;
	}

	type_ptr load_instruction::get_value_type() const {
		return m_value_type;
	}

	std::string load_instruction::to_string() {
		return m_destination->get_value_string() + " = load " + m_value_type->to_string() + ", " + m_operand->get_value_string() + " align " + std::to_string(m_alignment.get_value());
	}
}