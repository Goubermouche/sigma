#include "variable.h"

namespace sigma {
	variable::variable(
		value_ptr value,
		file_position position
	) : m_value(value),
	m_position(position) {}

	value_ptr variable::get_value() const {
		return m_value;
	}

	const file_position& variable::get_position() const {
		return m_position;
	}
}