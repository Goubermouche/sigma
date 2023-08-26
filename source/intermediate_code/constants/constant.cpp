#include "constant.h"

namespace ir {
	constant::constant(type_ptr type, const std::string& name)
		: value(name), m_type(type) {}

	type_ptr constant::get_type() const {
		return m_type;
	}
}
