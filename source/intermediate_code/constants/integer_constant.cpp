#include "integer_constant.h"

namespace ir {
	integer_constant::integer_constant(
		integer_type_ptr type,
		u64 value
	) : constant(""),
	m_type(type),
	m_value(value) {}

	integer_constant_ptr integer_constant::create(integer_type_ptr type, u64 value) {
		return std::make_shared<integer_constant>(type, value);
	}

	std::string integer_constant::to_string() {
		return m_type->to_string() + " " + std::to_string(m_value); // note: the to string should correctly describe the bit size of the value
	}

	std::string integer_constant::get_value_string() {
		return to_string();
	}
}
