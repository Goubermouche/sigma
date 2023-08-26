#include "store_instruction.h"

namespace ir {
	store_instruction::store_instruction(
		value_ptr destination, 
		constant_ptr value_to_store,
		alignment alignment,
		const std::string& name
	) : instruction(destination, name),
	m_value_to_store(value_to_store),
	m_alignment(alignment) {}

	value_ptr store_instruction::get_value_to_store() const {
		return m_value_to_store;
	}

	alignment store_instruction::get_alignment() const {
		return m_alignment;
	}

	std::string store_instruction::to_string() {
		return "store " + m_value_to_store->to_string() + ", " + get_value_string() + " align " + std::to_string(m_alignment.get_value());
	}
}