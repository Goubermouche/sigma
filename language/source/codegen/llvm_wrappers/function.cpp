#include "function.h"

namespace channel {
	function::function(type return_type, llvm::Function* function, const std::vector<std::pair<std::string, type>>& arguments)
		: m_return_type(return_type), m_value(function), m_arguments(arguments) {}

	type function::get_return_type() const {
		return m_return_type;
	}

	llvm::Function* function::get_function() const {
		return m_value;
	}
	const std::vector<std::pair<std::string, type>>& function::get_arguments() const {
		return m_arguments;
	}
}
