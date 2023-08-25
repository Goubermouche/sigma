#include "argument.h"

namespace ir {
	function_argument::function_argument(type_ptr type, const std::string& name)
	: value(name), m_type(type) {}

	type_ptr function_argument::get_type() const {
		return m_type;
	}

	std::string function_argument::to_string() {
		return m_type->to_string() + " " + get_value_string();
	}
}