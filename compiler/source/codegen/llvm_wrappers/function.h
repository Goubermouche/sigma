#pragma once
#include "../type.h"

namespace channel {
	class function {
	public:
		function(type return_type, llvm::Function* function, const std::vector<std::pair<std::string, type>>& arguments);

		type get_return_type() const;
		llvm::Function* get_function() const;
		const std::vector<std::pair<std::string, type>>& get_arguments() const;
	private:
		type m_return_type;
		llvm::Function* m_value;
		std::vector<std::pair<std::string, type>> m_arguments;
	};
}
