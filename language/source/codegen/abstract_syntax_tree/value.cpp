#include "value.h"

namespace channel {
	value::value(type type, llvm::Value* value)
		: m_type(type), m_value(value) {
	}

	type value::get_type() const {
		return m_type;
	}

	llvm::Value* value::get_value() const {
		return m_value;
	}
}