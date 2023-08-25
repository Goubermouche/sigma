#include "store_instruction.h"

namespace ir {
	store_instruction::store_instruction(
		value_ptr destination, 
		constant_ptr value_to_store,
		const std::string& name
	) : instruction(destination, name),
	m_value_to_store(value_to_store) {}

	value_ptr store_instruction::get_value_to_store() const {
		return m_value_to_store;
	}

	std::string store_instruction::to_string() {
		return "store " + m_value_to_store->to_string() + ", " + get_value_string();
	}
}