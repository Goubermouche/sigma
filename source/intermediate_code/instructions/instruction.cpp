#include "instruction.h"

namespace ir {
	instruction::instruction(value_ptr destination, const std::string& name)
		 : value(name), m_destination(destination) {}

	value_ptr instruction::get_destination() const {
		return m_destination;
	}

	std::string instruction::get_value_string() {
		return m_destination->get_value_string();
	}
}