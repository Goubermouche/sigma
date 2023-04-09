#include "function.h"

namespace channel {
	function::function(type return_type, llvm::Function* function)
		: m_return_type(return_type), m_value(function) {}

	type function::get_return_type() const {
		return m_return_type;
	}

	llvm::Function* function::get_function() const {
		return m_value;
	}
}
