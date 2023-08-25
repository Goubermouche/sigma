#include "variable.h"

namespace sigma {
	variable::variable(
		value_ptr value,
		const utility::file_range& range
	) : m_value(value),
	m_range(range) {}

	value_ptr variable::get_value() const {
		return m_value;
	}

	const utility::file_range& variable::get_range() const {
		return m_range;
	}
}