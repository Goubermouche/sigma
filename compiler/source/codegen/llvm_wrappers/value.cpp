#include "value.h"

namespace channel {
	value::value(
		const std::string& name, 
		type type,
		llvm::Value* value
	) : m_name(name),
	m_type(type),
	m_value(value) {}

	type value::get_type() const {
		return m_type;
	}

	llvm::Value* value::get_value() const {
		return m_value;
	}

	llvm::Value* value::get_pointer() const {
		return m_pointer;
	}

	void value::set_type(type ty) {
		m_type = ty;
	}

	void value::set_value(llvm::Value* value) {
		m_value = value;
	}

	void value::set_pointer(llvm::Value* pointer) {
		m_pointer = pointer;
	}

	const std::string& value::get_name() const {
		return m_name;
	}
}