#pragma once
#include "../type.h"

namespace channel {
	class function {
	public:
		function(type return_type, llvm::Function* function);

		type get_return_type() const;
		llvm::Function* get_function() const;
	private:
		type m_return_type;
		llvm::Function* m_value;
	};
}
