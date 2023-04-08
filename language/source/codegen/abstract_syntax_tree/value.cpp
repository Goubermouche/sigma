#include "value.h"

namespace channel {
	value::value(const std::string& name, type type, llvm::Value* value)
		: m_name(name), m_type(type), m_value(value) {
	}

	type value::get_type() const {
		return m_type;
	}

	llvm::Value* value::get_value() const {
		return m_value;
	}

	void value::set_type(type ty) {
		m_type = ty;
	}

	void value::set_value(llvm::Value* value) {
		m_value = value;
	}

	const std::string& value::get_name() const {
		return m_name;
	}

	function::function(type return_type, llvm::Function* function)
		: m_return_type(return_type), m_value(function) {}

	type function::get_return_type() const {
		return m_return_type;
	}

	llvm::Function* function::get_function() const {
		return m_value;
	}
}