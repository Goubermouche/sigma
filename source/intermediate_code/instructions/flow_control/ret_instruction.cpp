#include "ret_instruction.h"

namespace ir {
	ret_instruction::ret_instruction()
		: instruction(nullptr, ""),
	m_return_value(nullptr) {}

	ret_instruction::ret_instruction(
		value_ptr return_value
	) : instruction(nullptr, ""),
	m_return_value(return_value) {}

	value_ptr ret_instruction::get_return_value() const {
		return m_return_value;
	}

	std::string ret_instruction::to_string() {
		if (m_return_value) {
			return "ret " + m_return_value->get_value_string();
		}

		return "ret void";
	}
}